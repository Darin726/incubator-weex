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
// Created by Xu Jiacheng on 2019-01-15.
//

#ifndef QKING_ROOT_RAX_ERR_QUEUE_HOLDER_H
#define QKING_ROOT_RAX_ERR_QUEUE_HOLDER_H

#include <queue>
#include "rax_core_util.h"
#include "rax_element.h"

RAX_NAME_SPACE_BEGIN

class RaxErrQueueHolder {
 public:
  inline bool err_queue_empty() { return err_queue_.empty(); }

  inline void err_queue_push(qking_value_t err_value) {
    err_queue_.emplace(new QKValueRef(err_value));
  };

  inline void err_queue_move(std::queue<std::unique_ptr<QKValueRef>>& dest) {
    dest = std::move(err_queue_);
  };

  qking_value_t err_queue_pop();

  inline bool callback_queue_empty() { return callback_queue_.empty(); }

  inline void callback_queue_push(qking_value_t callback) {
    callback_queue_.emplace(new QKValueRef(callback));
  };

  inline void callback_queue_move(
      std::queue<std::unique_ptr<QKValueRef>>& dest) {
    dest = std::move(callback_queue_);
  };

 private:
  std::queue<std::unique_ptr<QKValueRef>> err_queue_;
  std::queue<std::unique_ptr<QKValueRef>> callback_queue_;
};
RAX_NAME_SPACE_END

#endif  // QKING_ROOT_RAX_ERR_QUEUE_HOLDER_H
