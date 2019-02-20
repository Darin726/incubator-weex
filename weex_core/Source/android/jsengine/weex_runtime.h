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
// Created by Darin on 28/04/2018.
//

#ifndef WEEXV8_JSRUNTIME_H
#define WEEXV8_JSRUNTIME_H

#include "android/jsengine/task/timer_queue.h"
#include "android/jsengine/object/weex_object_holder.h"
#include "android/jsengine/weex_ipc_client.h"

namespace WeexCore {
    class ScriptBridge;
}
class WeexRuntime {

private:
    std::unique_ptr<WeexObjectHolder> weexObjectHolder;
    std::map<std::string, WeexObjectHolder *> appWorkerContextHolderMap;
public:

    WeexCore::ScriptBridge* script_bridge_;

    RefPtr<VM> m_globalVM;

    explicit WeexRuntime();

  explicit WeexRuntime(TimerQueue* timeQueue, bool isMultiProgress = true);

    explicit WeexRuntime(TimerQueue* timeQueue, WeexCore::ScriptBridge *script_bridge, bool isMultiProgress = true);

    virtual bool hasInstanceId(String &id);

    virtual int initFramework(IPCArguments *arguments);

    virtual int initFramework(const String &script, std::vector<INIT_FRAMEWORK_PARAMS *> &params);

    virtual int initAppFrameworkMultiProcess(const String &instanceId, const String &appFramework, IPCArguments *arguments);

    virtual int
    initAppFramework(const String &instanceId, const String &appFramework, std::vector<INIT_FRAMEWORK_PARAMS *> &params);

    virtual int createAppContext(const String &instanceId, const String &jsBundle);

    virtual std::unique_ptr<WeexJSResult> exeJSOnAppWithResult(const String &instanceId, const String &jsBundle);

    virtual int callJSOnAppContext(IPCArguments *arguments);

    virtual int callJSOnAppContext(const String &instanceId, const String &func, std::vector<VALUE_WITH_TYPE *> &params);

    virtual int destroyAppContext(const String &instanceId);

    virtual int exeJsService(const String &source);

    virtual int exeCTimeCallback(const String &source);

//    int exeJS(const String &instanceId, const String &nameSpace, const String &func, IPCArguments *arguments);

    virtual int
    exeJS(const String &instanceId, const String &nameSpace, const String &func, std::vector<VALUE_WITH_TYPE *> &params);

//    std::unique_ptr<WeexJSResult>  exeJSWithResult(const String &instanceId, const String &nameSpace, const String &func,
//                          IPCArguments *arguments);

    virtual std::unique_ptr<WeexJSResult>  exeJSWithResult(const String &instanceId, const String &nameSpace, const String &func,
                          std::vector<VALUE_WITH_TYPE *> &params);

    virtual void exeJSWithCallback(const String &instanceId, const String &nameSpace, const String &func,
                          std::vector<VALUE_WITH_TYPE *> &params, long callback_id);

    virtual int createInstance(const String &instanceId, const String &func, const String &script, const String &opts,
                       const String &initData, const String &extendsApi,
                       std::vector<INIT_FRAMEWORK_PARAMS*>& params);

    virtual std::unique_ptr<WeexJSResult> exeJSOnInstance(const String &instanceId, const String &script);

    virtual int destroyInstance(const String &instanceId);

    virtual int updateGlobalConfig(const String &config);

    virtual int exeTimerFunction(const String &instanceId, uint32_t timerFunction, JSGlobalObject *globalObject);


    virtual WeexObjectHolder * getLightAppObjectHolder(const String &instanceId);

    virtual void removeTimerFunction(const uint32_t timerFunction, JSGlobalObject *globalObject);

    virtual int exeTimerFunctionForRunTimeApi(const String &instanceId, uint32_t timerFunction, bool is_from_instance){
        return 0;
    }

    virtual void removeTimerFunctionForRunTimeApi(const String &instanceId,const uint32_t timerFunction, bool is_from_instance){

    }

private:
    int _initFramework(const String &source);

    int _initAppFramework(const String &instanceId, const String &appFramework);

    void _getArgListFromIPCArguments(MarkedArgumentBuffer *obj, ExecState *state, IPCArguments *arguments,
                                     size_t start);

    void _getArgListFromJSParams(MarkedArgumentBuffer *obj, ExecState *state, std::vector<VALUE_WITH_TYPE *> &params);

    bool is_multi_process_;

    String m_source;
};


#endif //WEEXV8_JSRUNTIME_H
