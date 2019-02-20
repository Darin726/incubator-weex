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
// Created by Xu Jiacheng on 2019-01-30.
//

#ifndef QKING_ROOT_RAX_PROPS_HOLDER_H
#define QKING_ROOT_RAX_PROPS_HOLDER_H

#include "rax_core_util.h"

RAX_NAME_SPACE_BEGIN

class RaxPropsHolder {
 public:
  inline void set_style(const std::string &key, const std::string &value) {
    comp_styles_[key] = value;
  }

  inline void set_attr(const std::string &key, const std::string &value) {
    comp_attrs_[key] = value;
  }

  inline void set_event(const std::string &key, qking_value_t value) {
    comp_events_[key] = std::unique_ptr<QKValueRef>(new QKValueRef(value));
  }

  inline void remove_event(const std::string &key) { comp_events_.erase(key); }

  inline const std::map<std::string, std::string> &get_styles() {
    return comp_styles_;
  }

  inline const std::map<std::string, std::string> &get_attrs() {
    return comp_attrs_;
  }

  inline std::map<std::string, std::unique_ptr<QKValueRef>> &get_events() {
    return comp_events_;
  }

  void SetNativeProps(
      qking_value_t props,
      std::function<void(const std::string &key, const std::string &value)>
          style_hook,
      std::function<void(const std::string &key, const std::string &value)>
          attr_hook,
      std::function<void(const std::string &event, qking_value_t value)>
          event_hook);

 private:
  std::map<std::string, std::string> comp_styles_;
  std::map<std::string, std::string> comp_attrs_;
  std::map<std::string, std::unique_ptr<QKValueRef>> comp_events_;
};

RAX_NAME_SPACE_END

#endif  // QKING_ROOT_RAX_PROPS_HOLDER_H
