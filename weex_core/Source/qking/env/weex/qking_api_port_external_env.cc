#include "api/qking_api.h"

#ifdef QKING_ENABLE_EXTERNAL_WEEX_ENV
#include "core/data_render/vnode/vnode_render_manager.h"
#include "core/manager/weex_core_manager.h"

namespace qking {
namespace api {

qking_value_t qking_api_port_call_native_module(qking_executor_t executor,
                                                const std::string &module,
                                                const std::string &method,
                                                const std::string &args,
                                                int argc) {
  return weex::core::data_render::VNodeRenderManager::GetInstance()
      ->CallNativeModule(executor, module.c_str(), method.c_str(),
                         argc > 0 ? args.c_str() : "[]", argc);
}

bool qking_api_port_require_module(std::string &name, std::string &info) {
  return weex::core::data_render::VNodeRenderManager::GetInstance()
      ->RequireModule(name, info);
}

void qking_api_port_print(const char *pcstr) {
  WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->NativeLog(
      pcstr);
}

}  // namespace api
}  // namespace qking

#endif
