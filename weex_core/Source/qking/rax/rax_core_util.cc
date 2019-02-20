//
// Created by Xu Jiacheng on 2018/12/25.
//

#include "rax_core_util.h"
#include "base/qking_common_logger.h"

RAX_NAME_SPACE_BEGIN
const std::string ROOT_REF = "_root";

static void push_all_recursive(qking_value_t src, qking_value_t dest,
                               qking_length_t &index) {
  if (qking_value_is_array(src)) {
    uint32_t array_length = qking_get_array_length(src);
    for (uint32_t i = 0; i < array_length; ++i) {
      qking_value_t a_var = qking_get_property_by_index(src, i);
      if (qking_value_is_error(a_var)) {
        qking_release_value(a_var);
        RAX_LOGE("Error push_all_recursive");
        break;
      }
      push_all_recursive(a_var, dest, index);
      qking_release_value(a_var);
    }
  } else {
    qking_release_value(qking_set_property_by_index(dest, index, src));
    index++;
  }
}

qking_value_t rax_flatten_children(const qking_value_t *array,
                                   qking_length_t size) {
  if (size == 0) {
    return qking_create_undefined();
  }
  // only child not array
  if (size == 1 && !qking_value_is_array(array[0])) {
    return qking_acquire_value(array[0]);
  }

  qking_value_t result_array = qking_create_array(0);
  qking_length_t index = 0;
  for (qking_length_t i = 0; i < size; ++i) {
    qking_value_t it = array[i];
    push_all_recursive(it, result_array, index);
  }
  return result_array;
}

qking_value_t rax_flatten_style(qking_value_t value) {
  if (qking_value_is_null_or_undefined(value)) {
    return qking_create_undefined();
  }
  // make a copy
  qking_value_t new_style = qking_create_object();
  qking_value_add_entries_from_object(new_style, value);
  return new_style;
}

qking_value_t make_optional_call_on(qking_value_t class_inst,
                                    qking_magic_str_t func_name,
                                    const char *process_msg,
                                    const qking_value_t *args_p,
                                    qking_size_t args_count) {
  // componentWillMount
  qking_value_t func_by_name =
      qking_get_property_by_name_lit(class_inst, func_name);
  qking_value_t ret = qking_create_undefined();
  if (qking_value_is_error(func_by_name)) {
    RAX_LOGW("%s: Get func %s err: %s", process_msg,
             string_from_qking_string_value_lit(func_name).c_str(),
             string_from_qking_error(func_by_name).c_str());
    ret = qking_acquire_value(func_by_name);
  } else if (qking_value_is_function(func_by_name)) {
    ret = qking_call_function(func_by_name, class_inst, args_p, args_count);
    if (qking_value_is_error(ret)) {
      RAX_LOGW("%s: Call %s err: %s", process_msg,
               string_from_qking_string_value_lit(func_name).c_str(),
               string_from_qking_error(ret).c_str());
    }
  } /* else { <not found> } */

  qking_release_value(func_by_name);
  return ret;
}

qking_value_t make_optional_call_on_ret(qking_value_t class_inst,
                                        qking_magic_str_t func_name,
                                        const char *process_msg,
                                        bool &has_function,
                                        const qking_value_t *args_p,
                                        qking_size_t args_count) {
  // componentWillMount
  qking_value_t ret = qking_create_undefined();
  qking_value_t func_by_name =
      qking_get_property_by_name_lit(class_inst, func_name);

  if (qking_value_is_error(func_by_name)) {
    RAX_LOGW("%s: Get func %s err: %s", process_msg,
             string_from_qking_string_value_lit(func_name).c_str(),
             string_from_qking_error(func_by_name).c_str());
    ret = qking_acquire_value(func_by_name);
    has_function = false;
  } else if (qking_value_is_function(func_by_name)) {
    ret = qking_call_function(func_by_name, class_inst, args_p, args_count);
    if (qking_value_is_error(ret)) {
      RAX_LOGW("%s: Call %s err: %s", process_msg,
               string_from_qking_string_value_lit(func_name).c_str(),
               string_from_qking_error(ret).c_str());
    }
    has_function = true;
  } else {
    has_function = false;
  }

  qking_release_value(func_by_name);
  return ret;
}

qking_value_t get_optional_property(
    qking_value_t obj, qking_magic_str_t prop_name, const char *process_msg,
    const char *obj_name, std::function<void(qking_value_t)> succ_callback) {
  qking_value_t ret = qking_get_property_by_name_lit(obj, prop_name);
  if (qking_value_is_error(ret)) {
    RAX_LOGE("%s: %s['%s'] err: %s", process_msg, obj_name,
             string_from_qking_string_value_lit(prop_name).c_str(),
             string_from_qking_error(ret).c_str());
    qking_release_value(ret);
    ret = qking_create_undefined();
  } else {
    if (succ_callback) {
      succ_callback(ret);
    }
  }
  return ret;
}

bool set_optional_property(qking_value_t obj, qking_magic_str_t prop_name,
                           qking_value_t value, const char *process_msg,
                           const char *obj_msg) {
  bool succ = true;
  qking_value_t ret = qking_set_property_by_name_lit(obj, prop_name, value);
  if (qking_value_is_error(ret)) {
    RAX_LOGE("%s: %s['%s'] set err: %s", process_msg, obj_msg,
             string_from_qking_string_value_lit(prop_name).c_str(),
             string_from_qking_error(ret).c_str());
    succ = false;
  }
  qking_release_value(ret);
  return succ;
}

bool set_optional_property_if_not_present(qking_value_t obj,
                                          const char *prop_name,
                                          qking_value_t value,
                                          const char *process_msg,
                                          const char *obj_msg) {
  bool succ = true;
  qking_value_t prop_name_value = qking_create_string_c(prop_name);
  qking_value_t old_prop = qking_get_property(obj, prop_name_value);
  if (!qking_value_is_undefined(old_prop)) {
    qking_release_value(prop_name_value);
    qking_release_value(old_prop);
    return true;
  }
  qking_release_value(old_prop);
  qking_value_t ret = qking_set_property(obj, prop_name_value, value);
  if (qking_value_is_error(ret)) {
    RAX_LOGE("%s: %s['%s'] set err: %s", process_msg, obj_msg,
             string_from_qking_string_value(prop_name_value).c_str(),
             string_from_qking_error(ret).c_str());
    succ = false;
  }
  qking_release_value(prop_name_value);
  qking_release_value(ret);
  return succ;
}

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/is
// todo +0 and -0 return true in this impl, while by spec it should returns
// false
static bool polyfill_global_is(qking_value_t x, qking_value_t y) {
  if (qking_value_strict_equal(x, y)) {
    return true;
  }
  return qking_value_is_nan(x) && qking_value_is_nan(y);
}

bool shallow_equal(qking_value_t obj_a, qking_value_t obj_b) {
  if (polyfill_global_is(obj_a, obj_b)) {
    return true;
  }

  if (!qking_value_is_object(obj_a) || !qking_value_is_object(obj_b)) {
    return false;
  }
  struct Passer {
    bool equal;
    uint32_t size;
    qking_value_t obj_b;
  } tmp{true, 0, obj_b};

  qking_foreach_object_property_of(
      obj_a,
      [](const qking_value_t property_name, const qking_value_t property_value,
         void *user_data_p) -> bool {
        auto passer = ((Passer *)user_data_p);
        passer->size++;
        qking_value_t left = property_value;
        qking_value_t has_own =
            qking_has_own_property(passer->obj_b, property_name);
        if (!qking_get_boolean_value(has_own)) {
          qking_release_value(has_own);
          passer->equal = false;
          return false;
        }
        qking_release_value(has_own);
        qking_value_t right = qking_get_property(passer->obj_b, property_name);
        if (qking_value_is_error(right)) {
          qking_release_value(right);
          passer->equal = false;
          return false;
        }

        if (!polyfill_global_is(left, right)) {
          qking_release_value(right);
          passer->equal = false;
          return false;
        }
        qking_release_value(right);
        return true;
      },
      &tmp, false, true, false);

  if (!tmp.equal) {
    return false;
  }

  struct Passer2 {
    uint32_t size;
  } tmp2{0};

  qking_foreach_object_property_of(
      obj_b,
      [](const qking_value_t property_name, const qking_value_t property_value,
         void *user_data_p) -> bool {
        ((Passer2 *)user_data_p)->size++;
        return true;
      },
      &tmp2, false, true, false);

  return tmp.size == tmp2.size;
}

QKValueRef::QKValueRef(qking_value_t value) {
  value_ = qking_acquire_value(value);
}

QKValueRef::~QKValueRef() { qking_release_value(value_); }

rax_js_err::rax_js_err(qking_value_t err, bool release) {
  js_err_.reset(new QKValueRef(err));
  if (release) {
    qking_release_value(err);
  }
}

qking_value_t rax_js_err::acquire_err() const {
  auto ptr = js_err_.get();
  if (ptr) {
    return qking_acquire_value(ptr->get());
  } else {
    return qking_create_undefined();
  }
}

rax_common_err::rax_common_err(const char *err) : rax_exception(err) {}
rax_common_err::rax_common_err(const std::string &err) : rax_exception(err) {}

RAX_NAME_SPACE_END
