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
// Created by Xu Jiacheng on 2018/12/26.
//

#ifndef QKING_ROOT_RAX_CLASS_ELEMENT_H
#define QKING_ROOT_RAX_CLASS_ELEMENT_H

#include "rax_common.h"
#include "rax_core_util.h"
#include "rax_element.h"
#include "rax_err_queue_holder.h"
#include "rax_render_comp_holder.h"
#include "rax_state_holder.h"

RAX_NAME_SPACE_BEGIN

class RaxClassElement : public RaxRenderCompHolder,
                        public RaxStateHolder,
                        public RaxErrQueueHolder {
 public:
  RaxClassElement(uint32_t eid);

  void DebugPrintComponent() override;

  inline qking_value_t get_class_instance() {
    return js_component_instance_ ? js_component_instance_->get()
                                  : qking_create_undefined();
  }

  inline void set_class_instance(qking_value_t instance) {
    js_component_instance_.reset(new QKValueRef(instance));
  }

  inline bool &comp_pending_state() { return comp_pending_state_; }
  native_node_ptr get_first_render_node() override;
  native_node_ptr get_last_render_node() override;
  Json::object DebugCollectComponent() override;

 protected:
  void MountComponentInternal(native_node_ptr render_parent,
                              RaxElement *component_parent,
                              const ChildMounter &mounter) override;
  void UnmountComponentInternal(bool not_remove_child) override;
  void UpdateComponentInternal(RaxElement *prev_ele, RaxElement *next_ele,
                               uint32_t insert_start) override;

  qking_value_t ProcessPendingState(qking_value_t public_props);

 private:
  void UpdateRenderedComponent(uint32_t insert_start);
  RaxElement *CallRenderOnInstance(qking_value_t *err);

  bool comp_pending_state_ = false;
  std::unique_ptr<QKValueRef> js_component_instance_;
};
RAX_NAME_SPACE_END

#endif  // QKING_ROOT_RAX_CLASS_ELEMENT_H
