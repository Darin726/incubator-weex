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
// Created by Xu Jiacheng on 2019-01-22.
//

#ifndef QKING_ROOT_RAX_REF_H
#define QKING_ROOT_RAX_REF_H

#include "rax_common.h"
#include "rax_core_util.h"
#include "rax_element.h"

RAX_NAME_SPACE_BEGIN

void rax_ref_update(RaxElement *prev, RaxElement *next);

void rax_ref_attach(RaxElement *owner, qking_value_t ref,
                    RaxElement *component);

void rax_ref_detach(RaxElement *owner, qking_value_t ref,
                    RaxElement *component);

qking_value_t rax_create_native_node_obj(const std::string &ref_value);

qking_value_t rax_create_native_node_obj_by_str(qking_value_t str);

RAX_NAME_SPACE_END

#endif  // QKING_ROOT_RAX_REF_H
