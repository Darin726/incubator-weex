//
// Created by Xu Jiacheng on 2018/12/26.
//

#include "rax_native_element.h"
#include <numeric>
#include "base/qking_common_logger.h"
#include "rax_element_factory.h"
#include "rax_native_transform.h"
#include "rax_ref.h"
#include "rax_render_bridge.h"
#include "rax_update.h"

RAX_NAME_SPACE_BEGIN
using namespace render_bridge;

RaxNativeElement::~RaxNativeElement() {}

RaxNativeElement::RaxNativeElement(uint32_t eid)
    : RaxNativeNodeHolder(eid, RaxElementType::kNative) {}

void RaxNativeElement::MountComponentInternal(native_node_ptr render_parent,
                                              RaxElement *component_parent,
                                              const ChildMounter &mounter) {
  rax_native_transform(this);

  SetNativeCompProps();
  // create native
  native_node() = NativeNodeCreate(this);
  RAX_ASSERT(native_node());

  if (!qking_value_is_null_or_undefined(get_js_ref())) {
    rax_ref_attach(get_comp_owner(), get_js_ref(), this);
  }

  MountChildren();

  if (mounter) {
    mounter(render_parent, native_node());
  } else {
    NativeNodeAddChildren(render_parent, native_node());
  }
}

void RaxNativeElement::MountChildren() {
  qking_value_t child = get_js_children();

  // text has been transformed in createElement
  RAX_ASSERT(comp_type() != "text" || qking_value_is_null_or_undefined(child));

  if (!qking_value_is_array(child)) {
    if (!qking_value_is_null_or_undefined(child)) {
      // only one.
      RaxElement *child_ele = get_factory()->GetRawTypeElement(child);
      if (child_ele == nullptr) {
        throw rax_common_err("Native MountChildren: Not a valid child type");
      }
      child_ele->MountComponent(native_node(), this, nullptr);
      comp_children().push_back(child_ele);
    }
    // else { no children }
  } else {
    struct Passer {
      RaxNativeElement *parent_this;
      bool error;
      qking_value_t js_err;
      std::string js_err_msg;
    } tmp{this, false, qking_create_undefined(), ""};
    qking_foreach_object_property_of(
        child,
        [](const qking_value_t property_name,
           const qking_value_t property_value, void *user_data_p) -> bool {
          Passer *passer = (Passer *)user_data_p;
          RaxNativeElement *parent_this = passer->parent_this;
          RaxElement *child_ele =
              parent_this->get_factory()->GetRawTypeElement(property_value);
          if (child_ele == nullptr) {
            passer->error = true;
            return false;
          }
          try {
            child_ele->MountComponent(parent_this->native_node(), parent_this,
                                      nullptr);
          } catch (rax_js_err &e) {
            passer->js_err = e.acquire_err();
            return false;
          } catch (rax_common_err &e) {
            passer->js_err_msg = e.what();
          }
          parent_this->comp_children().push_back(child_ele);
          return true;
        },
        &tmp, true, true, false);

    if (tmp.error) {
      throw rax_common_err("Native MountChildren: Not a valid child type");
    }
    if (!tmp.js_err_msg.empty()) {
      throw rax_common_err(tmp.js_err_msg);
    }
    if (!qking_value_is_undefined(tmp.js_err)) {
      throw rax_js_err(tmp.js_err, true);
    }
  }
}

void RaxNativeElement::SetNativeCompProps() {
  qking_value_t props = get_js_props();

  SetNativeProps(props,
                 [this](const std::string &key, const std::string &value) {
                   set_style(key, value);
                 },
                 [this](const std::string &key, const std::string &value) {
                   set_attr(key, value);
                 },
                 [this](const std::string &event, qking_value_t value) {
                   set_event(event, value);
                 });
}

void RaxNativeElement::UnmountComponentInternal(bool not_remove_child) {
  RAX_ASSERT(native_node());

  if (!qking_value_is_null_or_undefined(get_js_ref())) {
    rax_ref_detach(get_comp_owner(), get_js_ref(), this);
  }

  if (!not_remove_child) {
    if (is_mount_to_root()) {
      RemoveNode(native_node());
    } else {
      NativeNodeRemoveNode(native_node());
    }
  }

  for (auto &it : comp_children()) {
    it->UnmountComponent(true);  // parent or higher parent has been removed.
  }
}

void RaxNativeElement::UpdateComponentInternal(RaxElement *prev_ele,
                                               RaxElement *next_ele,
                                               uint32_t insert_start) {
  RAX_ASSERT(prev_ele != next_ele);
  RAX_ASSERT(
      qking_value_strict_equal(this->get_js_type(), next_ele->get_js_type()));
  RAX_ASSERT(
      qking_value_strict_equal(this->get_js_key(), next_ele->get_js_key()));

  RaxNativeElement *next_native = dynamic_cast<RaxNativeElement *>(next_ele);

  if (rax_native_update_transform(next_native)) {
    RAX_LOGW("NativeElement<%s>: Skip update", comp_type().c_str());
    return;
  }

  rax_ref_update(prev_ele, next_ele);

  UpdateProperties(next_native);
  UpdateChildren(next_native);

  CopyFromAnotherElement(next_ele);
}

void RaxNativeElement::UpdateProperties(RaxNativeElement *next_ele) {
  // prepare native props to compare.
  next_ele->SetNativeCompProps();

  UpdateNodeProperties(this, next_ele);
}

void RaxNativeElement::UpdateChildren(RaxNativeElement *next_ele) {
  // text has been transformed in createElement
  RAX_ASSERT(next_ele->comp_type() != "text" ||
             qking_value_is_null_or_undefined(next_ele->get_js_children()));

  UpdateComponentChildren(next_ele, this, native_node(), 0);
}

void RaxNativeElement::DebugPrintComponent() {
  RaxElement::DebugPrintComponent();
  std::vector<std::string> event_keys;
  std::transform(
      get_events().begin(), get_events().end(), std::back_inserter(event_keys),
      [](const std::map<std::string, std::unique_ptr<QKValueRef>>::value_type
             &pair) { return pair.first; });

  RAX_LOGD(
      "RaxComponent(%u): Native <%s>\n"
      "\t\tevent[%u]: %s\n"
      "\t\tstyle[%u]: %s\n"
      "\t\tattr[%u]: %s",
      eid(), comp_type().c_str(), static_cast<uint32_t>(get_events().size()),
      Json(event_keys).dump().c_str(),
      static_cast<uint32_t>(get_styles().size()),
      Json(get_styles()).dump().c_str(),
      static_cast<uint32_t>(get_attrs().size()),
      Json(get_attrs()).dump().c_str());
}

Json::object RaxNativeElement::DebugCollectComponent() {
  Json::object obj = RaxElement::DebugCollectComponent();

  std::vector<std::string> event_keys;
  std::transform(
      get_events().begin(), get_events().end(), std::back_inserter(event_keys),
      [](const std::map<std::string, std::unique_ptr<QKValueRef>>::value_type
             &pair) { return pair.first; });

  obj["tag"] = comp_type();

  obj["style"] = get_styles();
  obj["attr"] = get_attrs();
  obj["events"] = event_keys;

  // child
  Json::array child_array;
  for (auto &it : comp_children()) {
    child_array.push_back(it->DebugCollectComponent());
  }
  if (!child_array.empty()) {
    obj["children"] = child_array;
  }
  return obj;
}

native_node_ptr RaxNativeElement::get_first_render_node() {
  RAX_ASSERT(native_node());
  return native_node();
}

native_node_ptr RaxNativeElement::get_last_render_node() {
  RAX_ASSERT(native_node());
  return native_node();
}

RAX_NAME_SPACE_END
