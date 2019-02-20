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
#ifndef QKING_API_H
#define QKING_API_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "api/qking.h"

namespace qking {
namespace api {

class CallBackManager {
 public:
  ~CallBackManager();
  uint32_t AddCallBack(qking_value_t callback);
  std::vector<uint32_t> GetLastCallbackIds();
  void PushCallbackId(uint32_t id);
  void ClearCallbackIds();
  bool Call(uint32_t id, const std::string &data, bool keep_alive);
  static CallBackManager *GetCallBackManager();
  static void AddCallBackManager(const std::string &page_id);
  static void RemoveCallBackManager(const std::string &page_id);

 private:
  uint32_t callback_id_ = 0;
  std::vector<uint32_t> last_ids_;
  std::map<uint32_t, qking_value_t> callback_map_;
  static std::map<std::string, std::unique_ptr<CallBackManager>>
      gs_callback_managers_;
};

std::string string_from_qking_string_value(const qking_value_t var);

std::string string_from_qking_get_property_by_name(const qking_value_t obj_val,
                                                   const char *name_p);

std::string string_from_qking_json_stringify(
    const qking_value_t object_to_stringify);

std::string string_from_qking_error(const qking_value_t err);

std::string string_from_qking_to_string(const qking_value_t object_to_string);

std::string string_from_qking_get_property_by_index(const qking_value_t obj_val,
                                                    uint32_t index);

std::string string_from_qking_string_value_lit(const qking_magic_str_t name);

bool qking_value_add_entries_from_object(const qking_value_t dest,
                                         const qking_value_t src);

#ifndef QKING_NDEBUG
std::string qking_value_print(const qking_value_t value);
#endif

#ifdef QKING_ENABLE_EXTERNAL_WEEX_ENV
void qking_api_register_weex_environment(void);
#endif

qking_value_t qking_api_port_call_native_module(qking_executor_t executor,
                                                const std::string &module,
                                                const std::string &method,
                                                const std::string &args,
                                                int argc = 0);

bool qking_api_port_require_module(std::string &name, std::string &info);

void qking_api_port_print(const char *pcstr);

void qking_api_register_variable(const std::string &init_data_str);

void qking_api_register_variable(const char *name_p,
                                 const std::string &jsonstr);

bool qking_api_execute_code(qking_executor_t executor, std::string &error);

bool qking_api_set_assembly_code(qking_executor_t executor, uint8_t *code,
                                 size_t size, std::string &error);

}  // namespace api
}  // namespace qking

#endif /* !QKING_API_H */
