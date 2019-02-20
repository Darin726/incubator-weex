//
// Created by Xu Jiacheng on 2019-01-15.
//

#include "rax_error_handle.h"
#include "rax_class_element.h"

RAX_NAME_SPACE_BEGIN

/**
 * will throw rax_js_err exception if no componentDidCatch found in tree.
 */
void rax_handle_error(RaxElement *element, qking_value_t err,
                      bool release_on_throw) {
  RAX_ASSERT(element);
  RAX_ASSERT(qking_value_is_error(err));

  RaxClassElement *boundary = nullptr;
  while (element) {
    if (element->type() == RaxElementType::kClass) {
      RaxClassElement *class_element = dynamic_cast<RaxClassElement *>(element);
      qking_value_t class_inst = class_element->get_class_instance();
      if (qking_value_is_object(class_inst)) {
        // has mounted
        qking_value_t did_catch_js_func = qking_get_property_by_name_lit(
            class_inst, QKING_LIT_MAGIC_STRING_EX_COMPONENT_DID_CATCH);
        if (qking_value_is_function(did_catch_js_func)) {
          // has function;
          qking_release_value(did_catch_js_func);
          boundary = class_element;
          break;
        } else {
          // no function;
          qking_release_value(did_catch_js_func);
          element = element->get_comp_parent();
        }
      } else {
        // not mounted.
        RAX_ASSERT(!boundary);
        break;
      }
    } else {
      element = element->get_comp_parent();
    }
  }

  if (boundary) {
    boundary->err_queue_push(err);
  } else {
    throw rax_js_err(err, release_on_throw);
  }
}

/**
 * @param element
 * @param func_to_call not suppose to throw any c++ exception
 */
void perform_safe_to_queue(RaxElement *element,
                           std::function<qking_value_t()> func_to_call) {
  qking_value_t ret = func_to_call();
  if (qking_value_is_error(ret)) {
    rax_handle_error(element, ret, true);
  }
  qking_release_value(ret);
}

/**
 * @param element
 * @param func_to_call not suppose to throw any c++ exception
 */
qking_value_t perform_safe_to_value(
    RaxElement *element, qking_value_t &err,
    std::function<qking_value_t()> func_to_call) {
  qking_value_t ret = func_to_call();
  if (qking_value_is_error(ret)) {
    qking_release_value(err);
    err = ret;
    return qking_create_undefined();
  } else {
    return ret;
  }
}

void flush_err_queue(RaxClassElement *element) {
  qking_value_t class_instance = element->get_class_instance();
  RAX_ASSERT(qking_value_is_object(class_instance));

  std::queue<std::unique_ptr<QKValueRef>> queue_to_flush;
  element->err_queue_move(queue_to_flush);
  RAX_ASSERT(element->err_queue_empty());

  while (!queue_to_flush.empty()) {
    qking_value_t err[1] = {
        qking_get_value_from_error(queue_to_flush.front()->get(), false)};
    qking_release_value(make_optional_call_on(
        class_instance, QKING_LIT_MAGIC_STRING_EX_COMPONENT_DID_CATCH,
        "RaxClassElement flush_err_queue", err, 1));
    queue_to_flush.pop();
    qking_release_value(err[0]);
  }

  RAX_ASSERT(queue_to_flush.empty());
}

RAX_NAME_SPACE_END
