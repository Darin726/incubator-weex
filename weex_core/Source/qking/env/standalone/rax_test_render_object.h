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

#ifndef QKING_ROOT_RAXTESTRENDEROBJECT_H
#define QKING_ROOT_RAXTESTRENDEROBJECT_H

#ifdef QKING_ENABLE_EXTERNAL_UNIT_TEST_ENV

#include <map>
#include <set>
#include <string>
#include <vector>
#include "third_party/json12/json12.hpp"

using namespace json12;

class RaxTestRenderObject {
 public:
  virtual ~RaxTestRenderObject() {}
  inline void set_ref(const std::string &ref) { this->ref_ = ref; }

  inline const std::string &ref() const { return ref_; }

  inline void set_page_id(const std::string &page_id) {
    this->page_id_ = page_id;
  }

  inline const std::string &page_id() const { return page_id_; }

  inline void set_type(const std::string &type) { this->type_ = type; }

  inline const std::string &type() const { return type_; }

  inline RaxTestRenderObject *parent() { return parent_; }

  inline void set_parent(RaxTestRenderObject *parent) { parent_ = parent; }

  inline void CopyFrom(RaxTestRenderObject *src) {
    set_ref(src->ref());
    set_page_id(src->page_id());
    set_type(src->type());
  }

  void UpdateAttr(std::string key, std::string value);

  void UpdateStyle(std::string key, std::string value);

  void AddEvent(std::string event);

  void RemoveEvent(const std::string &event);

  void AddChild(RaxTestRenderObject *child);

  void AddChildIndex(RaxTestRenderObject *child, size_t index);

  void RemoveChild(RaxTestRenderObject *child);

  void RemoveChildIndex(size_t index);

  inline const std::vector<RaxTestRenderObject *> &get_children() const {
    return children_;
  }

  Json::object DebugPrint();

  void MountToTree();

  void UnmountFromTree();

  bool CheckValid(bool flag);

  inline bool in_tree() { return in_tree_; }

  inline bool not_in_tree() { return !in_tree_; }

 private:
  std::string page_id_ = "";
  std::string ref_ = "";
  std::string type_ = "";

  std::map<std::string, std::string> styles_;
  std::map<std::string, std::string> attributes_;
  std::set<std::string> events_;

  std::vector<RaxTestRenderObject *> children_;
  RaxTestRenderObject *parent_ = nullptr;

  bool in_tree_ = false;
};

#endif  // QKING_ROOT_RAXTESTRENDEROBJECT_H

#endif  // QKING_ENABLE_EXTERNAL_UNIT_TEST_ENV
