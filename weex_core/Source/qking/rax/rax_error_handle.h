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

#ifndef QKING_ROOT_RAX_ERROR_HANDLE_H
#define QKING_ROOT_RAX_ERROR_HANDLE_H

#include "rax_class_element.h"
#include "rax_common.h"
#include "rax_core_util.h"
#include "rax_element.h"
#include "rax_err_queue_holder.h"

RAX_NAME_SPACE_BEGIN

void rax_handle_error(RaxElement* element, qking_value_t err,
                      bool release_on_throw);

void perform_safe_to_queue(RaxElement* element,
                           std::function<qking_value_t()> func_to_call);

qking_value_t perform_safe_to_value(
    RaxElement* element, qking_value_t& err,
    std::function<qking_value_t()> func_to_call);

void flush_err_queue(RaxClassElement* element);

RAX_NAME_SPACE_END

#endif  // QKING_ROOT_RAX_ERROR_HANDLE_H
