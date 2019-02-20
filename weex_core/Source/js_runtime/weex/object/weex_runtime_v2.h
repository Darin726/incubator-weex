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
// Created by 陈佩翰 on 2019/2/22.
//

#ifndef PROJECT_WEEX_RUNTIME_V2_H
#define PROJECT_WEEX_RUNTIME_V2_H

#include "android/jsengine/weex_runtime.h"
#include "weex_object_holder_v2.h"


class WeexRuntimeV2 : public WeexRuntime {

public:
    explicit WeexRuntimeV2(TimerQueue *timeQueue, bool isMultiProgress);

    explicit WeexRuntimeV2(TimerQueue *timeQueue, WeexCore::ScriptBridge *script_bridge, bool isMultiProgress);

    bool hasInstanceId(String &id) override;

    //int initFramework(IPCArguments *arguments) override;

    int initFramework(const String &script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) override;

//    int initAppFrameworkMultiProcess(const String &instanceId, const String &appFramework,
//                                     IPCArguments *arguments) override;

    int
    initAppFramework(const String &instanceId, const String &appFramework,
                     std::vector<INIT_FRAMEWORK_PARAMS *> &params) override;

    int createAppContext(const String &instanceId, const String &jsBundle) override;

    std::unique_ptr<WeexJSResult> exeJSOnAppWithResult(const String &instanceId, const String &jsBundle) override;

    // int callJSOnAppContext(IPCArguments *arguments) override;

    int
    callJSOnAppContext(const String &instanceId, const String &func, std::vector<VALUE_WITH_TYPE *> &params) override;

    int destroyAppContext(const String &instanceId) override;

    int exeJsService(const String &source) override;

    int exeCTimeCallback(const String &source) override;

//    int exeJS(const String &instanceId, const String &nameSpace, const String &func, IPCArguments *arguments);

    int
    exeJS(const String &instanceId, const String &nameSpace, const String &func,
          std::vector<VALUE_WITH_TYPE *> &params) override;

//    std::unique_ptr<WeexJSResult>  exeJSWithResult(const String &instanceId, const String &nameSpace, const String &func,
//                          IPCArguments *arguments);

    std::unique_ptr<WeexJSResult> exeJSWithResult(const String &instanceId, const String &nameSpace, const String &func,
                                                  std::vector<VALUE_WITH_TYPE *> &params) override;

    void exeJSWithCallback(const String &instanceId, const String &nameSpace, const String &func,
                           std::vector<VALUE_WITH_TYPE *> &params, long callback_id) override;

    int createInstance(const String &instanceId, const String &func, const String &script, const String &opts,
                       const String &initData, const String &extendsApi,
                       std::vector<INIT_FRAMEWORK_PARAMS *> &params) override;

    std::unique_ptr<WeexJSResult> exeJSOnInstance(const String &instanceId, const String &script) override;

    int destroyInstance(const String &instanceId) override;

    int updateGlobalConfig(const String &config) override;

    int exeTimerFunction(const String &instanceId, uint32_t timerFunction, JSGlobalObject *globalObject) override;

    WeexObjectHolder *getLightAppObjectHolder(const String &instanceId) override;

    WeexObjectHolderV2 *getLightAppObjectHolderV2(const String &instanceId);

    int exeTimerFunctionForRunTimeApi(const String &instanceId, uint32_t timerFunction, bool is_from_instance) override;

    void removeTimerFunctionForRunTimeApi(const String &instanceId,const uint32_t timerFunction, bool is_from_instance) override;


protected:
    int _initFrameworkWithScript(const String &source);

    int _initAppFrameworkWithScript(const String &instanceId, const String &appFramework);

    void _geJSRuntimeArgsFromWeexParams(unicorn::EngineContext *context, std::vector<unicorn::ScopeValues> *obj,
                                        std::vector<VALUE_WITH_TYPE *> &params);

    WeexGlobalObjectV2* findWeexObj(const String &instanceId, bool is_instance);


protected:
    std::unique_ptr<WeexObjectHolderV2> weex_object_holder_v2_;
    std::map<std::string, WeexObjectHolderV2 *> app_worker_context_holder_map_v2_;
    unicorn::RuntimeVM *vm_;
    bool multi_process_flag_;
};


#endif //PROJECT_WEEX_RUNTIME_V2_H
