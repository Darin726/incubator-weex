//
// Created by Xu Jiacheng on 2018/12/25.
//

#include "rax_test_env.h"
#include "rax_builtin_env.h"
#include "rax_core_util.h"
#include "rax_element_factory.h"
#include "rax_test_render_object.h"

#ifdef QKING_ENABLE_EXTERNAL_UNIT_TEST_ENV

RaxTestRenderObject *RaxTestEnv::g_root_ = nullptr;
static std::string page_id = "1";

RAX_NAME_SPACE_BEGIN
std::string rax_get_current_page_name() { return page_id; }
RAX_NAME_SPACE_END

static qking_value_t LogRenderRoot(const qking_value_t function_obj,
                                   const qking_value_t this_val,
                                   const qking_value_t *args_p,
                                   const qking_length_t args_count) {
  RAX_ASSERT(RaxTestEnv::g_root_);
  std::string result = Json(RaxTestEnv::g_root_->DebugPrint()).dump();
  RAX_LOGD("RenderObj: %s", result.c_str());

  return qking_create_string_from_utf8_c(result.c_str());
}

static qking_value_t LogComponent(const qking_value_t function_obj,
                                  const qking_value_t this_val,
                                  const qking_value_t *args_p,
                                  const qking_length_t args_count) {
  auto factory = RAX_NS::RaxElementFactory::GetFactory(page_id);
  RAX_ASSERT(factory->root());
  const Json::object &comp_root = factory->root()->DebugCollectComponent();
  std::string result = Json(comp_root).dump();
  RAX_LOGD("ComponentTree: %s", result.c_str());

  return qking_create_string_from_utf8_c(result.c_str());
}

static qking_value_t LogEleCompDetail(const qking_value_t function_obj,
                                      const qking_value_t this_val,
                                      const qking_value_t *args_p,
                                      const qking_length_t args_count) {
  auto factory = RAX_NS::RaxElementFactory::GetFactory(page_id);
  factory->DebugPrintElements();
  factory->DebugPrintMountedComponent();
  return qking_create_undefined();
}

void RaxTestEnv::PrepareRaxTestEnv() {
  g_root_ = nullptr;
  RAX_NS::RaxElementFactory::CreateFactory(page_id);
  RAX_NS::qking_rax_register_builtin_env();
  qking_external_handler_register_global("__LogRenderRoot", LogRenderRoot);
  qking_external_handler_register_global("__LogComponent", LogComponent);
  qking_external_handler_register_global("__LogEleCompDetail",
                                         LogEleCompDetail);
}

void RaxTestEnv::DestroyRaxEnv() {
  RAX_LOGI("===============Destroy Start===============");
  RAX_NS::RaxElementFactory::DestroyFactory(page_id);
}

bool RaxTestEnv::CheckRootTreeValid() {
  if (g_root_) {
    return g_root_->CheckValid(true);
  } else {
    return true;
  }
}

#endif
