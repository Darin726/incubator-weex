//
// Created by Xu Jiacheng on 2018/12/26.
//

#include "rax_root_element.h"
#include "base/qking_common_logger.h"
#include "rax_core_util.h"
#include "rax_ref.h"

RAX_NAME_SPACE_BEGIN

RaxRootElement::RaxRootElement(uint32_t eid)
    : RaxNativeNodeHolder(eid, RaxElementType::kRoot) {}

void RaxRootElement::MountComponentInternal(native_node_ptr render_parent,
                                            RaxElement *component_parent,
                                            const ChildMounter &mounter) {
  // forward to child
  RAX_ASSERT(child());
  child()->MountComponent(render_parent, this, mounter);
}

void RaxRootElement::UnmountComponentInternal(bool not_remove_child) {
  RAX_ASSERT(false);
}

void RaxRootElement::UpdateComponentInternal(RaxElement *prev_ele,
                                             RaxElement *next_ele,
                                             uint32_t insert_start) {
  RAX_ASSERT(false);
}

void RaxRootElement::DebugPrintComponent() {
  RaxElement::DebugPrintComponent();
  RaxElement::DebugPrintComponent();
  std::vector<std::string> event_keys;
  std::transform(
      get_events().begin(), get_events().end(), std::back_inserter(event_keys),
      [](const std::map<std::string, std::unique_ptr<QKValueRef>>::value_type
             &pair) { return pair.first; });

  RAX_LOGD(
      "RaxComponent(%u): Root <root>\n"
      "\t\tevent[%u]: %s\n"
      "\t\tstyle[%u]: %s\n"
      "\t\tattr[%u]: %s",
      eid(), static_cast<uint32_t>(get_events().size()),
      Json(event_keys).dump().c_str(),
      static_cast<uint32_t>(get_styles().size()),
      Json(get_styles()).dump().c_str(),
      static_cast<uint32_t>(get_attrs().size()),
      Json(get_attrs()).dump().c_str());
}

Json::object RaxRootElement::DebugCollectComponent() {
  Json::object obj = RaxElement::DebugCollectComponent();
  RAX_ASSERT(child());
  obj["child"] = child()->DebugCollectComponent();
  return obj;
}
native_node_ptr RaxRootElement::get_first_render_node() {
  RAX_ASSERT(child());
  return child()->get_first_render_node();
}
native_node_ptr RaxRootElement::get_last_render_node() {
  RAX_ASSERT(child());
  return child()->get_last_render_node();
}

qking_value_t RaxRootElement::get_js_native_node() {
  if (!js_native_node_) {
    if (comp_native_node_) {
      // has mounted
      qking_value_t obj = rax_create_native_node_obj(eid_str());
      js_native_node_.reset(new QKValueRef(obj));
      qking_release_value(obj);
      return obj;
    } else {
      // not mounted yet
      return qking_create_undefined();
    }
  } else {
    return js_native_node_->get();
  }
}

RAX_NAME_SPACE_END
