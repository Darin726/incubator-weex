//
// Created by Xu Jiacheng on 2018/12/25.
//

#include "rax_test_render_object.h"
#include <algorithm>
#include "core/jrt/jrt.h"

#ifdef QKING_ENABLE_EXTERNAL_UNIT_TEST_ENV

void RaxTestRenderObject::UpdateAttr(std::string key, std::string value) {
  attributes_[key] = value;
}

void RaxTestRenderObject::UpdateStyle(std::string key, std::string value) {
  styles_[key] = value;
}

void RaxTestRenderObject::AddEvent(std::string event) { events_.insert(event); }

void RaxTestRenderObject::RemoveEvent(const std::string &event) {
  events_.erase(event);
}

void RaxTestRenderObject::AddChild(RaxTestRenderObject *child) {
  child->set_parent(this);
  children_.push_back(child);
}

void RaxTestRenderObject::AddChildIndex(RaxTestRenderObject *child,
                                        size_t index) {
  QKING_ASSERT(index <= children_.size());
  children_.insert(children_.begin() + index, child);
  child->set_parent(this);
}

void RaxTestRenderObject::RemoveChild(RaxTestRenderObject *child) {
  auto ptr = std::find(children_.begin(), children_.end(), child);
  QKING_ASSERT(ptr != children_.end());
  children_.erase(ptr);
  child->set_parent(nullptr);
}

void RaxTestRenderObject::RemoveChildIndex(size_t index) {
  QKING_ASSERT(index < children_.size());
  children_[index]->set_parent(nullptr);
  children_.erase(children_.begin() + index);
}

Json::object RaxTestRenderObject::DebugPrint() {
  Json::object root;

  root["1_page_id"] = page_id_;
  root["2_ref"] = ref_;
  root["3_type"] = type_;
  if (!styles_.empty()) {
    root["4_style"] = styles_;
  }
  if (!attributes_.empty()) {
    root["5_attr"] = attributes_;
  }

  if (!events_.empty()) {
    root["6_event"] = std::vector<std::string>(events_.begin(), events_.end());
  }

  if (!children_.empty()) {
    Json::array child;
    for (auto &it : children_) {
      child.push_back(it->DebugPrint());
    }
    root["7_child"] = child;
  }
  return root;
}

void RaxTestRenderObject::MountToTree() {
  this->in_tree_ = true;
  for (auto it : children_) {
    it->MountToTree();
  }
}

void RaxTestRenderObject::UnmountFromTree() {
  this->in_tree_ = false;
  for (auto it : children_) {
    it->UnmountFromTree();
  }
}

bool RaxTestRenderObject::CheckValid(bool flag) {
  if (flag != this->in_tree_) {
    return false;
  }
  for (auto it : children_) {
    if (!it->CheckValid(flag)) {
      return false;
    }
  }

  return true;
}

#endif
