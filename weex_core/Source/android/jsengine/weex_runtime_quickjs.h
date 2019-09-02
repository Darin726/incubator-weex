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
// Created by 董亚运 on 2019-09-02.
//

#ifndef WEEX_PROJECT_WEEX_RUNTIME_QUICKJS_H
#define WEEX_PROJECT_WEEX_RUNTIME_QUICKJS_H

#include "weex_runtime.h"
#include "task/timer_queue.h"
#include "include/WeexApiHeader.h"
#include "task/timer_queue.h"

extern "C"
{
#include "quickjs/quickjs.h"
}

class WeexRuntimeQuickJS : public WeexRuntime {
public:
    WeexRuntimeQuickJS(TimerQueue *timeQueue,
                       WeexCore::ScriptBridge *script_bridge,
                       bool isMultiProgress);

    bool hasInstanceId(std::string &id) override;

    int initFramework(const std::string &script,
                      std::vector<INIT_FRAMEWORK_PARAMS *> &params) override;

    int
    initAppFramework(const std::string &instanceId, const std::string &appFramework,
                     std::vector<INIT_FRAMEWORK_PARAMS *> &params) override;

    int createAppContext(const std::string &instanceId, const std::string &jsBundle) override;

    std::unique_ptr<WeexJSResult> exeJSOnAppWithResult(const std::string &instanceId,
                                                       const std::string &jsBundle) override;

    int
    callJSOnAppContext(const std::string &instanceId,
                       const std::string &func,
                       std::vector<VALUE_WITH_TYPE *> &params) override;

    int destroyAppContext(const std::string &instanceId) override;

    int exeJsService(const std::string &source) override;

    int exeCTimeCallback(const std::string &source) override;

    int
    exeJS(const std::string &instanceId, const std::string &nameSpace, const std::string &func,
          std::vector<VALUE_WITH_TYPE *> &params) override;

    std::unique_ptr<WeexJSResult> exeJSWithResult(const std::string &instanceId,
                                                  const std::string &nameSpace,
                                                  const std::string &func,
                                                  std::vector<VALUE_WITH_TYPE *> &params) override;

    void exeJSWithCallback(const std::string &instanceId,
                           const std::string &nameSpace,
                           const std::string &func,
                           std::vector<VALUE_WITH_TYPE *> &params,
                           long callback_id) override;

    int createInstance(const std::string &instanceId,
                       const std::string &func,
                       const std::string &script,
                       const std::string &opts,
                       const std::string &initData,
                       const std::string &extendsApi,
                       std::vector<INIT_FRAMEWORK_PARAMS *> &params) override;

    std::unique_ptr<WeexJSResult> exeJSOnInstance(const std::string &instanceId,
                                                  const std::string &script) override;

    int destroyInstance(const std::string &instanceId) override;

    int updateGlobalConfig(const std::string &config) override;

    int exeTimerFunctionForRunTimeApi(const std::string &instanceId,
                                      uint32_t timerFunction,
                                      bool is_from_instance) override;

    void removeTimerFunctionForRunTimeApi(const std::string &instanceId,
                                          const uint32_t timerFunction,
                                          bool is_from_instance) override;

    JSContext *createContext();

private:
    void initFrameworkParams(JSContext *ctx, std::vector<INIT_FRAMEWORK_PARAMS *> &params);

    void bindGlobalContextFunctions(JSContext *ctx);

    void bindInstanceContextFunctions(JSContext *ctx);

private:
    JSRuntime *m_jsRuntime;
    JSContext *m_jsContextFramework;
    std::map<std::string, JSContext *> m_contextMap;
    std::vector<INIT_FRAMEWORK_PARAMS *> m_initFrameworkParams;
};

#endif //WEEX_PROJECT_WEEX_RUNTIME_QUICKJS_H
