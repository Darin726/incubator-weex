//
// Created by Xu Jiacheng on 2019-01-23.
//

#include "rax_native_node_holder.h"
#include "rax_ref.h"
RAX_NAME_SPACE_BEGIN

qking_value_t RaxNativeNodeHolder::get_js_native_node() {
  if (!js_native_node_) {
    if (comp_native_node_) {
      // has mounted
      qking_value_t obj = rax_create_native_node_obj(eid_str());
      js_native_node_.reset(new QKValueRef(obj));
      qking_release_value(obj);
      return obj;
    } else {
      // not mounted yet
      return qking_create_undefined();
    }
  } else {
    return js_native_node_->get();
  }
}

RAX_NAME_SPACE_END