/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
//
// Created by Xu Jiacheng on 2018/12/25.
//

#ifndef QKING_ROOT_RAX_CORE_UTIL_H
#define QKING_ROOT_RAX_CORE_UTIL_H

#include <algorithm>
#include <functional>
#include <sstream>
#include "api/qking_api.h"
#include "base/qking_string_utils.h"
#include "rax_common.h"

using namespace qking::api;
using namespace qking::utils;

#define THROW_JS_ERR_IF(expr, msg)                              \
  do {                                                          \
    if (!!(expr)) {                                             \
      qking_release_value(func_ret);                            \
      func_ret = qking_create_error(QKING_ERROR_COMMON, (msg)); \
      throw rax_builtin_js_err();                               \
    }                                                           \
  } while (0)

#define CHECK_TMP_VAR(expr)                  \
  do {                                       \
    qking_value_t _expr_value = (expr);      \
    if (qking_value_is_error(_expr_value)) { \
      qking_release_value(func_ret);         \
      func_ret = _expr_value;                \
      throw rax_builtin_js_err();            \
    }                                        \
  } while (0)

#define CHECK_TMP_VAR_RELEASE(expr, release) \
  do {                                       \
    qking_value_t _expr_value = (expr);      \
    if (qking_value_is_error(_expr_value)) { \
      qking_release_value(func_ret);         \
      qking_release_value((release));        \
      func_ret = _expr_value;                \
      throw rax_builtin_js_err();            \
    }                                        \
  } while (0)

#define CHECK_RESOURCE(expr)                       \
  do {                                             \
    qking_value_t _expr_value = (expr);            \
    if (qking_value_is_error(_expr_value)) {       \
      qking_release_value(func_ret);               \
      func_ret = qking_acquire_value(_expr_value); \
      throw rax_builtin_js_err();                  \
    }                                              \
  } while (0)

#define COMMON_RETURN(expr)        \
  do {                             \
    qking_release_value(func_ret); \
    func_ret = (expr);             \
    throw rax_builtin_js_err();    \
  } while (0)

#define COMMON_START() qking_value_t func_ret = qking_create_undefined();

#define COMMON_RESOURCE() \
  try {                   \
  ((void)0)

#define COMMON_FINALIZE()                                        \
  }                                                              \
  catch (const rax_builtin_js_err& e) {                          \
  }                                                              \
  catch (const rax_common_err& e) {                              \
    qking_release_value(func_ret);                               \
    func_ret = qking_create_error(QKING_ERROR_COMMON, e.what()); \
  }                                                              \
  catch (const rax_js_err& e) {                                  \
    qking_release_value(func_ret);                               \
    func_ret = e.acquire_err();                                  \
  }                                                              \
  ((void)0)

#define COMMON_END() return func_ret

#ifdef QKING_RAX_DEBUG
#define RAX_LOGD(...) LOGI("[RAX] " __VA_ARGS__)
#else
#define RAX_LOGD(...) ((void)0)
#endif

#define RAX_LOGI(...) LOGI("[RAX] " __VA_ARGS__)
#define RAX_LOGW(...) LOGW("[RAX] " __VA_ARGS__)
#define RAX_LOGE(...) LOGE("[RAX] " __VA_ARGS__)

RAX_NAME_SPACE_BEGIN
extern const std::string ROOT_REF;

class QKValueRef {
 public:
  QKValueRef(qking_value_t value);
  virtual ~QKValueRef();
  inline qking_value_t get() { return value_; };

 private:
  qking_value_t value_;
  DISALLOW_COPY_AND_ASSIGN(QKValueRef);
};

class rax_exception : public std::logic_error {
 public:
  explicit rax_exception(const std::string& what) : logic_error(what){};
  explicit rax_exception(const char* what) : logic_error(what){};
  explicit rax_exception() : logic_error(""){};
};

class rax_builtin_js_err : public rax_exception {};

class rax_js_err : public rax_exception {
 public:
  rax_js_err(qking_value_t err, bool release = false);
  qking_value_t acquire_err() const;

 private:
  std::unique_ptr<QKValueRef> js_err_;
};

class rax_common_err : public rax_exception {
 public:
  rax_common_err(const char* err);
  rax_common_err(const std::string& err);
};

qking_value_t rax_flatten_children(const qking_value_t* array,
                                   qking_length_t size);

qking_value_t rax_flatten_style(qking_value_t value);

qking_value_t make_optional_call_on(qking_value_t class_inst,
                                    qking_magic_str_t func_name,
                                    const char* process_msg,
                                    const qking_value_t* args_p = nullptr,
                                    qking_size_t args_count = 0);

qking_value_t make_optional_call_on_ret(qking_value_t class_inst,
                                        qking_magic_str_t func_name,
                                        const char* process_msg,
                                        bool& has_function,
                                        const qking_value_t* args_p = nullptr,
                                        qking_size_t args_count = 0);

qking_value_t get_optional_property(
    qking_value_t obj, qking_magic_str_t prop_name, const char* process_msg,
    const char* obj_name,
    std::function<void(qking_value_t)> succ_callback = nullptr);

bool set_optional_property(qking_value_t obj, qking_magic_str_t prop_name,
                           qking_value_t value, const char* process_msg,
                           const char* obj_msg);

bool set_optional_property_if_not_present(qking_value_t obj, const char*,
                                          qking_value_t value,
                                          const char* process_msg,
                                          const char* obj_msg);

bool shallow_equal(qking_value_t obj_a, qking_value_t obj_b);

RAX_NAME_SPACE_END

#endif  // QKING_ROOT_RAX_CORE_UTIL_H
