//
// Created by Xu Jiacheng on 2018/12/25.
//

#include "rax_builtin_env.h"
#include "base/qking_common_logger.h"
#include "rax_class_element.h"
#include "rax_core_util.h"
#include "rax_element_factory.h"
#include "rax_native_transform.h"
#include "rax_ref.h"
#include "rax_render_bridge.h"
#include "rax_root_element.h"
#include "rax_update.h"

RAX_NAME_SPACE_BEGIN

static qking_value_t CreateElement(const qking_value_t function_obj,
                                   const qking_value_t this_val,
                                   const qking_value_t args_p[],
                                   const qking_length_t args_count) {
  //  RAX_LOGD("Builtin: CreateElement()");

  COMMON_START();

  qking_value_t key = qking_create_null();
  qking_value_t ref = qking_create_null();
  qking_value_t props = qking_create_object();
  qking_value_t children_array = qking_create_undefined();

  COMMON_RESOURCE();

  THROW_JS_ERR_IF(args_count < 2, "CreateElement: args less than 2");

  qking_value_t type = args_p[0];
  THROW_JS_ERR_IF(qking_value_is_null_or_undefined(type),
                  "CreateElement: type should not be null or undefined");

  qking_value_t config = args_p[1];

  // props, ref and key
  if (!qking_value_is_null_or_undefined(config)) {
    THROW_JS_ERR_IF(!qking_value_is_object(config),
                    "CreateElement: config is not a object");

    key = qking_get_property_by_name_lit(config, QKING_LIT_MAGIC_STRING_EX_KEY);
    CHECK_RESOURCE(key);
    if (qking_value_is_undefined(key)) {
      key = qking_create_null();
    } else {
      qking_value_t tmp = key;
      key = qking_value_to_string(key);
      qking_release_value(tmp);
      CHECK_RESOURCE(key);
    }

    ref = qking_get_property_by_name_lit(config, QKING_LIT_MAGIC_STRING_EX_REF);
    CHECK_RESOURCE(ref);
    if (qking_value_is_undefined(ref)) {
      ref = qking_create_null();
    }

    // props = config filter out key and ref.
    struct Passer {
      qking_value_t update_prop;
      qking_value_t key_string;
      qking_value_t ref_string;
    } tmp_pass{props, qking_create_string_lit(QKING_LIT_MAGIC_STRING_EX_KEY),
               qking_create_string_lit(QKING_LIT_MAGIC_STRING_EX_REF)};

    qking_foreach_object_property(
        config,
        [](const qking_value_t property_name,
           const qking_value_t property_value, void *user_data_p) {
          Passer *passer = (Passer *)user_data_p;
          qking_value_t update_prop = passer->update_prop;
          // ignore err if set failed
          if (qking_value_strict_equal(passer->key_string, property_name) ||
              qking_value_strict_equal(passer->ref_string, property_name)) {
            // skip ref or key
            return true;
          }
          qking_release_value(
              qking_set_property(update_prop, property_name, property_value));
          return true;
        },
        &tmp_pass);

    qking_release_value(tmp_pass.key_string);
    qking_release_value(tmp_pass.ref_string);
  }

  // children flatten
  children_array = rax_flatten_children(args_p + 2, args_count - 2);
  if (!qking_value_is_undefined(children_array)) {
    // has child.
    qking_release_value(qking_set_property_by_name_lit(
        props, QKING_LIT_MAGIC_STRING_EX_CHILDREN, children_array));
  } else {
    // using exist props.children.
    children_array = qking_get_property_by_name_lit(
        props, QKING_LIT_MAGIC_STRING_EX_CHILDREN);
  }

  // resolve default props
  if (qking_value_is_object(type)) {
    qking_value_t default_props =
        qking_get_property_by_name(type, "defaultProps");
    if (qking_value_is_error(default_props) ||
        qking_value_is_undefined(default_props) ||
        !qking_value_is_object(default_props)) {
      // no props
    } else {
      qking_foreach_object_property_of(
          default_props,
          [](const qking_value_t property_name,
             const qking_value_t property_value, void *user_data_p) -> bool {
            qking_value_t raw_props = *((qking_value_t *)user_data_p);
            qking_value_t old_prop =
                qking_get_property(raw_props, property_name);

            if (qking_value_is_undefined(old_prop)) {
              // no user prop, use default.
              qking_release_value(
                  qking_set_property(raw_props, property_name, property_value));
            } else {
              // has user prop, no-op
            }
            qking_release_value(old_prop);
            return true;
          },
          &props, false, true, false);
    }
    qking_release_value(default_props);
  }

  // flatten style
  {
    qking_value_t style_prop =
        qking_create_string_lit(QKING_LIT_MAGIC_STRING_EX_STYLE);
    qking_value_t old_style = qking_get_property(props, style_prop);
    CHECK_TMP_VAR_RELEASE(old_style, style_prop);
    qking_value_t new_style = rax_flatten_style(old_style);
    qking_release_value(old_style);
    CHECK_TMP_VAR_RELEASE(new_style, style_prop);
    qking_release_value(qking_set_property(props, style_prop, new_style));
    qking_release_value(new_style);
    qking_release_value(style_prop);
  }

  const std::string &page_id = rax_get_current_page_name();
  RAX_ASSERT(!page_id.empty());
  RaxElementFactory *factory = RaxElementFactory::GetFactory(page_id);
  RAX_ASSERT(factory);
  RaxElement *native_ele =
      factory->CreateElement(type, props, children_array, key, ref);
  THROW_JS_ERR_IF(!native_ele, "CreateElement: Invalid element type");

  native_ele->set_comp_owner(factory->owner());
  rax_create_element_filter_props(native_ele);

  qking_value_t final_value = factory->MakeJSValue(native_ele);
  func_ret = final_value;

  COMMON_FINALIZE();

  qking_release_value(key);
  qking_release_value(ref);
  qking_release_value(props);
  qking_release_value(children_array);

  COMMON_END();
}

static qking_value_t CloneElement(const qking_value_t function_obj,
                                  const qking_value_t this_val,
                                  const qking_value_t args_p[],
                                  const qking_length_t args_count) {
  //  RAX_LOGD("Builtin: CloneElement()");

  const std::string &page_id = rax_get_current_page_name();
  RAX_ASSERT(!page_id.empty());
  RaxElementFactory *factory = RaxElementFactory::GetFactory(page_id);
  RAX_ASSERT(factory);

  COMMON_START();

  qking_value_t type = qking_create_null();
  qking_value_t key = qking_create_null();
  qking_value_t ref = qking_create_null();
  qking_value_t props = qking_create_object();
  qking_value_t children_array = qking_create_undefined();

  COMMON_RESOURCE();

  THROW_JS_ERR_IF(args_count < 1, "CloneElement: args less than 1");

  qking_value_t element_value = args_p[0];
  RaxElement *element = factory->ExtractElement(element_value);
  THROW_JS_ERR_IF(!element, "CloneElement: not a valid element");

  type = qking_acquire_value(element->get_js_type());

  // Original props are copied
  qking_value_add_entries_from_object(props, element->get_js_props());

  key = qking_acquire_value(element->get_js_key());
  ref = qking_acquire_value(element->get_js_ref());
  RaxElement *owner = element->get_comp_owner();

  if (args_count >= 2) {
    qking_value_t config = args_p[1];

    // props, ref and key
    if (!qking_value_is_null_or_undefined(config)) {
      THROW_JS_ERR_IF(!qking_value_is_object(config),
                      "CloneElement: config is not a object");
      qking_value_t new_ref =
          qking_get_property_by_name_lit(config, QKING_LIT_MAGIC_STRING_EX_REF);
      CHECK_TMP_VAR(new_ref);
      if (!qking_value_is_undefined(new_ref)) {
        // Should reset ref and owner if has a new ref
        qking_release_value(ref);
        ref = new_ref;
        owner = factory->owner();
      } else {
        qking_release_value(new_ref);
      }

      qking_value_t new_key =
          qking_get_property_by_name_lit(config, QKING_LIT_MAGIC_STRING_EX_KEY);
      CHECK_TMP_VAR(new_key);
      if (!qking_value_is_undefined(new_key)) {
        qking_value_t tmp = new_key;
        new_key = qking_value_to_string(new_key);
        qking_release_value(tmp);
        CHECK_TMP_VAR(new_key);
        key = new_key;
      } else {
        qking_release_value(new_key);
      }

      // props = config filter out key and ref.
      struct Passer {
        qking_value_t update_prop;
        qking_value_t key_string;
        qking_value_t ref_string;
      } tmp_pass{props, qking_create_string_lit(QKING_LIT_MAGIC_STRING_EX_KEY),
                 qking_create_string_lit(QKING_LIT_MAGIC_STRING_EX_REF)};

      qking_foreach_object_property(
          config,
          [](const qking_value_t property_name,
             const qking_value_t property_value, void *user_data_p) {
            Passer *passer = (Passer *)user_data_p;
            qking_value_t update_prop = passer->update_prop;
            // ignore err if set failed
            if (qking_value_strict_equal(passer->key_string, property_name) ||
                qking_value_strict_equal(passer->ref_string, property_name)) {
              // skip ref or key
              return true;
            }
            qking_release_value(
                qking_set_property(update_prop, property_name, property_value));
            return true;
          },
          &tmp_pass);

      qking_release_value(tmp_pass.key_string);
      qking_release_value(tmp_pass.ref_string);

      // resolve default props
      if (qking_value_is_object(type)) {
        qking_value_t default_props =
            qking_get_property_by_name(type, "defaultProps");
        if (qking_value_is_error(default_props) ||
            qking_value_is_undefined(default_props) ||
            !qking_value_is_object(default_props)) {
          // no props
        } else {
          qking_foreach_object_property_of(
              default_props,
              [](const qking_value_t property_name,
                 const qking_value_t property_value,
                 void *user_data_p) -> bool {
                qking_value_t raw_props = *((qking_value_t *)user_data_p);
                qking_value_t old_prop =
                    qking_get_property(raw_props, property_name);

                if (qking_value_is_undefined(old_prop)) {
                  // no user prop, use default.
                  qking_release_value(qking_set_property(
                      raw_props, property_name, property_value));
                } else {
                  // has user prop, no-op
                }
                qking_release_value(old_prop);
                return true;
              },
              &props, false, true, false);
        }
        qking_release_value(default_props);
      }
    }
  }

  // children flatten
  if (args_count <= 2) {
    children_array = qking_create_undefined();
  } else {
    children_array = rax_flatten_children(args_p + 2, args_count - 2);
  }
  if (!qking_value_is_undefined(children_array)) {
    // has child.
    qking_release_value(qking_set_property_by_name_lit(
        props, QKING_LIT_MAGIC_STRING_EX_CHILDREN, children_array));
  } else {
    // using exist props.children.
    children_array = qking_get_property_by_name_lit(
        props, QKING_LIT_MAGIC_STRING_EX_CHILDREN);
  }

  RaxElement *native_ele =
      factory->CreateElement(type, props, children_array, key, ref);
  THROW_JS_ERR_IF(!native_ele, "CreateElement: Invalid element type");
  native_ele->set_comp_owner(owner);
  rax_create_element_filter_props(native_ele);

  qking_value_t final_value = factory->MakeJSValue(native_ele);
  func_ret = final_value;

  COMMON_FINALIZE();

  qking_release_value(type);
  qking_release_value(key);
  qking_release_value(ref);
  qking_release_value(props);
  qking_release_value(children_array);

  COMMON_END();
}

static qking_value_t Render(const qking_value_t function_obj,
                            const qking_value_t this_val,
                            const qking_value_t args_p[],
                            const qking_length_t args_count) {
  RAX_LOGD("Builtin: Render()");
  const std::string &page_id = rax_get_current_page_name();
  RAX_ASSERT(!page_id.empty());
  RaxElementFactory *factory = RaxElementFactory::GetFactory(page_id);
  RAX_ASSERT(factory);

  COMMON_START();

  bool root_op = factory->increase_op_level() == 0;

  COMMON_RESOURCE();

  THROW_JS_ERR_IF(args_count != 1, "Render: Invalid argument count");

  THROW_JS_ERR_IF(factory->root(), "Render: Render twice, already has a root");

  RaxEleNativeType *element = nullptr;
  // todo. must be a Element of Class, Native, Function. but Rax also support
  // fragment, which we don't
  THROW_JS_ERR_IF(
      !qking_get_object_native_pointer(args_p[0], (void **)&element, nullptr),
      "Render: arg[0] is not a element");
  RaxElement *extract_element = factory->ExtractElement(element);
  RAX_MAKESURE(extract_element);

  RaxRootElement *root = factory->CreateRootElement();
  root->child() = extract_element;
  factory->root() = root;
  native_node_ptr render_root = render_bridge::NativeNodeCreateRootNode();
  root->native_node() = render_root;

  root->MountComponent(render_root, nullptr, nullptr);

  render_bridge::SetRootNode(render_root);

  // todo. Rax returns native node for NativeElement and array for
  // FragmentElement.
  if (extract_element->type() == RaxElementType::kClass) {
    func_ret = qking_acquire_value(
        ((RaxClassElement *)extract_element)->get_class_instance());
  } else if (extract_element->type() == RaxElementType::kFunction) {
    func_ret = qking_create_null();
  }

  if (root_op) {
    factory->CleanUpUnmountedComponent();
  }

  COMMON_FINALIZE();

  factory->decrease_op_level();

  COMMON_END();
}

static qking_value_t SetState(const qking_value_t function_obj,
                              const qking_value_t this_val,
                              const qking_value_t args_p[],
                              const qking_length_t args_count) {
  RAX_LOGD("Builtin: SetState()");

  const std::string &page_id = rax_get_current_page_name();
  RAX_ASSERT(!page_id.empty());
  RaxElementFactory *factory = RaxElementFactory::GetFactory(page_id);
  RAX_ASSERT(factory);

  COMMON_START();

  bool root_op = factory->increase_op_level() == 0;

  COMMON_RESOURCE();

  // component_class_instance,  new_partial_state, callback
  THROW_JS_ERR_IF(args_count < 2, "SetState: args num less than 2");

  qking_value_t component_class_instance = args_p[0];
  qking_value_t partial_state = args_p[1];
  RaxElement *ele_to_update = nullptr;
  {
    qking_value_t native_ele_obj = qking_get_property_by_name_lit(
        component_class_instance, QKING_LIT_MAGIC_STRING_EX_UD_INTERNAL);
    CHECK_TMP_VAR(native_ele_obj);
    RaxEleNativeType *eleNativeType = nullptr;
    THROW_JS_ERR_IF(
        !qking_get_object_native_pointer(native_ele_obj,
                                         (void **)&eleNativeType, nullptr),
        "SetState: setState on a component with invalid '_internal' stub");
    ele_to_update = factory->ExtractElement(native_ele_obj);
    qking_release_value(native_ele_obj);
  }

  THROW_JS_ERR_IF(!ele_to_update, "SetState: RaxElement already released");
  THROW_JS_ERR_IF(ele_to_update->type() != RaxElementType::kClass,
                  "SetState: SetState on a none-class Component");
  RaxClassElement *class_element = (RaxClassElement *)ele_to_update;

  // enqueueState(internal, partialState);
  class_element->state_queue_push(partial_state);

  // enqueueCallback(internal, callback);
  if (args_count >= 3) {
    qking_value_t callback_func = args_p[2];
    if (qking_value_is_function(callback_func)) {
      class_element->callback_queue_push(callback_func);
    }
  }

  bool has_rendered_comp = class_element->get_rendered_comp();
  if (has_rendered_comp && !class_element->comp_pending_state() &&
      !class_element->state_queue_empty()) {
    // stash callback queue
    std::queue<std::unique_ptr<QKValueRef>> callback_queue;
    class_element->callback_queue_move(callback_queue);

    // update component
    g_by_state = 1;
    // root ele has only 1 child, insert_start is 0;
    class_element->UpdateComponent(class_element, class_element, 0);

    // flush callback queue
    FlushCallbackQueue(callback_queue, class_element);

    RAX_ASSERT(g_by_state == 0);
    if (root_op) {
      factory->CleanUpUnmountedComponent();
    }
  } else {
    RAX_LOGD(
        "Builtin: SetState UpdateComponent skipped: no_rendered_comp=%s, "
        "comp_pending_state=%s, state_queue_empty=%s",
        has_rendered_comp ? "pass" : "skip",
        class_element->comp_pending_state() ? "skip" : "pass",
        class_element->state_queue_empty() ? "skip" : "pass");
  }

  COMMON_FINALIZE();

  factory->decrease_op_level();

  COMMON_END();
}

static qking_value_t RegisterDocumentEventHandler(
    const qking_value_t function_obj, const qking_value_t this_val,
    const qking_value_t args_p[], const qking_length_t args_count) {
  RAX_LOGD("Builtin: RegisterGlobalEventHandler()");

  COMMON_START();

  COMMON_RESOURCE();

  THROW_JS_ERR_IF(args_count != 1,
                  "RegisterGlobalEventHandler: Invalid argument count");

  const std::string &page_id = rax_get_current_page_name();
  RAX_ASSERT(!page_id.empty());
  RaxElementFactory *factory = RaxElementFactory::GetFactory(page_id);
  RAX_ASSERT(factory);

  THROW_JS_ERR_IF(
      !qking_value_is_function(args_p[0]),
      "RegisterDocumentEventHandler: argument needs to be a function");

  factory->set_document_event_handler(args_p[0]);

  COMMON_FINALIZE();

  COMMON_END();
}

static qking_value_t FindDOMNode(const qking_value_t function_obj,
                                 const qking_value_t this_val,
                                 const qking_value_t args_p[],
                                 const qking_length_t args_count) {
  const std::string &page_id = rax_get_current_page_name();
  RAX_ASSERT(!page_id.empty());
  RaxElementFactory *factory = RaxElementFactory::GetFactory(page_id);
  RAX_ASSERT(factory);

  COMMON_START();

  COMMON_RESOURCE();

  THROW_JS_ERR_IF(args_count != 1, "FindDOMNode: Invalid argument count");
  qking_value_t instance = args_p[0];
  if (qking_value_is_null_or_undefined(instance)) {
    COMMON_RETURN(qking_create_null());
  }

  if (qking_value_is_string(instance)) {
    // todo should search through native_node.
    COMMON_RETURN(rax_create_native_node_obj_by_str(instance));
  }

  if (qking_value_is_object(instance)) {
    // native node
    qking_value_t node_type_name = qking_create_string_c("nodeType");
    if (qking_get_boolean_value(qking_has_property(instance, node_type_name))) {
      qking_release_value(node_type_name);
      COMMON_RETURN(qking_acquire_value(instance));
    }
    qking_release_value(node_type_name);

    qking_value_t internal_name =
        qking_create_string_lit(QKING_LIT_MAGIC_STRING_EX_UD_INTERNAL);
    if (!qking_get_boolean_value(qking_has_property(instance, internal_name))) {
      qking_release_value(internal_name);
      THROW_JS_ERR_IF(true,
                      "findDOMNode: find by neither component nor DOM node.");
    }
    qking_value_t internal_obj = qking_get_property(instance, internal_name);
    qking_release_value(internal_name);

    RaxElement *element = factory->ExtractElement(internal_obj);
    qking_release_value(internal_obj);
    THROW_JS_ERR_IF(!element,
                    "findDOMNode: find by neither component nor DOM node.");

    // native component
    if (element->is_native_holder_component()) {
      COMMON_RETURN(qking_acquire_value(
          dynamic_cast<RaxNativeNodeHolder *>(element)->get_js_native_node()));
    } else if (element->is_composite_component()) {
      RaxRenderCompHolder *comp_it =
          dynamic_cast<RaxRenderCompHolder *>(element);
      RaxElement *internal = comp_it->get_rendered_comp();
      THROW_JS_ERR_IF(!internal,
                      "findDOMNode: find on an unmounted component.");
      while (!internal->is_native_holder_component()) {
        if (internal->is_composite_component()) {
          internal = dynamic_cast<RaxRenderCompHolder *>(internal)
                         ->get_rendered_comp();

          if (internal == nullptr) {
            COMMON_RETURN(qking_create_null());
          }
        } else {
          COMMON_RETURN(qking_create_null());
        }
      }
      RAX_ASSERT(internal->is_native_holder_component());
      COMMON_RETURN(qking_acquire_value(
          dynamic_cast<RaxNativeNodeHolder *>(internal)->get_js_native_node()));
    }
  }

  COMMON_FINALIZE();

  COMMON_END();
}

static void UpdateNativeProps(native_node_ptr render_node,
                              RaxPropsHolder *props_element,
                              RaxElement *element, qking_value_t props) {
  bool mounted = element->is_mount_to_root();
  std::vector<std::pair<std::string, std::string>> *styles =
      new std::vector<std::pair<std::string, std::string>>();
  std::vector<std::pair<std::string, std::string>> *attrs =
      new std::vector<std::pair<std::string, std::string>>();
  props_element->SetNativeProps(
      props,
      [styles](const std::string &key, const std::string &value) {
        styles->push_back({key, value});
      },
      [attrs](const std::string &key, const std::string &value) {
        attrs->push_back({key, value});
      },
      [render_node, props_element, mounted](const std::string &event,
                                            qking_value_t value) {
        // to support event , we has to modify element.
        props_element->set_event(event, value);
        if (mounted) {
          render_bridge::AddEvent(render_node, event);
        } else {
          render_bridge::NativeNodeAddEvent(render_node, event);
        }
      });
  if (mounted) {
    render_bridge::UpdateStyle(render_node, styles);
    render_bridge::UpdateAttr(render_node, attrs);
  } else {
    render_bridge::NativeNodeUpdateStyle(render_node, styles);
    render_bridge::NativeNodeUpdateAttr(render_node, attrs);
  }
}

static void ProcessRefAndReturn(RaxElementFactory *factory, qking_value_t props,
                                qking_value_t &func_ret,
                                qking_value_t ref_str_value) {
  native_node_ptr render_node = nullptr;
  RaxPropsHolder *props_element = nullptr;
  const std::string &ref_str = string_from_qking_string_value((ref_str_value));
  RaxElement *ele = factory->GetComponent(ref_str);
  if (!ele) {
    THROW_JS_ERR_IF(true, "setNativeProps: node not found.");
  }
  if (ele->is_native_holder_component() && ele->is_props_holder_component()) {
    props_element = dynamic_cast<RaxPropsHolder *>(ele);
    render_node = dynamic_cast<RaxNativeNodeHolder *>(ele)->native_node();
    UpdateNativeProps(render_node, props_element, ele, props);
    COMMON_RETURN(qking_create_undefined());
  } else {
    THROW_JS_ERR_IF(true, "setNativeProps: node is not a native_node.");
  }
}

static qking_value_t SetNativeProps(const qking_value_t function_obj,
                                    const qking_value_t this_val,
                                    const qking_value_t args_p[],
                                    const qking_length_t args_count) {
  const std::string &page_id = rax_get_current_page_name();
  RAX_ASSERT(!page_id.empty());
  RaxElementFactory *factory = RaxElementFactory::GetFactory(page_id);
  RAX_ASSERT(factory);

  COMMON_START();

  qking_value_t ref_value = qking_create_undefined();

  COMMON_RESOURCE();

  THROW_JS_ERR_IF(args_count != 2, "setNativeProps: Invalid argument count");
  qking_value_t instance = args_p[0];
  qking_value_t props = args_p[1];

  if (qking_value_is_null_or_undefined(instance)) {
    COMMON_RETURN(qking_create_undefined());
  }
  THROW_JS_ERR_IF(!qking_value_is_object(props),
                  "setNativeProps: props not an object");

  if (qking_value_is_string(instance)) {
    ProcessRefAndReturn(factory, props, func_ret, instance);
  } else if (qking_value_is_object(instance)) {
    // native node
    qking_value_t node_type_name = qking_create_string_c("nodeType");
    if (qking_get_boolean_value(qking_has_property(instance, node_type_name))) {
      qking_release_value(node_type_name);
      // is a ref node it self.
      ref_value = qking_get_property_by_name(instance, "ref");
      if (qking_value_is_string(ref_value)) {
        ProcessRefAndReturn(factory, props, func_ret, ref_value);
      } else {
        COMMON_RETURN(qking_create_undefined());
      }
    }
    qking_release_value(node_type_name);

    qking_value_t internal_name =
        qking_create_string_lit(QKING_LIT_MAGIC_STRING_EX_UD_INTERNAL);
    if (!qking_get_boolean_value(qking_has_property(instance, internal_name))) {
      qking_release_value(internal_name);
      THROW_JS_ERR_IF(
          true, "setNativeProps: find by neither component nor DOM node.");
    }
    qking_value_t internal_obj = qking_get_property(instance, internal_name);
    qking_release_value(internal_name);

    RaxElement *element = factory->ExtractElement(internal_obj);
    qking_release_value(internal_obj);
    THROW_JS_ERR_IF(!element,
                    "setNativeProps: find by neither component nor DOM node.");

    // native component
    if (element->is_native_holder_component()) {
      ref_value = qking_create_string_c(element->eid_str().c_str());
      ProcessRefAndReturn(factory, props, func_ret, ref_value);
    } else if (element->is_composite_component()) {
      RaxRenderCompHolder *comp_it =
          dynamic_cast<RaxRenderCompHolder *>(element);
      RaxElement *internal = comp_it->get_rendered_comp();
      THROW_JS_ERR_IF(!internal,
                      "setNativeProps: find on an unmounted component.");
      while (!internal->is_native_holder_component()) {
        if (internal->is_composite_component()) {
          internal = dynamic_cast<RaxRenderCompHolder *>(internal)
                         ->get_rendered_comp();

          if (internal == nullptr) {
            COMMON_RETURN(qking_create_null());
          }
        } else {
          COMMON_RETURN(qking_create_null());
        }
      }
      RAX_ASSERT(internal->is_native_holder_component());
      ref_value = qking_create_string_c(element->eid_str().c_str());
      ProcessRefAndReturn(factory, props, func_ret, ref_value);
    }
  }

  COMMON_FINALIZE();

  qking_release_value(ref_value);

  COMMON_END();
}

static qking_value_t CreateRef(const qking_value_t function_obj,
                               const qking_value_t this_val,
                               const qking_value_t args_p[],
                               const qking_length_t args_count) {
  qking_value_t obj = qking_create_object();
  qking_release_value(
      qking_set_property_by_name(obj, "current", qking_create_null()));
  return obj;
}

void qking_rax_register_builtin_env() {
  qking_external_handler_register_global("__createElement", CreateElement);
  qking_external_handler_register_global("__cloneElement", CloneElement);
  qking_external_handler_register_global("__render", Render);
  qking_external_handler_register_global("__setState", SetState);
  qking_external_handler_register_global("__createRef", CreateRef);
  qking_external_handler_register_global("__setNativeProps", SetNativeProps);
  qking_external_handler_register_global("__findDOMNode", FindDOMNode);
  qking_external_handler_register_global("__registerDocumentEventHandler",
                                         RegisterDocumentEventHandler);
}

RAX_NAME_SPACE_END
