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
// Created by Xu Jiacheng on 2019-01-18.
//

#ifndef QKING_ROOT_RAX_NATIVE_TRANSFORM_H
#define QKING_ROOT_RAX_NATIVE_TRANSFORM_H
#include "rax_common.h"
#include "rax_core_util.h"
#include "rax_native_element.h"

RAX_NAME_SPACE_BEGIN

bool rax_native_transform(RaxNativeElement* element);

/**
 * @return true if you should skip update. false if no trans happened or update
 * should continue.
 */
bool rax_native_update_transform(RaxNativeElement* element);

void rax_create_element_filter_props(RaxElement* element);

RAX_NAME_SPACE_END

#endif  // QKING_ROOT_RAX_NATIVE_TRANSFORM_H
