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

#ifndef QKING_ROOT_RAX_COMMON_H
#define QKING_ROOT_RAX_COMMON_H

#include <cstdlib>
#include <exception>
#include <string>
#include "api/qking_api.h"
#include "base/qking_common_base.h"
#include "core/jrt/jrt.h"
#ifdef QKING_ENABLE_EXTERNAL_WEEX_ENV
#include "core/data_render/vnode/vnode_render_context.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_creator.h"
#include "core/render/node/render_object.h"
#endif
#ifdef QKING_ENABLE_EXTERNAL_UNIT_TEST_ENV
#include "env/standalone/rax_test_render_object.h"
#include "qking_common_logger.h"
#endif

#define RAX_NAME_SPACE_BEGIN \
  namespace qking {          \
  namespace rax {

#define RAX_NAME_SPACE_END \
  }                        \
  }

#define RAX_NS ::qking::rax

/*
 * Make sure unused parameters, variables, or expressions trigger no compiler
 * warning.
 */
#define RAX_UNUSED(x) ((void)(x))

#define RAX_MAKESURE(expr)                                                 \
  do {                                                                     \
    if (!(expr)) {                                                         \
      LOGE("RAX: Exception '%s' failed at %s(%s):%lu.\n", #expr, __FILE__, \
           __func__, (unsigned long)__LINE__);                             \
      QKING_ASSERT(0);                                                     \
    }                                                                      \
  } while (0)

#define RAX_ASSERT(expr) QKING_ASSERT(expr)

RAX_NAME_SPACE_BEGIN
std::string
rax_get_current_page_name();  // should use extern context to get name
RAX_NAME_SPACE_END

#ifdef QKING_ENABLE_EXTERNAL_WEEX_ENV
using native_node_ptr = WeexCore::RenderObject *;
using namespace json11;
#endif

#ifdef QKING_ENABLE_EXTERNAL_UNIT_TEST_ENV
using native_node_ptr = RaxTestRenderObject *;
using namespace json12;
#endif

#endif  // QKING_ROOT_RAX_COMMON_H
