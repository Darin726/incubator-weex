//
// Created by Xu Jiacheng on 2018/12/26.
//

#include "rax_text_element.h"
#include "base/qking_common_logger.h"
#include "rax_core_util.h"
#include "rax_element_factory.h"
#include "rax_ref.h"
#include "rax_render_bridge.h"

RAX_NAME_SPACE_BEGIN

using namespace render_bridge;

RaxTextElement::RaxTextElement(uint32_t eid, const std::string &text)
    : text_(text), RaxNativeNodeHolder(eid, RaxElementType::kText) {}

void RaxTextElement::MountComponentInternal(native_node_ptr render_parent,
                                            RaxElement *component_parent,
                                            const ChildMounter &mounter) {
  // text element can't be mount under Native<text> node.
  RAX_ASSERT(component_parent->type() != RaxElementType::kText);
  // create native
  native_node() = NativeTextNodeCreate(this);
  RAX_ASSERT(native_node());

  if (mounter) {
    mounter(render_parent, native_node());
  } else {
    NativeNodeAddChildren(render_parent, native_node());
  }
}

void RaxTextElement::UnmountComponentInternal(bool not_remove_child) {
  RAX_ASSERT(native_node());
  if (not_remove_child) {
    return;
  }
  if (is_mount_to_root()) {
    RemoveNode(native_node());
  } else {
    NativeNodeRemoveNode(native_node());
  }
}

void RaxTextElement::UpdateComponentInternal(RaxElement *prev_ele,
                                             RaxElement *next_ele,
                                             uint32_t insert_start) {
  RaxTextElement *next_txt = dynamic_cast<RaxTextElement *>(next_ele);
  const std::string &new_str = next_txt->text();

  if (new_str == text_) {
    return;
  }

  text_ = new_str;
  std::vector<std::pair<std::string, std::string>> *p_vec =
      new std::vector<std::pair<std::string, std::string>>{{"value", text_}};
  if (is_mount_to_root()) {
    UpdateAttr(native_node(), p_vec);
  } else {
    NativeNodeUpdateAttr(native_node(), p_vec);
  }
}

void RaxTextElement::DebugPrintComponent() {
  RaxElement::DebugPrintComponent();
  RAX_LOGD("RaxComponent(%u): Text \"%s\"", eid(), text().c_str());
}

Json::object RaxTextElement::DebugCollectComponent() {
  Json::object obj = RaxElement::DebugCollectComponent();
  obj["text"] = text();
  return obj;
}
native_node_ptr RaxTextElement::get_first_render_node() {
  RAX_ASSERT(native_node());
  return native_node();
}
native_node_ptr RaxTextElement::get_last_render_node() {
  RAX_ASSERT(native_node());
  return native_node();
}
RAX_NAME_SPACE_END
