//
// Created by Xu Jiacheng on 2018/12/26.
//

#include "rax_class_element.h"
#include "base/qking_common_logger.h"
#include "rax_core_util.h"
#include "rax_element_factory.h"
#include "rax_error_handle.h"
#include "rax_ref.h"
#include "rax_render_bridge.h"
#include "rax_update.h"

RAX_NAME_SPACE_BEGIN

RaxClassElement::RaxClassElement(uint32_t eid)
    : RaxRenderCompHolder(eid, RaxElementType::kClass) {}

RaxElement *RaxClassElement::CallRenderOnInstance(qking_value_t *err) {
  qking_value_t class_inst = get_class_instance();
  RAX_ASSERT(!qking_value_is_undefined(class_inst));

  qking_value_t render_func =
      get_optional_property(class_inst, QKING_LIT_MAGIC_STRING_EX_RENDER,
                            "ClassElement MountComponent", "class_inst");

  qking_value_t ret = qking_call_function(render_func, class_inst, nullptr, 0);

  if (qking_value_is_error(ret)) {
    const std::string &err_msg = string_from_qking_error(ret);
    std::string final_msg =
        "ClassElement MountComponent: ClassElement Element render exception, ";
    final_msg.append(err_msg);
    RAX_LOGW("%s", final_msg.c_str());

    if (err) {
      qking_release_value(*err);
      *err = ret;
    } else {
      rax_handle_error(this, ret, true);
      qking_release_value(ret);
    }
    ret = qking_create_undefined();
  }

  RaxElement *rendered = get_factory()->GetRawTypeElement(ret);
  if (!rendered) {
    qking_release_value(ret);
    qking_release_value(render_func);

    if (err) {
      qking_release_value(*err);
    }
    throw rax_common_err("ClassElement MountComponent: Invalid return type");
  }

  qking_release_value(ret);
  qking_release_value(render_func);
  return rendered;
}

void RaxClassElement::MountComponentInternal(native_node_ptr render_parent,
                                             RaxElement *component_parent,
                                             const ChildMounter &mounter) {
  qking_value_t class_inst = qking_create_undefined();
  qking_value_t err_value = qking_create_undefined();

  try {
    // instance class
    {
      qking_value_t args[1] = {get_js_props()};
      class_inst = qking_construct_object(get_js_type(), args, 1);
      if (qking_value_is_error(class_inst)) {
        const std::string &err_msg = string_from_qking_error(class_inst);
        std::string final_msg =
            "ClassElement MountComponent: Class Element instantiate "
            "exception, ";
        final_msg.append(err_msg);
        RAX_LOGW("%s", final_msg.c_str());

        // create an empty element to fill.
        RaxElement *empty_child =
            get_factory()->GetRawTypeElement(qking_create_undefined());
        set_rendered_comp(empty_child);
        empty_child->MountComponent(render_parent, this, mounter);

        rax_handle_error(component_parent, class_inst, false);
        qking_release_value(class_inst);
        return;
      }
      // hold class instance
      set_class_instance(class_inst);

      // setup props, refs. according to rax:composites.js:122.
      {
        set_optional_property(class_inst, QKING_LIT_MAGIC_STRING_EX_PROPS,
                              get_js_props(), "ClassElement MountComponent",
                              "class_inst");

        qking_value_t empty_refs = qking_create_object();
        qking_release_value(
            qking_set_property_by_name(class_inst, "refs", empty_refs));
        qking_release_value(empty_refs);
      }

      // setup state
      {
        qking_value_t old_state = get_optional_property(
            class_inst, QKING_LIT_MAGIC_STRING_EX_STATE,
            "ClassElement MountComponent", "class_inst",
            [=](qking_value_t ret) {
              if (qking_value_is_undefined(ret)) {
                set_optional_property(
                    class_inst, QKING_LIT_MAGIC_STRING_EX_STATE,
                    qking_create_null(), "ClassElement MountComponent",
                    "class_inst");
              }
            });
        qking_release_value(old_state);
      }

      // setup _internal
      {
        // make sure js_ele is still valid
        RAX_ASSERT(
            qking_get_object_native_pointer(get_js_ele(), nullptr, nullptr));
        set_optional_property(class_inst, QKING_LIT_MAGIC_STRING_EX_UD_INTERNAL,
                              get_js_ele(), "ClassElement MountComponent",
                              "class_inst");
      }
    }

    // componentWillMount
    {
      qking_release_value(
          perform_safe_to_value(this, err_value, [=]() -> qking_value_t {
            return make_optional_call_on(
                class_inst, QKING_LIT_MAGIC_STRING_EX_COMPONENT_WILL_MOUNT,
                "ClassElement MountComponent");
          }));
    }

    get_factory()->owner() = this;

    // Process pending state when call setState in componentWillMount
    {
      qking_value_t new_state = ProcessPendingState(get_js_props());
      RAX_ASSERT(!qking_value_is_error(new_state));
      qking_value_t ret = qking_set_property_by_name_lit(
          class_inst, QKING_LIT_MAGIC_STRING_EX_STATE, new_state);
      if (qking_value_is_error(ret)) {
        RAX_LOGE("ClassElement MountComponent: class_inst['state'] set err: %s",
                 string_from_qking_error(ret).c_str());
      }
      qking_release_value(new_state);
      qking_release_value(ret);
    }

    // render
    {
      RaxElement *rendered = CallRenderOnInstance(&err_value);

      get_factory()->owner() = nullptr;

      set_rendered_comp(rendered);
      rendered->MountComponent(render_parent, this, mounter);
    }

    // err handle
    if (qking_value_is_error(err_value)) {
      rax_handle_error(this, err_value, false);
    }

    // ref
    if (!qking_value_is_null_or_undefined(get_js_ref())) {
      rax_ref_attach(get_comp_owner(), get_js_ref(), this);
    }

    // componentDidMount
    {
      perform_safe_to_queue(this, [=]() -> qking_value_t {
        return make_optional_call_on(
            class_inst, QKING_LIT_MAGIC_STRING_EX_COMPONENT_DID_MOUNT,
            "ClassElement MountComponent");
      });
    }

    // at this point, sub native-element's render_object has been hooked to the
    // root tree. setState call inside err queue will be considered as a normal
    // update process, instead of a not-mounted update process.
    set_comp_mounted(true);

    // flush err callback
    flush_err_queue(this);
    FlushCallbackQueue(this);

  } catch (const rax_exception &e) {
    qking_release_value(class_inst);
    qking_release_value(err_value);
    throw;
  }
  qking_release_value(class_inst);
  qking_release_value(err_value);
}

void RaxClassElement::UnmountComponentInternal(bool not_remove_child) {
  RAX_ASSERT(get_rendered_comp());
  // componentWillUnmount
  {
    qking_value_t class_instance = get_class_instance();
    RAX_ASSERT(qking_value_is_object(class_instance));
    perform_safe_to_queue(this, [=]() {
      return make_optional_call_on(
          class_instance, QKING_LIT_MAGIC_STRING_EX_COMPONENT_WILL_UNMOUNT,
          "ClassElement UnmountComponent");
    });
  }

  if (!qking_value_is_null_or_undefined(get_js_ref())) {
    rax_ref_detach(get_comp_owner(), get_js_ref(), this);
  }

  get_rendered_comp()->UnmountComponent(not_remove_child);
}

void RaxClassElement::UpdateComponentInternal(RaxElement *prev_ele,
                                              RaxElement *next_ele,
                                              uint32_t insert_start) {
  // only called by setState, could prev_ele == next_ele;
  RAX_ASSERT((prev_ele != next_ele) || g_by_state--);

  qking_value_t class_inst = get_class_instance();
  if (!qking_value_is_object(class_inst)) {
    RAX_LOGE(
        "Update class component(%u) that failed to mount(class construct err)",
        eid());
    // Skip
    return;
  }
  RAX_ASSERT(
      qking_value_strict_equal(this->get_js_type(), next_ele->get_js_type()));
  RAX_ASSERT(
      qking_value_strict_equal(this->get_js_key(), next_ele->get_js_key()));

  // to release value
  qking_value_t next_state = qking_create_undefined();
  qking_value_t prev_props = qking_create_undefined();
  qking_value_t prev_state = qking_create_undefined();

  try {
    qking_value_t next_props = next_ele->get_js_props();
    // componentWillReceiveProps
    {
      bool will_receive = false;
      if (prev_ele != next_ele) {
        will_receive = true;
      }

      if (will_receive) {
        // Calling this.setState() within componentWillReceiveProps will not
        // trigger an additional render.
        comp_pending_state() = true;
        perform_safe_to_queue(this, [=]() {
          return make_optional_call_on(
              class_inst,
              QKING_LIT_MAGIC_STRING_EX_COMPONENT_WILL_RECEIVE_PROPS,
              "ClassElement UpdateComponent", &next_props, 1);
        });

        comp_pending_state() = false;
      }
    }

    // Update refs
    rax_ref_update(prev_ele, next_ele);

    next_state = ProcessPendingState(next_props);
    RAX_ASSERT(!qking_value_is_error(next_state));

    bool should_update = true;
    prev_props = qking_acquire_value(prev_ele->get_js_props());
    prev_state =
        get_optional_property(class_inst, QKING_LIT_MAGIC_STRING_EX_STATE,
                              "ClassElement UpdateComponent", "class_inst");
    // ShouldComponentUpdate
    {
      bool has_function = false;
      qking_value_t args[2] = {next_props, next_state};
      qking_value_t should_update_ret = make_optional_call_on_ret(
          class_inst, QKING_LIT_MAGIC_STRING_EX_SHOULD_COMPONENT_UPDATE,
          "ClassElement UpdateComponent", has_function, args, 2);
      if (qking_value_is_error(should_update_ret)) {
        rax_handle_error(this, should_update_ret, true);
        qking_release_value(should_update_ret);
        should_update_ret = qking_create_undefined();
      }

      if (has_function) {
        should_update = qking_get_boolean_value(should_update_ret);
      } else {
        qking_value_t pure_class_func = qking_get_property_by_name_lit(
            class_inst, QKING_LIT_MAGIC_STRING_EX_IS_PURE_COMPONENT_CLASS);
        if (qking_value_is_function(pure_class_func)) {
          // is pure component
          should_update = !shallow_equal(prev_props, next_props) ||
                          !shallow_equal(prev_state, next_state);
        } else {
          // not pure or shouldComponentUpdate, no-op
        }
        qking_release_value(pure_class_func);
      }

      qking_release_value(should_update_ret);
    }

    RAX_LOGD("ClassComponent prev(%u) -> next(%u) shouldUpdate: %s",
             prev_ele->eid(), next_ele->eid(),
             should_update ? "true" : "false");
    if (should_update) {
      // componentWillUpdate
      {
        perform_safe_to_queue(this, [=]() {
          qking_value_t args[2] = {next_props, next_state};
          return make_optional_call_on(
              class_inst, QKING_LIT_MAGIC_STRING_EX_COMPONENT_WILL_UPDATE,
              "ClassElement UpdateComponent", args, 2);
        });
      }

      // using next ele content to replace current
      {
        CopyFromAnotherElement(next_ele);
        set_optional_property(class_inst, QKING_LIT_MAGIC_STRING_EX_STATE,
                              next_state, "ClassElement UpdateComponent",
                              "class_inst");
        set_optional_property(class_inst, QKING_LIT_MAGIC_STRING_EX_PROPS,
                              next_props, "ClassElement UpdateComponent",
                              "class_inst");
      }
      // render
      UpdateRenderedComponent(insert_start);

      // componentDidUpdate
      {
        perform_safe_to_queue(this, [=]() {
          qking_value_t args[2] = {prev_props, prev_state};
          return make_optional_call_on(
              class_inst, QKING_LIT_MAGIC_STRING_EX_COMPONENT_DID_UPDATE,
              "ClassElement UpdateComponent", args, 2);
        });
      }
    } else {
      // using next ele content to replace current
      CopyFromAnotherElement(next_ele);
      set_optional_property(class_inst, QKING_LIT_MAGIC_STRING_EX_STATE,
                            next_state, "ClassElement UpdateComponent",
                            "class_inst");
      set_optional_property(class_inst, QKING_LIT_MAGIC_STRING_EX_PROPS,
                            next_props, "ClassElement UpdateComponent",
                            "class_inst");
    }

    flush_err_queue(this);
    FlushCallbackQueue(this);

  } catch (const rax_exception &e) {
    qking_release_value(next_state);
    qking_release_value(prev_props);
    qking_release_value(prev_state);
    throw;
  }
  qking_release_value(next_state);
  qking_release_value(prev_props);
  qking_release_value(prev_state);
}

qking_value_t RaxClassElement::ProcessPendingState(qking_value_t public_props) {
  qking_value_t class_inst = get_class_instance();
  RAX_ASSERT(qking_value_is_object(class_inst));
  qking_value_t old_state = qking_get_property_by_name_lit(
      class_inst, QKING_LIT_MAGIC_STRING_EX_STATE);
  if (qking_value_is_error(old_state)) {
    RAX_LOGE("ClassElement ProcessPendingState: Get instance.state err: %s",
             string_from_qking_error(old_state).c_str());
    qking_release_value(old_state);
    old_state = qking_create_undefined();
  }

  if (state_queue_empty()) {
    return old_state;
  }

  qking_value_t ret_state = qking_create_object();
  qking_value_add_entries_from_object(ret_state, old_state);

  while (!state_queue_empty()) {
    qking_value_t new_partial_state = state_queue_pop();

    if (qking_value_is_function(new_partial_state)) {
      qking_value_t ret_new_state =
          qking_call_function(new_partial_state, class_inst, &public_props, 1);
      if (qking_value_is_error(old_state)) {
        RAX_LOGE(
            "ClassElement ProcessPendingState: call newState function err: %s",
            string_from_qking_error(old_state).c_str());
      } else {
        qking_value_add_entries_from_object(ret_state, ret_new_state);
      }
      qking_release_value(ret_new_state);
    } else {
      qking_value_add_entries_from_object(ret_state, new_partial_state);
    }
    qking_release_value(new_partial_state);
  }
  qking_release_value(old_state);
  return ret_state;
}

void RaxClassElement::UpdateRenderedComponent(uint32_t insert_start) {
  RaxElement *prev_rendered_ele = get_rendered_comp();

  get_factory()->owner() = this;
  RaxElement *next_rendered_ele = CallRenderOnInstance(nullptr);
  get_factory()->owner() = nullptr;

  bool should_update =
      ShouldUpdateComponent(prev_rendered_ele, next_rendered_ele);

  if (should_update) {
    if (prev_rendered_ele != next_rendered_ele) {
      prev_rendered_ele->UpdateComponent(prev_rendered_ele, next_rendered_ele,
                                         insert_start);
    }
  } else {
    prev_rendered_ele->UnmountComponent(false);
    native_node_ptr render_parent = get_comp_render_parent();
    next_rendered_ele->MountComponent(
        render_parent, this,
        [&insert_start, render_parent, this](native_node_ptr parent,
                                             native_node_ptr child) {
          RAX_ASSERT(parent && child);
          RAX_ASSERT(render_parent == parent);
          if (is_mount_to_root()) {
            render_bridge::InsertNode(parent, child, insert_start);
          } else {
            render_bridge::NativeNodeInsertNode(parent, child, insert_start);
          }
          insert_start++;
        });
    set_rendered_comp(next_rendered_ele);
  }
}

void RaxClassElement::DebugPrintComponent() {
  RaxElement::DebugPrintComponent();
  RAX_ASSERT(get_rendered_comp());
  RAX_LOGD("RaxComponent(%u): Class, Rendered: (%u)", eid(),
           get_rendered_comp()->eid());
}
native_node_ptr RaxClassElement::get_first_render_node() {
  RAX_ASSERT(get_rendered_comp());
  return get_rendered_comp()->get_first_render_node();
}
native_node_ptr RaxClassElement::get_last_render_node() {
  RAX_ASSERT(get_rendered_comp());
  return get_rendered_comp()->get_last_render_node();
}

Json::object RaxClassElement::DebugCollectComponent() {
  RAX_ASSERT(get_rendered_comp());
  Json::object obj = RaxElement::DebugCollectComponent();
  obj["child"] = get_rendered_comp()->DebugCollectComponent();
  return obj;
}

RAX_NAME_SPACE_END
