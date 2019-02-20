#include "qking_api.h"
#include <iostream>
#include "base/qking_common_error.h"
#include "base/qking_common_logger.h"
#include "base/qking_string_utils.h"
#include "core/vm/vm_exec_state.h"
#include "rax/rax_builtin_env.h"
#include "rax/vdom/rax_element_factory.h"

namespace qking {
namespace api {

std::map<std::string, std::unique_ptr<CallBackManager>>
    CallBackManager::gs_callback_managers_;

CallBackManager::~CallBackManager() {
  try {
    for (auto iter = callback_map_.begin(); iter != callback_map_.end();
         iter++) {
      qking_release_value(iter->second);
    }
  } catch (std::exception &e) {
    LOGE("[exception]:=>qking delete call back error: %s", e.what());
  }
}

std::vector<uint32_t> CallBackManager::GetLastCallbackIds() {
  return last_ids_;
}

void CallBackManager::PushCallbackId(uint32_t id) { last_ids_.push_back(id); }
void CallBackManager::ClearCallbackIds() { last_ids_.clear(); }

uint32_t CallBackManager::AddCallBack(qking_value_t callback) {
  callback_id_++;
  callback_map_[callback_id_] = callback;
  PushCallbackId(callback_id_);
  return callback_id_;
}

CallBackManager *CallBackManager::GetCallBackManager() {
  const auto &page_id = rax::rax_get_current_page_name();
  auto it = CallBackManager::gs_callback_managers_.find(page_id);
  if (it != CallBackManager::gs_callback_managers_.end()) {
    return it->second.get();
  }
  return nullptr;
}

void CallBackManager::RemoveCallBackManager(const std::string &page_id) {
  auto callback = gs_callback_managers_.find(page_id);
  if (callback != gs_callback_managers_.end()) {
    gs_callback_managers_.erase(callback);
  }
}

void CallBackManager::AddCallBackManager(const std::string &page_id) {
  gs_callback_managers_.insert(std::make_pair(
      page_id, std::unique_ptr<CallBackManager>(new CallBackManager)));
}

bool CallBackManager::Call(uint32_t id, const std::string &data,
                           bool keep_alive) {
  bool is_error = false;
  do {
    auto iter = callback_map_.find(id);
    if (iter == callback_map_.end()) {
      break;
    }
    qking_value_t func = iter->second;
    qking_value_t completion_value = qking_create_undefined();
    try {
      if (!qking_value_is_function(func)) {
        LOGE("[data-render] InvokeCallback: not callable");
        break;
      }
      // todo add args;
      if (data.length() > 0) {
        qking_value_t result = qking_json_parse(
            (const qking_char_t *)data.c_str(), (qking_size_t)data.length());
        completion_value =
            qking_call_function(func, qking_create_undefined(), &result, 1);
        qking_release_value(result);
      } else {
        completion_value =
            qking_call_function(func, qking_create_undefined(), nullptr, 0);
      }
      if (!is_error) {
        qking_run_all_enqueued_jobs();
      }
      if (!keep_alive) {
        qking_release_value(func);
        callback_map_.erase(iter);
      }
      qking_release_value(completion_value);

    } catch (std::exception &e) {
      LOGE("[exception]:=>qking call back error: %s", e.what());
      is_error = true;
    }

  } while (0);

  return !is_error;
}

#ifdef QKING_NDEBUG
static void qking_handler_fatal_error(int code) {
  throw qking::FatalError(code);
}
#endif

static void qking_handler_print_debugger_fatal_error(int code) {
  struct qking_context_t *context_p = qking_port_get_current_context();
  qking_vm_exec_state_t *executor_p =
      (qking_vm_exec_state_t *)context_p->executor_p;
  vm_frame_ctx_t *frame_ctx_p = context_p->vm_top_context_p;
  if (!frame_ctx_p) {
    return;
  }
  while (frame_ctx_p) {
    const ecma_compiled_code_t *bytecode_header_p =
        frame_ctx_p->bytecode_header_p;
    size_t size = ((size_t)bytecode_header_p->size) << JMEM_ALIGNMENT_LOG;
    if (size !=
        QKING_ALIGNUP(sizeof(ecma_compiled_function_state_t), JMEM_ALIGNMENT)) {
      frame_ctx_p = frame_ctx_p->prev_context_p;
      continue;
    }
    break;
  }
  std::string error = "\n[exception][pc]:=>";
  if (executor_p->symbols_pp && executor_p->symbols_size > 0 &&
      frame_ctx_p->pc_current_idx < executor_p->symbols_size) {
    error += executor_p->symbols_pp[frame_ctx_p->pc_current_idx];
  } else {
    error += qking::utils::to_string(frame_ctx_p->pc_current_idx);
  }
  error += "\n";
  bool is_first_stack = true;

  frame_ctx_p = context_p->vm_top_context_p;
  while (frame_ctx_p) {
    const ecma_compiled_code_t *bytecode_header_p =
        frame_ctx_p->bytecode_header_p;
    size_t size = ((size_t)bytecode_header_p->size) << JMEM_ALIGNMENT_LOG;
    if (size !=
        QKING_ALIGNUP(sizeof(ecma_compiled_function_state_t), JMEM_ALIGNMENT)) {
      frame_ctx_p = frame_ctx_p->prev_context_p;
      continue;
    }
    const ecma_compiled_function_state_t *func_state_p =
        (ecma_compiled_function_state_t *)bytecode_header_p;
    if (executor_p->symbols_pp && executor_p->symbols_size > 0 &&
        func_state_p->func_symbol_idx < executor_p->symbols_size) {
      if (is_first_stack) {
        error += "[stack]:=>";
        is_first_stack = false;
      }
      error += executor_p->symbols_pp[func_state_p->func_symbol_idx];
      error += "\n";
    } else if (func_state_p->func_symbol_idx >= 0) {
      if (is_first_stack) {
        error += "[stack]:=>";
        is_first_stack = false;
      }
      error += qking::utils::to_string(func_state_p->func_symbol_idx);
      error += "\n";
    }
    frame_ctx_p = frame_ctx_p->prev_context_p;
  }
  LOGD("%s", error.c_str());
  if (executor_p->exception_p) {
    free(executor_p->exception_p);
    executor_p->exception_p = NULL;
  }
  executor_p->exception_p = (char *)malloc(error.length() + 1);
  if (executor_p->exception_p) {
    memcpy(executor_p->exception_p, error.c_str(), error.length() + 1);
  }
}

std::string string_from_qking_string_value(const qking_value_t string_var) {
  std::string str;
  if (qking_value_is_string(string_var)) {
    uint32_t string_size = qking_get_string_size(string_var);
    qking_char_t *buffer = new qking_char_t[string_size];
    if (buffer) {
      qking_size_t result_size =
          qking_string_to_char_buffer(string_var, buffer, string_size);
      std::string str((char *)buffer, result_size);
      delete[] buffer;
      return str;
    }
  }
  return str;
}

std::string string_from_qking_get_property_by_name(const qking_value_t obj_val,
                                                   const char *name_p) {
  qking_value_t result = qking_get_property_by_name(obj_val, name_p);
  std::string str = string_from_qking_string_value(result);
  qking_release_value(result);
  return str;
}

std::string string_from_qking_json_stringify(
    const qking_value_t object_to_stringify) {
  if (qking_value_is_undefined(object_to_stringify)) {
    return "undefined";
  }
  qking_value_t string_var = qking_json_stringify(object_to_stringify);
  if (qking_value_is_error(string_var)) {
    const std::string &err_log = string_from_qking_error(string_var);
    LOGE("[qking] string_from_qking_json_stringify err: %s", err_log.c_str());
    qking_release_value(string_var);
    return "";
  }
  std::string str = string_from_qking_string_value(string_var);
  qking_release_value(string_var);
  return str;
}

std::string string_from_qking_error(const qking_value_t err) {
  if (!qking_value_is_error(err)) {
    return "not an err";
  }
  qking_value_t err_value = qking_get_value_from_error(err, false);
  qking_value_t str_val = qking_value_to_string(err_value);
  qking_release_value(err_value);

  if (qking_value_is_error(str_val)) {
    qking_release_value(str_val);
    return "err can' convert to string";
  }
  std::string err_msg_str = string_from_qking_string_value(str_val);
  qking_api_get_last_exception(err_msg_str);
  qking_release_value(str_val);
  return err_msg_str;
}

std::string string_from_qking_to_string(const qking_value_t object_to_string) {
  qking_value_t string_var = qking_value_to_string(object_to_string);
  if (qking_value_is_error(string_var)) {
    const std::string &err_log = string_from_qking_error(string_var);
    LOGE("[qking] string_from_qking_to_string err: %s", err_log.c_str());
    qking_release_value(string_var);
    return "";
  }
  std::string str = string_from_qking_string_value(string_var);
  qking_release_value(string_var);
  return str;
}

std::string string_from_qking_get_property_by_index(const qking_value_t obj_val,
                                                    uint32_t index) {
  qking_value_t string_var = qking_get_property_by_index(obj_val, index);
  std::string str = string_from_qking_string_value(string_var);
  qking_release_value(string_var);
  return str;
}

bool qking_api_get_last_exception(std::string &exception) {
  bool success = false;
  do {
    struct qking_context_t *context_p = qking_port_get_current_context();
    qking_vm_exec_state_t *executor_p =
        (qking_vm_exec_state_t *)context_p->executor_p;
    if (!executor_p->exception_p) {
      break;
    }
    exception += executor_p->exception_p;
    success = true;

  } while (0);

  return success;
}

bool qking_api_execute_code(qking_executor_t executor, std::string &error) {
  qking_value_t error_var = qking_create_undefined();
  bool success = false;
  try {
    success = qking_execute_code(executor, &error_var);
  } catch (std::exception &e) {
    error = e.what();
    std::cerr << e.what() << std::endl;
    LOGE("[exception]:=>qking execute code error: %s", e.what());
    return success;
  }
  if (!success) {
    error = string_from_qking_error(error_var);
  }
  qking_release_value(error_var);
  return success;
}

bool qking_api_set_assembly_code(qking_executor_t executor, uint8_t *code,
                                 size_t size, std::string &error) {
  if (!code) {
    error = "qking set assembly code error: null *code";
    return false;
  }

  qking_value_t error_var = qking_create_undefined();
  bool success = false;
  try {
    success = qking_set_assembly_code(executor, code, size, &error_var);
  } catch (std::exception &e) {
    error = e.what();
    std::cerr << e.what() << std::endl;
    LOGE("[exception]:=>qking set assembly code error: %s", e.what());
    return success;
  }
  if (!success) {
    error = string_from_qking_to_string(error_var);
    qking_release_value(error_var);
  }
  return success;
}

#ifndef QKING_NDEBUG

std::string qking_value_print(const qking_value_t value) {
  qking_value_t string = qking_value_debug_print(value);
  std::string str = string_from_qking_string_value(string);
  qking_release_value(string);
  return str;
}

#endif

bool qking_value_add_entries_from_object(const qking_value_t dest,
                                         const qking_value_t src) {
  if (!qking_value_is_object(src) || !qking_value_is_object(dest)) {
    return false;
  }

  qking_foreach_object_property_of(
      src,
      [](const qking_value_t property_name, const qking_value_t property_value,
         void *user_data_p) {
        qking_value_t dest_in = *((qking_value_t *)user_data_p);
        qking_set_property(dest_in, property_name, property_value);
        return true;
      },
      (void *)&dest, false, true, false);
  return true;
}

std::string string_from_qking_string_value_lit(const qking_magic_str_t name) {
  std::string str;
  qking_value_t string_var = qking_create_string_lit(name);
  if (qking_value_is_string(string_var)) {
    uint32_t string_size = qking_get_string_size(string_var);
    qking_char_t *buffer = new qking_char_t[string_size];
    if (buffer) {
      qking_size_t result_size =
          qking_string_to_char_buffer(string_var, buffer, string_size);
      std::string str((char *)buffer, result_size);
      delete[] buffer;
      qking_release_value(string_var);
      return str;
    }
  }
  qking_release_value(string_var);
  return str;
}

static qking_value_t qking_api_get_last_callback_ids(
    const qking_value_t function_obj, const qking_value_t this_val,
    const qking_value_t args_p[], const qking_length_t args_count) {
  CallBackManager *callback_manager = CallBackManager::GetCallBackManager();
  if (callback_manager == nullptr) {
    return qking_create_undefined();
  }
  const std::vector<uint32_t> &id = callback_manager->GetLastCallbackIds();
  qking_value_t ret = qking_create_array(static_cast<uint32_t>(id.size()));
  for (uint32_t i = 0; i < id.size(); ++i) {
    qking_value_t number = qking_create_number(id[i]);
    qking_release_value(qking_set_property_by_index(ret, i, number));
    qking_release_value(number);
  }
  return ret;
}

static qking_value_t qking_api_call_native_module(
    const qking_value_t function_obj, const qking_value_t this_val,
    const qking_value_t args_p[], const qking_length_t args_count) {
  qking_value_t result = qking_create_undefined();
  do {
    if (args_count < 1) {
      break;
    }
    if (!qking_value_is_object(args_p[0])) {
      break;
    }
    std::string module =
        string_from_qking_get_property_by_name(args_p[0], "module");
    if (module.empty()) {
      break;
    }
    std::string method =
        string_from_qking_get_property_by_name(args_p[0], "method");
    if (method.empty()) {
      break;
    }
    qking_value_t args_var = qking_get_property_by_name(args_p[0], "args");
    if (!qking_value_is_object(args_var)) {
      qking_release_value(args_var);
      break;
    }
    qking_value_t length_var = qking_get_property_by_name(args_var, "length");
    if (!qking_value_is_number(length_var)) {
      qking_release_value(length_var);
      break;
    }
    int argc = qking_get_number_value(length_var);
    qking_release_value(length_var);
    std::string args;
    CallBackManager *callback_manager = CallBackManager::GetCallBackManager();
    if (callback_manager == nullptr) {
      break;
    }
    callback_manager->ClearCallbackIds();
    if (argc > 0) {
      qking_value_t args_array = qking_create_array(argc);
      for (int i = 0; i < argc; i++) {
        qking_value_t var = qking_get_property_by_index(args_var, i);
        bool is_function = qking_value_is_function(var);
        if (is_function) {
          uint32_t func_callback = callback_manager->AddCallBack(var);
          var = qking_create_string_from_utf8(
              (const qking_char_t *)(qking::utils::to_string(func_callback)
                                         .c_str()));
        }
        qking_set_property_by_index(args_array, i, var);
        qking_release_value(var);
      }
      args = string_from_qking_json_stringify(args_array);
      qking_release_value(args_array);
    }
    qking_release_value(args_var);
    result = qking_api_port_call_native_module(
        qking_get_current_executor(), module.c_str(), method.c_str(),
        argc > 0 ? args.c_str() : "", argc);

  } while (0);

  return result;  // only the first function will return now.
}

static qking_value_t qking_api_require_module(const qking_value_t function_obj,
                                              const qking_value_t this_val,
                                              const qking_value_t args_p[],
                                              const qking_length_t args_count) {
  qking_value_t result = qking_create_undefined();
  do {
    if (!args_count) {
      break;
    }
    if (!qking_value_is_string(args_p[0])) {
      break;
    }
    std::string module_name = string_from_qking_string_value(args_p[0]);
    std::string module_info;
    if (!qking_api_port_require_module(module_name, module_info)) {
      break;
    }
    result = qking_json_parse((const qking_char_t *)module_info.c_str(),
                              (qking_size_t)module_info.length());

  } while (0);

  return result;
}

static qking_value_t qking_api_print(const qking_value_t function_obj,
                                     const qking_value_t this_val,
                                     const qking_value_t args_p[],
                                     const qking_length_t args_count) {
  std::string log_result;
  for (int i = 0; i < args_count; ++i) {
    log_result.append(string_from_qking_to_string(args_p[i]));
    log_result.append(" ");
  }
  return qking_print_log("QK_JSLog: ", function_obj, this_val, args_p,
                         args_count);
}

static qking_value_t qking_api_do_nothing(const qking_value_t function_obj,
                                          const qking_value_t this_val,
                                          const qking_value_t args_p[],
                                          const qking_length_t args_count) {
  return qking_create_undefined();
}

#ifdef QKING_ENABLE_EXTERNAL_WEEX_ENV

void qking_api_register_weex_environment(void) {
  qking_register_handler_debugger_fatal_error(
      qking_handler_print_debugger_fatal_error);
#ifdef QKING_NDEBUG
  qking_register_handler_fatal_error(qking_handler_fatal_error);
#else
#endif
  const auto &page_id = rax::rax_get_current_page_name();
  RAX_NS::RaxElementFactory::CreateFactory(page_id);
  CallBackManager::AddCallBackManager(page_id);
  rax::qking_rax_register_builtin_env();
  qking_external_handler_register_global("__callNativeModule",
                                         qking_api_call_native_module);
  qking_external_handler_register_global("__getLastCallbackIds",
                                         qking_api_get_last_callback_ids);
  qking_external_handler_register_global("__requireModule",
                                         qking_api_require_module);
  qking_external_handler_register_global("__print", qking_api_print);
  qking_external_handler_register_global("__isRegisteredModule",
                                         qking_api_do_nothing);  // todo
  qking_external_handler_register_global("__isRegisteredComponent",
                                         qking_api_do_nothing);
}

#endif

#ifdef QKING_ENABLE_EXTERNAL_UNIT_TEST_ENV
void qking_api_register_test_environment(void) {
  qking_register_handler_debugger_fatal_error(
      qking_handler_print_debugger_fatal_error);

  qking_external_handler_register_global("__print", qking_api_print);
}
#endif

void qking_api_register_variable(const std::string &init_data_str) {
  try {
    do {
      qking_value_t result =
          qking_json_parse((const qking_char_t *)init_data_str.c_str(),
                           (qking_length_t)init_data_str.length());
      if (qking_value_is_error(result)) {
        qking_release_value(result);
        break;
      }
      qking_external_variable_register_global("_init_data", result);
      qking_external_variable_register_global("__weex_data__", result);
      qking_release_value(result);

    } while (0);

  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    LOGE("[exception]:=>qking api register variable error: %s", e.what());
  }
}

void qking_api_register_variable(const char *name_p,
                                 const std::string &jsonstr) {
  try {
    do {
      qking_value_t result =
          qking_json_parse((const qking_char_t *)jsonstr.c_str(),
                           (qking_length_t)jsonstr.length());
      if (qking_value_is_error(result)) {
        break;
      }
      qking_external_variable_register_global(name_p, result);
      qking_release_value(result);

    } while (0);
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    LOGE("[exception]:=>qking api register variable error: %s", e.what());
  }
}

}  // namespace api
}  // namespace qking
