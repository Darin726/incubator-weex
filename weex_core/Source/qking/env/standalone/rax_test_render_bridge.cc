
#include <algorithm>
#include "rax_native_element.h"
#include "rax_render_bridge.h"
#include "rax_test_env.h"

#ifdef QKING_ENABLE_EXTERNAL_UNIT_TEST_ENV

RAX_NAME_SPACE_BEGIN
namespace render_bridge {

native_node_ptr NativeNodeCreate(RaxNativeElement *rax_element) {
  RaxTestRenderObject *ele = new RaxTestRenderObject();
  const auto &styles = rax_element->get_styles();
  const auto &attrs = rax_element->get_attrs();
  const auto &events = rax_element->get_events();

  ele->set_page_id(rax_get_current_page_name());
  ele->set_ref(rax_element->eid_str());
  ele->set_type(rax_element->actrual_comp_type());

  for (const auto &it : styles) {
    ele->UpdateStyle(it.first, it.second);
  }

  for (const auto &it : attrs) {
    ele->UpdateAttr(it.first, it.second);
  }

  for (const auto &it : events) {
    ele->AddEvent(it.first);
  }
  RAX_ASSERT(RaxTestEnv::CheckRootTreeValid());
  return ele;
}

native_node_ptr NativeTextNodeCreate(RaxTextElement *rax_element) {
  RaxTestRenderObject *ele = new RaxTestRenderObject();

  ele->set_page_id(rax_get_current_page_name());
  ele->set_ref(rax_element->eid_str());
  ele->set_type("text");

  ele->UpdateAttr("value", rax_element->text());
  RAX_ASSERT(RaxTestEnv::CheckRootTreeValid());
  return ele;
}

void NativeNodeAddChildren(native_node_ptr parent, native_node_ptr child) {
  RAX_ASSERT(parent);
  RAX_ASSERT(child);
  RAX_ASSERT(parent->not_in_tree());
  RAX_ASSERT(child->not_in_tree());
  parent->AddChild(child);
  RAX_ASSERT(RaxTestEnv::CheckRootTreeValid());
}

int32_t NativeNodeIndexOf(native_node_ptr parent, native_node_ptr child) {
  const auto &children = parent->get_children();
  const auto &it = std::find(children.begin(), children.end(), child);

  if (it == children.end()) {
    return -1;
  } else {
    return static_cast<int32_t>(std::distance(children.begin(), it));
  }
}

native_node_ptr NativeNodeChildOfIndex(native_node_ptr parent, uint32_t index) {
  RAX_ASSERT(parent);
  RAX_ASSERT(index < parent->get_children().size());
  return parent->get_children()[index];
}

native_node_ptr NativeNodeCreateRootNode() {
  auto root = new RaxTestRenderObject();
  root->set_type("div");
  root->set_page_id(rax_get_current_page_name());
  root->set_ref(ROOT_REF);
  RAX_ASSERT(RaxTestEnv::CheckRootTreeValid());
  return root;
}

void SetRootNode(native_node_ptr node) {
  RAX_ASSERT(node);
  node->CheckValid(false);
  RaxTestEnv::g_root_ = node;
  node->MountToTree();
  RAX_ASSERT(RaxTestEnv::CheckRootTreeValid());
}

void InsertNode(native_node_ptr parent, native_node_ptr child, uint32_t index) {
  RAX_ASSERT(parent && child);
  RAX_ASSERT(parent->in_tree() && child->not_in_tree());
  parent->AddChildIndex(child, index);
  child->MountToTree();
  RAX_ASSERT(RaxTestEnv::CheckRootTreeValid());
}

void RemoveNode(native_node_ptr node) {
  RAX_ASSERT(node->parent());  // root can't be removed.
  RAX_ASSERT(node->parent()->in_tree());
  node->parent()->RemoveChild(node);
  node->UnmountFromTree();
  RAX_ASSERT(RaxTestEnv::CheckRootTreeValid());
}

template <typename T>
static uint32_t find_index_of(const std::vector<T> &container, const T &value) {
  const auto &it = std::find(container.begin(), container.end(), value);
  RAX_ASSERT(it != container.end());
  return std::distance(container.begin(), it);
}

void MoveNode(native_node_ptr parent, native_node_ptr child, uint32_t index) {
  RAX_ASSERT(parent && child);
  RAX_ASSERT(child->parent() == parent);  // we only do move in same parent.
  uint32_t index_of_child = find_index_of(parent->get_children(), child);
  RAX_ASSERT(index <= index_of_child);  // we only support move to front;
  RAX_ASSERT(parent->in_tree() && child->in_tree());

  parent->RemoveChildIndex(index_of_child);
  parent->AddChildIndex(child, index);
  RAX_ASSERT(RaxTestEnv::CheckRootTreeValid());
}

void UpdateAttr(native_node_ptr node,
                std::vector<std::pair<std::string, std::string>> *attrs) {
  RAX_ASSERT(node);
  RAX_ASSERT(node->in_tree());
  RAX_ASSERT(attrs);
  for (auto &it : *attrs) {
    node->UpdateAttr(it.first, it.second);
  }
  delete attrs;
}
void UpdateStyle(native_node_ptr node,
                 std::vector<std::pair<std::string, std::string>> *styles) {
  RAX_ASSERT(node);
  RAX_ASSERT(node->in_tree());
  RAX_ASSERT(styles);
  for (auto &it : *styles) {
    node->UpdateStyle(it.first, it.second);
  }
  delete styles;
}
void AddEvent(native_node_ptr node, const std::string &event) {
  RAX_ASSERT(node);
  RAX_ASSERT(node->in_tree());
  node->AddEvent(event);
}
void RemoveEvent(native_node_ptr node, const std::string &event) {
  RAX_ASSERT(node);
  RAX_ASSERT(node->in_tree());
  node->RemoveEvent(event);
}

void NativeNodeRemoveNode(native_node_ptr node) {
  RAX_ASSERT(node->parent());  // root can't be removed.
  RAX_ASSERT(node->parent()->not_in_tree());
  RAX_ASSERT(node->not_in_tree());
  node->parent()->RemoveChild(node);
  RAX_ASSERT(RaxTestEnv::CheckRootTreeValid());
}

void NativeNodeInsertNode(native_node_ptr parent, native_node_ptr child,
                          uint32_t index) {
  RAX_ASSERT(parent && child);
  RAX_ASSERT(parent->not_in_tree());
  RAX_ASSERT(child->not_in_tree());
  parent->AddChildIndex(child, index);
  RAX_ASSERT(RaxTestEnv::CheckRootTreeValid());
}
void NativeNodeMoveNode(native_node_ptr parent, native_node_ptr child,
                        uint32_t index) {
  RAX_ASSERT(parent && child);
  RAX_ASSERT(parent->not_in_tree());
  RAX_ASSERT(child->not_in_tree());
  RAX_ASSERT(child->parent() == parent);  // we only do move in same parent.
  uint32_t index_of_child = find_index_of(parent->get_children(), child);
  RAX_ASSERT(index <= index_of_child);  // we only support move to front;

  parent->RemoveChildIndex(index_of_child);
  parent->AddChildIndex(child, index);
  RAX_ASSERT(RaxTestEnv::CheckRootTreeValid());
}
void NativeNodeUpdateAttr(
    native_node_ptr node,
    std::vector<std::pair<std::string, std::string>> *attrs) {
  RAX_ASSERT(node);
  RAX_ASSERT(node->not_in_tree());
  RAX_ASSERT(attrs);
  for (auto &it : *attrs) {
    node->UpdateAttr(it.first, it.second);
  }
  delete attrs;
}
void NativeNodeUpdateStyle(
    native_node_ptr node,
    std::vector<std::pair<std::string, std::string>> *styles) {
  RAX_ASSERT(node);
  RAX_ASSERT(node->not_in_tree());
  RAX_ASSERT(styles);
  for (auto &it : *styles) {
    node->UpdateStyle(it.first, it.second);
  }
  delete styles;
}
void NativeNodeAddEvent(native_node_ptr node, const std::string &event) {
  RAX_ASSERT(node);
  RAX_ASSERT(node->not_in_tree());
  node->AddEvent(event);
}
void NativeNodeRemoveEvent(native_node_ptr node, const std::string &event) {
  RAX_ASSERT(node);
  RAX_ASSERT(node->not_in_tree());
  node->RemoveEvent(event);
}

}  // namespace render_bridge
RAX_NAME_SPACE_END

#endif
