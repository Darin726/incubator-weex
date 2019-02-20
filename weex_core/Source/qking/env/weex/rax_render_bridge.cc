#ifdef QKING_ENABLE_EXTERNAL_WEEX_ENV

#include "rax_render_bridge.h"
#include "base/qking_common_logger.h"
#include "rax/rax_common.h"
#include "rax_core_util.h"
#include "rax_native_element.h"

RAX_NAME_SPACE_BEGIN

namespace render_bridge {

using WeexCore::RenderCreator;
using WeexCore::RenderManager;
using WeexCore::RenderObject;

native_node_ptr NativeNodeCreate(RaxNativeElement *rax_element) {
  WeexCore::RenderObject *ele = static_cast<WeexCore::RenderObject *>(
      RenderCreator::GetInstance()->CreateRender(
          rax_element->actrual_comp_type(), rax_element->eid_str()));
  const auto &styles = rax_element->get_styles();
  const auto &attrs = rax_element->get_attrs();
  const auto &events = rax_element->get_events();

  ele->set_page_id(rax_get_current_page_name());

  for (const auto &it : styles) {
    ele->AddStyle(it.first, it.second);
  }

  for (const auto &it : attrs) {
    ele->AddAttr(it.first, it.second);
  }

  for (const auto &it : events) {
    ele->AddEvent(it.first);
  }
  ele->ApplyDefaultStyle();
  ele->ApplyDefaultAttr();
  return ele;
}

native_node_ptr NativeTextNodeCreate(RaxTextElement *rax_element) {
  RenderObject *ele = static_cast<WeexCore::RenderObject *>(
      RenderCreator::GetInstance()->CreateRender("text",
                                                 rax_element->eid_str()));

  ele->set_page_id(rax_get_current_page_name());

  ele->UpdateAttr("value", rax_element->text());

  ele->ApplyDefaultStyle();
  ele->ApplyDefaultAttr();
  return ele;
}

void NativeNodeAddChildren(native_node_ptr parent, native_node_ptr child) {
  RAX_ASSERT(parent);
  RAX_ASSERT(child);
  parent->AddRenderObject(static_cast<int>(parent->getChildCount()), child);
}

int32_t NativeNodeIndexOf(native_node_ptr parent, native_node_ptr child) {
  return parent->getChildIndex(child);
}

native_node_ptr NativeNodeChildOfIndex(native_node_ptr parent, uint32_t index) {
  RAX_ASSERT(parent);
  RAX_ASSERT(index < parent->getChildCount());
  return parent->GetChild(index);
}

native_node_ptr NativeNodeCreateRootNode() {
  auto ele = static_cast<WeexCore::RenderObject *>(
      RenderCreator::GetInstance()->CreateRender("div", ROOT_REF));

  ele->set_page_id(rax_get_current_page_name());

  ele->ApplyDefaultStyle();
  ele->ApplyDefaultAttr();
  return ele;
}

void SetRootNode(native_node_ptr node) {
  RenderManager::GetInstance()->CreatePage(node->page_id(), node);
  RenderManager::GetInstance()->CreateFinish(node->page_id());
}

void InsertNode(native_node_ptr parent, native_node_ptr child, uint32_t index) {
  RAX_ASSERT(parent && child);
  RenderManager::GetInstance()->AddRenderObject(parent->page_id(),
                                                parent->ref(), index, child);
}

void RemoveNode(native_node_ptr node) {
  LOGW("KKK RemvoeNode %s", node->ref().c_str());
  RAX_ASSERT(node->parent_render());  // root can't be removed.
  RenderManager::GetInstance()->RemoveRenderObject(node->page_id(),
                                                   node->ref());
}

template <typename T>
static uint32_t find_index_of(const std::vector<T> &container, const T &value) {
  const auto &it = std::find(container.begin(), container.end(), value);
  RAX_ASSERT(it != container.end());
  return static_cast<uint32_t>(std::distance(container.begin(), it));
}

void MoveNode(native_node_ptr parent, native_node_ptr child, uint32_t index) {
  RAX_ASSERT(parent && child);
  RAX_ASSERT(child->parent_render() ==
             parent);  // we only do move in same parent.
  int32_t raw_index = parent->getChildIndex(child);
  RAX_ASSERT(raw_index >= 0);
  uint32_t index_of_child = static_cast<uint32_t>(raw_index);
  RAX_ASSERT(index <= index_of_child);  // we only support move to front;

  RenderManager::GetInstance()->MoveRenderObject(
      parent->page_id(), child->ref(), parent->ref(), index);
}
void UpdateAttr(native_node_ptr node,
                std::vector<std::pair<std::string, std::string>> *attrs) {
  RAX_ASSERT(node);
  RAX_ASSERT(attrs);
  RenderManager::GetInstance()->UpdateAttr(node->page_id(), node->ref(), attrs);
}
void UpdateStyle(native_node_ptr node,
                 std::vector<std::pair<std::string, std::string>> *styles) {
  RAX_ASSERT(node);
  RAX_ASSERT(styles);
  RenderManager::GetInstance()->UpdateStyle(node->page_id(), node->ref(),
                                            styles);
}
void AddEvent(native_node_ptr node, const std::string &event) {
  RAX_ASSERT(node);
  RenderManager::GetInstance()->AddEvent(node->page_id(), node->ref(), event);
}
void RemoveEvent(native_node_ptr node, const std::string &event) {
  RAX_ASSERT(node);
  RenderManager::GetInstance()->RemoveEvent(node->page_id(), node->ref(),
                                            event);
}

void NativeNodeMoveNode(native_node_ptr parent, native_node_ptr child,
                        uint32_t index) {
  RAX_ASSERT(index <= parent->getChildCount());
  RAX_ASSERT(parent->getChildIndex(child) >= 0);
  parent->RemoveRenderObject(child);
  parent->addChildAt(child, index);
}

void NativeNodeRemoveEvent(native_node_ptr node, const std::string &event) {
  node->RemoveEvent(event);
}

void NativeNodeAddEvent(native_node_ptr node, const std::string &event) {
  node->AddEvent(event);
}

void NativeNodeUpdateStyle(
    native_node_ptr node,
    std::vector<std::pair<std::string, std::string>> *styles) {
  if (!styles) return;
  for (auto &it : *styles) {
    node->UpdateStyle(it.first, it.second);
  }
  delete styles;
}

void NativeNodeUpdateAttr(
    native_node_ptr node,
    std::vector<std::pair<std::string, std::string>> *attrs) {
  if (!attrs) return;
  for (auto &it : *attrs) {
    node->UpdateAttr(it.first, it.second);
  }
  delete attrs;
}

void NativeNodeInsertNode(native_node_ptr parent, native_node_ptr child,
                          uint32_t index) {
  parent->addChildAt(child, index);
}

void NativeNodeRemoveNode(native_node_ptr node) {
  node->parent_render()->RemoveRenderObject(node);
}
}  // namespace render_bridge

std::string
rax_get_current_page_name() {  // should use extern context to get name
  weex::core::data_render::VNodeRenderContext *context =
      (weex::core::data_render::VNodeRenderContext *)
          qking_get_current_external_context();
  return context->page_id();
}

RAX_NAME_SPACE_END

#endif
