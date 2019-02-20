#include "api/qking_api.h"

#ifdef QKING_ENABLE_EXTERNAL_UNIT_TEST_ENV

namespace qking {
namespace api {

qking_value_t qking_api_port_call_native_module(qking_executor_t executor,
                                                const std::string &module,
                                                const std::string &method,
                                                const std::string &args,
                                                int argc) {
  return qking_create_undefined();
}

bool qking_api_port_require_module(std::string &name, std::string &info) {
  return false;
}

void qking_api_port_print(const char *pcstr) {}

}  // namespace api
}  // namespace qking

#endif
