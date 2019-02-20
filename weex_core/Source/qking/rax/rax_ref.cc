//
// Created by Xu Jiacheng on 2019-01-22.
//

#include "rax_ref.h"
#include "rax_class_element.h"
#include "rax_native_element.h"
RAX_NAME_SPACE_BEGIN

qking_value_t rax_get_public_instance(RaxElement *component) {
  qking_value_t instance;
  RaxElementType type = component->type();
  if (type == RaxElementType::kClass) {
    RaxClassElement *class_ele = dynamic_cast<RaxClassElement *>(component);
    instance = class_ele->get_class_instance();
  } else if (type == RaxElementType::kNative) {
    RaxNativeElement *native_ele = dynamic_cast<RaxNativeElement *>(component);
    instance = native_ele->get_js_native_node();
  } else {
    instance = qking_create_null();
  }
  // text and func don't have getPublicInstance interface in rax.
  return qking_acquire_value(instance);
}

static void rax_add_ref_to_instance(RaxElement *owner, qking_value_t ref,
                                    qking_value_t instance) {
  if (owner->type() != RaxElementType::kClass) {
    return;
  }

  RaxClassElement *owner_ele = dynamic_cast<RaxClassElement *>(owner);
  qking_value_t class_instance = owner_ele->get_class_instance();
  if (!qking_value_is_object(class_instance)) {
    return;
  }

  qking_value_t refs = qking_get_property_by_name(class_instance, "refs");
  if (!qking_value_is_object(refs)) {
    qking_release_value(refs);
    return;
  }

  qking_release_value(qking_set_property(refs, ref, instance));
  qking_release_value(refs);
}

static void rax_try_remove_ref_from_instance(RaxElement *owner,
                                             qking_value_t ref,
                                             qking_value_t instance) {
  if (owner->type() != RaxElementType::kClass) {
    return;
  }

  RaxClassElement *owner_ele = dynamic_cast<RaxClassElement *>(owner);
  qking_value_t class_instance = owner_ele->get_class_instance();
  if (!qking_value_is_object(class_instance)) {
    return;
  }

  qking_value_t refs = qking_get_property_by_name(class_instance, "refs");
  if (!qking_value_is_object(refs)) {
    qking_release_value(refs);
    return;
  }

  qking_value_t old = qking_get_property(refs, ref);
  if (qking_value_strict_equal(old, instance)) {
    qking_delete_property(refs, ref);
  }
  qking_release_value(old);
  qking_release_value(refs);
}

void rax_ref_update(RaxElement *prev, RaxElement *next) {
  qking_value_t prev_ref = prev->get_js_ref();
  qking_value_t next_ref = next->get_js_ref();

  if (!qking_value_strict_equal(prev_ref, next_ref)) {
    if (!qking_value_is_null_or_undefined(prev_ref)) {
      rax_ref_detach(prev->get_comp_owner(), prev_ref, prev);
    }

    if (!qking_value_is_null_or_undefined(next_ref)) {
      rax_ref_attach(next->get_comp_owner(), next_ref, prev);
    }
  }
}

void rax_ref_attach(RaxElement *owner, qking_value_t ref,
                    RaxElement *component) {
  if (!owner) {
    throw rax_common_err(
        "You might be adding a ref to a component that was not created inside a"
        " component\\'s `render` method, or you have multiple copies of Rax "
        "loaded.");
  }

  qking_value_t instance = rax_get_public_instance(component);
  if (qking_value_is_function(ref)) {
    qking_release_value(
        qking_call_function(ref, qking_create_undefined(), &instance, 1));
  } else if (qking_value_is_object(ref)) {
    qking_release_value(qking_set_property_by_name(ref, "current", instance));
  } else {
    rax_add_ref_to_instance(owner, ref, instance);
  }
  qking_release_value(instance);
}

void rax_ref_detach(RaxElement *owner, qking_value_t ref,
                    RaxElement *component) {
  if (qking_value_is_function(ref)) {
    qking_value_t null_value = qking_create_null();
    qking_release_value(
        qking_call_function(ref, qking_create_undefined(), &null_value, 1));
  } else {
    qking_value_t instance = rax_get_public_instance(component);

    bool is_obj_ref = false;
    if (qking_value_is_object(ref)) {
      qking_value_t inner_instance = qking_get_property_by_name(ref, "current");
      is_obj_ref = qking_value_strict_equal(inner_instance, instance);
      qking_release_value(inner_instance);
    }

    if (is_obj_ref) {
      qking_release_value(
          qking_set_property_by_name(ref, "current", qking_create_null()));
    } else {
      rax_try_remove_ref_from_instance(owner, ref, instance);
    }

    qking_release_value(instance);
  }
}
qking_value_t make_native_node_ojb(qking_value_t str) {
  qking_value_t obj = qking_create_object();
  qking_release_value(qking_set_property_by_name(obj, "ref", str));
  qking_release_value(
      qking_set_property_by_name(obj, "nodeType", qking_create_boolean(true)));
  return obj;
}

qking_value_t rax_create_native_node_obj(const std::string &ref_value) {
  qking_value_t ref_s = qking_create_string_c(ref_value.c_str());
  qking_value_t obj = make_native_node_ojb(ref_s);
  qking_release_value(ref_s);
  return obj;
}
qking_value_t rax_create_native_node_obj_by_str(qking_value_t str) {
  RAX_ASSERT(qking_value_is_string(str));
  return make_native_node_ojb(str);
}

RAX_NAME_SPACE_END
