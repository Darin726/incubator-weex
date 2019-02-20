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

#include <js_runtime/runtime/js_runtime_conversion.h>
#include <js_runtime/weex/utils/weex_conversion_utils.h>
#include <object/weex_env.h>
#include "weex_runtime_v2.h"
#include "js_runtime/runtime/runtime_vm.h"
#include "core/bridge/script_bridge.h"
#include "js_runtime/runtime/engine_context.h"
#include "js_runtime/utils/log_utils.h"

WeexRuntimeV2::WeexRuntimeV2(TimerQueue *timeQueue, bool isMultiProgress)
        : WeexRuntime() {

}

WeexRuntimeV2::WeexRuntimeV2(TimerQueue *timeQueue, WeexCore::ScriptBridge *script_bridge, bool isMultiProgress)
        : WeexRuntimeV2(timeQueue, isMultiProgress) {
    this->script_bridge_ = script_bridge;
//    if (!WEEXICU::initICUEnv(isMultiProgress)) {
//        LOG_RUNTIME("[initICUEnv]failed to init ICUEnv single process");
//    }
//    //todo if delete ,will crash
//    WTF::initializeMainThread();
//    initHeapTimer();

    //  });
    WeexEnv::getEnv()->initJSC(isMultiProgress);
    WeexEnv::getEnv()->jsc_init_finished();
    WeexEnv::getEnv()->locker()->signal();
    //create vm
    this->vm_ = new unicorn::RuntimeVM();
    //code
    weex_object_holder_v2_.reset(new WeexObjectHolderV2(this->vm_, timeQueue, isMultiProgress));

}

bool WeexRuntimeV2::hasInstanceId(String &id) {
    auto iterator = app_worker_context_holder_map_v2_.find(id.utf8().data());
    if (iterator != app_worker_context_holder_map_v2_.end()) {
        return true;
    }
    auto it = weex_object_holder_v2_->m_jsInstanceGlobalObjectMap.find(id.utf8().data());
    return it != weex_object_holder_v2_->m_jsInstanceGlobalObjectMap.end();
}

int WeexRuntimeV2::initFramework(const String &script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    weex_object_holder_v2_->initFromParams(params, false);
    return this->_initFrameworkWithScript(script);
}

//int WeexRuntimeV2::initFramework(IPCArguments *arguments) {
//    weex_object_holder_v2_->initFromIPCArguments(arguments, 1, false);
//    const IPCString *ipcSource = arguments->getString(0);
//    const String &source = jString2String(ipcSource->content, ipcSource->length);
//    return _initFrameworkWithScript(source);
//}
//
//int WeexRuntimeV2::initAppFrameworkMultiProcess(const String &instanceId, const String &appFramework,
//                                                IPCArguments *arguments) {
//    auto pHolder = getLightAppObjectHolder(instanceId);
//    if (pHolder == nullptr) {
//        auto holder = new WeexObjectHolderV2(this->vm_, weex_object_holder_v2_->timeQueue, true);
//        holder->initFromIPCArguments(arguments, 2, true);
//        app_worker_context_holder_map_v2_[instanceId.utf8().data()] = holder;
//    }
//
//    return _initAppFrameworkWithScript(instanceId, appFramework);
//}

int WeexRuntimeV2::initAppFramework(const String &instanceId, const String &appFramework,
                                    std::vector<INIT_FRAMEWORK_PARAMS *> &params) {

    auto pHolder = getLightAppObjectHolderV2(instanceId);
    LOG_RUNTIME("Weex jsserver initAppFramework %s", instanceId.utf8().data());
    if (pHolder == nullptr) {
        auto holder = new WeexObjectHolderV2(this->vm_, weex_object_holder_v2_->timeQueue, multi_process_flag_);
        holder->initFromParams(params, true);
        LOG_RUNTIME("Weex jsserver initAppFramework pHolder == null and id %s", instanceId.utf8().data());
        app_worker_context_holder_map_v2_[std::string(instanceId.utf8().data())] = holder;
    }
    return _initAppFrameworkWithScript(instanceId, appFramework);
}

int WeexRuntimeV2::createAppContext(const String &instanceId, const String &jsBundle) {

    if (instanceId == "") {
        return static_cast<int32_t>(false);
    }

    String pre = "";
    if (instanceId.length() > 6) {
        pre = instanceId.substring(0, 7);
    }

    String get_context_fun_name = "";
    String final_instanceId = "";
    if (pre == "plugin_") {
        get_context_fun_name = "__get_plugin_context__";
        final_instanceId = instanceId.substring(7, instanceId.length() - 7);
    } else {
        get_context_fun_name = "__get_app_context__";
        final_instanceId = instanceId;
    }

    LOG_RUNTIME("WeexRuntime  createAppContext get_context_fun_name = %s", get_context_fun_name.utf8().data());

    // new a global object
    // --------------------------------------------------
    auto appWorkerObjectHolderV2 = getLightAppObjectHolderV2(final_instanceId);
    if (appWorkerObjectHolderV2 == nullptr) {
        return static_cast<int32_t>(false);
    }

    auto worker_globalObject = appWorkerObjectHolderV2->globalObject.get();
    if (worker_globalObject == nullptr) {
        LOGE("WeexRuntime createAppContext worker_globalObject is null");
        return static_cast<int32_t>(false);
    }
    auto app_globalObject = appWorkerObjectHolderV2->createAppWorkerObject();
    app_globalObject->setScriptBridge(script_bridge_);


    // Create App Context from Worker
    std::vector<unicorn::ScopeValues> args;
    std::string jsException;
    JSRunTimeValue funcRet = worker_globalObject->context->GetEngineContext()->callJavaScriptFunc(
            nullptr,
            std::string(get_context_fun_name.utf8().data()),
            args,
            &jsException
    );
    if (nullptr == funcRet) {
        LOGE("WeexRuntime: createAppContext failed , exception :%s", jsException.c_str());
        return static_cast<int32_t>(false);
    }
    LOG_RUNTIME("WeexRuntime: try convert funcRetValue to obj");
    JSRunTimeObject fucRetJSObject = worker_globalObject->context->GetEngineContext()->toObjectFromValue(funcRet);
    if (nullptr == fucRetJSObject) {
        LOGE("WeexRuntime: CreateAppContext get funcRet obj failed");
        return static_cast<int32_t>(false);
    }

    std::vector<std::string> nameArray;

    if (!worker_globalObject->context->GetEngineContext()->GetObjectPropertyNameArray(fucRetJSObject, nameArray)) {
        LOGE("WeexRuntime:  createAppContext  get fucRetJSObject properties name array failed");
        return static_cast<int32_t>(false);
    }

    for (auto propertyName : nameArray) {
        auto propertyValue = worker_globalObject->context->GetEngineContext()->GetPropertyValueFromObject(
                propertyName, fucRetJSObject);
        if (nullptr == propertyValue) {
            LOGE("WeexRuntime:  createAppContext  get fucRetJSObject properties value failed, name:%s",
                 propertyName.c_str());
            return static_cast<int32_t>(false);
        }
        app_globalObject->context->GetEngineContext()->setObjectValue(nullptr, propertyName, propertyValue);
    }

    app_globalObject->id = std::string(final_instanceId.utf8().data());
    LOG_RUNTIME("WeexRuntime:createAppContext instanceId.utf8().data() %s", final_instanceId.utf8().data());

    if (!jsBundle.isEmpty()) {
        std::string exeption;
        if (!app_globalObject->context->ExecuteJavaScript(std::string(jsBundle.utf8().data()), &exeption)) {
            if (!exeption.empty()) {
                app_globalObject->js_bridge()->core_side()->ReportException(instanceId.utf8().data(),
                                                                            "createAppContext", exeption.c_str());
            }
            return static_cast<int32_t>(false);
        }
    } else {
        LOGE("WeexRuntime: createAppContext app.js is empty!");
        return static_cast<int32_t>(false);
    }
    LOG_RUNTIME("WeexRuntime: createAppContext complete!");
    return static_cast<int32_t>(true);
}

std::unique_ptr<WeexJSResult> WeexRuntimeV2::exeJSOnAppWithResult(const String &instanceId, const String &jsBundle) {
    LOGE("WeexRuntime: exeJSOnAppWithResult app.js is empty!!!!!!!!!!!, instanceId:%s, jsBundle:%s",
         instanceId.utf8().data(), jsBundle.utf8().data());
    //return WeexRuntime::exeJSOnAppWithResult(instanceId, jsBundle);
    std::unique_ptr<WeexJSResult> returnResult;
    return returnResult;
}

//int WeexRuntimeV2::callJSOnAppContext(IPCArguments *arguments) {
//    LOGE("WeexRuntimeV2 ERROR : CALL  callJSOnAppContext IPCArguments");
//    return 0;
//}

int WeexRuntimeV2::callJSOnAppContext(const String &instanceId, const String &func,
                                      std::vector<VALUE_WITH_TYPE *> &params) {
    if (instanceId == "" || func == "") {
        return static_cast<int32_t>(false);
    }
    std::string runFunc = std::string(func.utf8().data());
    LOG_RUNTIME("WeexRuntime callJSOnAppContext func is %s", runFunc.c_str());

    // Get Worker global object
    auto appWorkerHolder = getLightAppObjectHolderV2(instanceId);
    if (appWorkerHolder == nullptr) {
        return static_cast<int32_t>(false);
    }

    auto worker_globalObject = appWorkerHolder->globalObject.get();
    if (worker_globalObject == nullptr) {
        return static_cast<int32_t>(false);
    }

    std::vector<unicorn::ScopeValues> args;
    _geJSRuntimeArgsFromWeexParams(worker_globalObject->context->GetEngineContext(), &args, params);

    std::string jsException;
    worker_globalObject->context->GetEngineContext()->callJavaScriptFunc(
            nullptr,
            runFunc,
            args,
            &jsException
    );

    if (!jsException.empty()) {
        worker_globalObject->js_bridge()->core_side()->ReportException(instanceId.utf8().data(), func.utf8().data(),
                                                                       jsException.c_str());
        LOGE("callJSOnAppContext error on instance %s ,func:%s", instanceId.utf8().data(), runFunc.c_str());
        return static_cast<int32_t>(false);
    }
    LOG_RUNTIME("WeexRuntime callJSOnAppContext func complete: %s", runFunc.c_str());
    return static_cast<int32_t>(true);
}

int WeexRuntimeV2::destroyAppContext(const String &instanceId) {
    auto appWorkerObjectHolder = getLightAppObjectHolderV2(instanceId);
    if (appWorkerObjectHolder == nullptr) {
        return static_cast<int32_t>(false);
    }

    LOG_RUNTIME("Weex jsserver IPCJSMsg::DESTORYAPPCONTEXT end1 %s", instanceId.utf8().data());
    std::map<std::string, WeexGlobalObjectV2 *>::iterator it_find_instance;
    auto objectMap = appWorkerObjectHolder->m_jsInstanceGlobalObjectMap;
    it_find_instance = objectMap.find(instanceId.utf8().data());
    if (it_find_instance != objectMap.end()) {
        // LOGE("Weex jsserver IPCJSMsg::DESTORYAPPCONTEXT mAppInstanceGlobalObjectMap donnot contain and return");
        objectMap.erase(instanceId.utf8().data());
    }

    if (appWorkerObjectHolder->timeQueue != nullptr)
        appWorkerObjectHolder->timeQueue->destroyPageTimer(instanceId.utf8().data());

    app_worker_context_holder_map_v2_.erase(instanceId.utf8().data());
    delete appWorkerObjectHolder;
    appWorkerObjectHolder = nullptr;
    return static_cast<int32_t>(true);
}

int WeexRuntimeV2::exeJsService(const String &source) {
    std::string source_str = std::string(source.utf8().data());

    std::string js_exception;
    bool succeed = weex_object_holder_v2_->globalObject->context->ExecuteJavaScript(source_str, &js_exception);

    if (!js_exception.empty()) {
        weex_object_holder_v2_->globalObject->js_bridge()->core_side()->ReportException("service", "exeJsService",
                                                                                        js_exception.c_str());
    }

    if (!succeed) {
        LOGE("exec service error :%s ,script: :%s", js_exception.c_str(), source_str.c_str());
        return static_cast<int32_t>(false);
    } else {
        return static_cast<int32_t>(true);
    }
}

int WeexRuntimeV2::exeCTimeCallback(const String &source) {
    std::string source_str = std::string(source.utf8().data());
    if (!weex_object_holder_v2_->globalObject->context->ExecuteJavaScript(source_str, nullptr)) {
        LOGE("jsLog JNI_Error EXECTIMERCALLBACK >>> scriptStr :%s", source_str.c_str());
        return static_cast<int32_t>(false);
    } else {
        return static_cast<int32_t>(true);
    }
}

int WeexRuntimeV2::exeJS(const String &instanceId, const String &nameSpace, const String &func,
                         std::vector<VALUE_WITH_TYPE *> &params) {
    //LOGE("jsengine shopp EXECJS func:%s and params size is %d", func.utf8().data(), params.size());

    std::string runFunc = std::string(func.utf8().data());
    std::string instance_id_str = std::string(instanceId.utf8().data());

    WeexGlobalObjectV2 *globalObject = nullptr;

    if (runFunc == "callJS") {
        globalObject = weex_object_holder_v2_->m_jsInstanceGlobalObjectMap[instance_id_str];
        if (globalObject == nullptr) {
            LOG_RUNTIME("[runtime2]----------->[global] exeJS func:%s", runFunc.c_str());
            globalObject = weex_object_holder_v2_->globalObject.get();
        } else {
            runFunc = std::string("__WEEX_CALL_JAVASCRIPT__");
            LOG_RUNTIME("[runtime2]----------->[instance][%s] exeJS func:%s", instance_id_str.c_str(), runFunc.c_str());
        }
    } else {
        globalObject = weex_object_holder_v2_->globalObject.get();
        LOG_RUNTIME("[runtime2]----------->[global] exeJS func:%s", runFunc.c_str());
    }

    std::vector<unicorn::ScopeValues> args;
    _geJSRuntimeArgsFromWeexParams(globalObject->context->GetEngineContext(), &args, params);

    std::string jsException;
    globalObject->context->GetEngineContext()->callJavaScriptFunc(
            nullptr,
            runFunc,
            args,
            &jsException
    );

    if (!jsException.empty()) {
        globalObject->js_bridge()->core_side()->ReportException(instance_id_str.c_str(), runFunc.c_str(),
                                                                jsException.c_str());
        LOGE("[runtime2]exeJS error on instance %s ,func:%s", instance_id_str.c_str(), runFunc.c_str());
        return static_cast<int32_t>(false);
    }
    return static_cast<int32_t>(true);
}

std::unique_ptr<WeexJSResult>
WeexRuntimeV2::exeJSWithResult(const String &instanceId, const String &nameSpace, const String &func,
                               std::vector<VALUE_WITH_TYPE *> &params) {

    std::string instance_id_str = std::string(instanceId.utf8().data());
    std::string runFunc = std::string(func.utf8().data());

    LOG_RUNTIME("[runtime2]WeexRuntime exeJSWithResult func is %s", runFunc.c_str());

    std::unique_ptr<WeexJSResult> returnResult;
    returnResult.reset(new WeexJSResult);

    // JSGlobalObject *globalObject;
    WeexGlobalObjectV2 *globalObject = nullptr;
    // fix instanceof Object error
    // if function is callJs should us Instance object to call __WEEX_CALL_JAVASCRIPT__
    if (runFunc == "callJS") {
        globalObject = weex_object_holder_v2_->m_jsInstanceGlobalObjectMap[instance_id_str];
        if (globalObject == nullptr) {
            globalObject = weex_object_holder_v2_->globalObject.get();
        } else {
            runFunc = std::string("__WEEX_CALL_JAVASCRIPT__");
        }
    } else {
        globalObject = weex_object_holder_v2_->globalObject.get();
    }

    std::vector<unicorn::ScopeValues> args;
    _geJSRuntimeArgsFromWeexParams(globalObject->context->GetEngineContext(), &args, params);

    std::string jsException;
    auto values = globalObject->context->GetEngineContext()->CallJavaScriptFuncWithRuntimeValue(
            nullptr,
            runFunc,
            args,
            &jsException
    );

    if (!jsException.empty()) {
        globalObject->js_bridge()->core_side()->ReportException(instance_id_str.c_str(), runFunc.c_str(),
                                                                jsException.c_str());
        LOGE("[runtime2]exeJS error on instance %s ,func:%s", instance_id_str.c_str(), runFunc.c_str());
        return returnResult;
    }
    //convert values to returnResult
    weex::jsengine::WeexConversionUtils::ConvertRunTimeValueToWeexJSResult(values, returnResult.get());
    return returnResult;
}

void WeexRuntimeV2::exeJSWithCallback(const String &instanceId, const String &nameSpace, const String &func,
                                      std::vector<VALUE_WITH_TYPE *> &params, long callback_id) {
    auto result = exeJSWithResult(instanceId, nameSpace, func, params);
    script_bridge_->core_side()->OnReceivedResult(callback_id, result);
}

int
WeexRuntimeV2::createInstance(const String &instanceId, const String &func, const String &script, const String &opts,
                              const String &initData, const String &extendsApi,
                              std::vector<INIT_FRAMEWORK_PARAMS *> &params) {

    LOG_RUNTIME("WeexRuntime: create createInstance  Context");


    WeexGlobalObjectV2 *impl_globalObject = weex_object_holder_v2_->globalObject.get();
    WeexGlobalObjectV2 *globalObject;
    if (instanceId == "") {
        LOGE("[runtime2]WeexRuntime:  globalObject = impl_globalObject");
        globalObject = impl_globalObject;
    } else {
        std::string instance_id_str = std::string(instanceId.utf8().data());
        auto *temp_object = weex_object_holder_v2_->m_jsInstanceGlobalObjectMap[instance_id_str];

        if (temp_object == nullptr) {
            temp_object = weex_object_holder_v2_->createInstancecObject(instance_id_str, instance_id_str);
            temp_object->addExtraOptions(params);
            temp_object->id = instance_id_str;
            temp_object->setScriptBridge(script_bridge_);
            temp_object->timeQueue = weex_object_holder_v2_->timeQueue;

            std::string opts_str = std::string(opts.utf8().data());
            std::string init_data_str = std::string(initData.utf8().data());

            std::vector<unicorn::ScopeValues> args;
            args.push_back(unicorn::RuntimeValues::MakeString(instance_id_str));
            args.push_back(unicorn::RuntimeValues::MakeObjectFromJsonStr(opts_str.c_str()));
            args.push_back(unicorn::RuntimeValues::MakeObjectFromJsonStr(init_data_str.c_str()));
            std::string jsException;


            auto funcRet = impl_globalObject->context->GetEngineContext()->callJavaScriptFunc(
                    nullptr,
                    std::string("createInstanceContext"),
                    args,
                    &jsException
            );

            if (!jsException.empty()) {
                temp_object->js_bridge()->core_side()->ReportException(instanceId.utf8().data(), "run raxApi",
                                                                       jsException.c_str());
            }

            if (nullptr == funcRet) {
                LOGE("WeexRuntime: createInstance failed");
                return static_cast<int32_t>(false);
            }

            //1. set vue's prototype to newContext's globalObject prototype
            JSRunTimeObject fucRetJSObject = impl_globalObject->context->GetEngineContext()->toObjectFromValue(funcRet);
            if (nullptr == fucRetJSObject) {
                LOGE("WeexRuntime: get funcRet obj failed");
                return static_cast<int32_t>(false);
            }

            auto vueJSValue = impl_globalObject->context->GetEngineContext()->GetPropertyValueFromObject("Vue",
                                                                                                         fucRetJSObject);
            if (nullptr != vueJSValue) {
                LOG_RUNTIME("WeexRuntime:  try set vue's prototype to newContext's globalObject prototypet");
                auto vueObject = impl_globalObject->context->GetEngineContext()->toObjectFromValue(vueJSValue);
                if (!temp_object->context->GetEngineContext()->SetObjectPrototypeFromValue(vueObject, nullptr)) {
                    LOGE("WeexRuntime: failed ====> set vue's prototype to newContext's globalObject prototype");
                }
            }

            std::vector<std::string> nameArray;

            if (!impl_globalObject->context->GetEngineContext()->GetObjectPropertyNameArray(fucRetJSObject,
                                                                                            nameArray)) {
                LOGE("WeexRuntime:   get fucRetJSObject properties name array failed");
                return static_cast<int32_t>(false);
            }

            LOG_RUNTIME(
                    "WeexRuntime:   get fucRetJSObject properties name array succeed,globalContext:%p,instance context:%p",
                    impl_globalObject->context->GetEngineContext()->GetContext(),
                    temp_object->context->GetEngineContext()->GetContext()
            );

            for (auto propertyName : nameArray) {
                auto propertyValue = impl_globalObject->context->GetEngineContext()->GetPropertyValueFromObject(
                        propertyName, fucRetJSObject);
                if (nullptr == propertyValue) {
                    LOGE("WeexRuntime:   get fucRetJSObject properties value failed, name:%s", propertyName.c_str());
                    return static_cast<int32_t>(false);
                }
                //LOG_RUNTIME("WeexRuntime:  set global item [%s,%p] ,this:%p", propertyName.c_str(),propertyValue,this);
//
//                if (propertyName == "weex"){
//                    std::vector<std::string> weexPropties;
//                    JSRunTimeObject weexObject = impl_globalObject->context->GetEngineContext()->toObjectFromValue(propertyValue);
//                    impl_globalObject->context->GetEngineContext()->GetObjectPropertyNameArray(weexObject, weexPropties);
//
//                    for(auto weexKey : weexPropties){
//                        LOG_RUNTIME("WeexRuntime:  weex.config [%s,%p] ,this:%p", weexKey.c_str(),propertyValue,this);
//                    }
//                }


                temp_object->context->GetEngineContext()->setObjectValue(nullptr, propertyName, propertyValue);
            }
            weex_object_holder_v2_->m_jsInstanceGlobalObjectMap[instance_id_str] = temp_object;
            LOG_RUNTIME("create Instance succeed :%s", instance_id_str.c_str());
        }
        globalObject = temp_object;
    }

    std::string js_exception;
    if (!extendsApi.isEmpty()) {
        LOGE("[runtime2] run rax api ,length:%d", extendsApi.length());
        if (!globalObject->context->ExecuteJavaScript(std::string(extendsApi.utf8().data()), &js_exception)) {
            LOGE("before createInstanceContext run rax api Error :%s", js_exception.c_str());
            if (!js_exception.empty()) {
                globalObject->js_bridge()->core_side()->ReportException(instanceId.utf8().data(), "run raxApi",
                                                                        js_exception.c_str());
            }
            return static_cast<int32_t>(false);
        }
    }

    if (!script.isEmpty()) {
        if (!globalObject->context->GetEngineContext()->RunJavaScript(std::string(script.utf8().data()),
                                                                      &js_exception)) {
            LOGE("createInstanceContext and ExecuteJavaScript Error :%s", js_exception.c_str());
            if (!js_exception.empty()) {
                globalObject->js_bridge()->core_side()->ReportException(instanceId.utf8().data(),
                                                                        "createInstanceContext", js_exception.c_str());
            }
            return static_cast<int32_t>(false);
        }
    }
    return static_cast<int32_t>(true);
}

std::unique_ptr<WeexJSResult> WeexRuntimeV2::exeJSOnInstance(const String &instanceId, const String &script) {
    std::string instance_id_str = std::string(instanceId.utf8().data());

    std::unique_ptr<WeexJSResult> returnResult;
    returnResult.reset(new WeexJSResult);

    // JSGlobalObject *globalObject = weexObjectHolder->m_jsInstanceGlobalObjectMap[instanceId.utf8().data()];
    WeexGlobalObjectV2 *globalObject = weex_object_holder_v2_->m_jsInstanceGlobalObjectMap[instance_id_str];
    if (globalObject == nullptr) {
        //   globalObject = weexObjectHolder->m_globalObject.get();
        globalObject = weex_object_holder_v2_->globalObject.get();
    }
    std::string jsException;
    auto execResult = globalObject->context->ExecuteJavaScriptWithResult(std::string(script.utf8().data()),
                                                                         &jsException);

    if (!jsException.empty()) {
        LOGE("exec JS on instance %s, exception:%s", instance_id_str.c_str(), jsException.c_str());
        globalObject->js_bridge()->core_side()->ReportException(instance_id_str.c_str(), "execJSOnInstance",
                                                                jsException.c_str());
        return nullptr;
    }
    // const char *data = returnValue.toWTFString(globalObject->globalExec()).utf8().data();
    std::string execResultStr;
    execResult.get()->GetAsString(&execResultStr);
    const char *data = execResultStr.c_str();
    returnResult->length = strlen(data);
    char *buf = new char[returnResult->length + 1];
    strcpy(buf, data);
    returnResult->data.reset(buf);
    return returnResult;

}

int WeexRuntimeV2::destroyInstance(const String &instanceId) {
    std::string instance_id_str = std::string(instanceId.utf8().data());
    // LOGE("IPCJSMsg::DESTORYINSTANCE");
    auto *globalObject = weex_object_holder_v2_->m_jsInstanceGlobalObjectMap[instance_id_str];
    if (globalObject == nullptr) {
        return static_cast<int32_t>(true);
    }
    // LOGE("DestoryInstance map 11 length:%d", weexObjectHolder->m_jsGlobalObjectMap.size());
    weex_object_holder_v2_->m_jsInstanceGlobalObjectMap.erase(instance_id_str);
    // LOGE("DestoryInstance map 22 length:%d", weexObjectHolder->m_jsGlobalObjectMap.size());
    if (weex_object_holder_v2_->timeQueue != nullptr) {
        weex_object_holder_v2_->timeQueue->destroyPageTimer(instance_id_str.c_str());
    }
    LOGE("[WEEX][release] delete object");
    delete globalObject;
    globalObject = nullptr;
    return static_cast<int32_t>(true);
}

int WeexRuntimeV2::updateGlobalConfig(const String &config) {
    doUpdateGlobalSwitchConfig(config.utf8().data());
    return static_cast<int32_t>(true);
}

int WeexRuntimeV2::exeTimerFunction(const String &instanceId, uint32_t timerFunction, JSGlobalObject *globalObject) {
    LOGE("[runtimev2] should not exec timer here!!!!");
    //return WeexRuntime::exeTimerFunction(instanceId, timerFunction, globalObject);
    return 0;

}

int WeexRuntimeV2::_initFrameworkWithScript(const String &source) {

    weex_object_holder_v2_->globalObject->setScriptBridge(script_bridge_);
    std::string exception;

    weex_object_holder_v2_->globalObject->context->ExecuteJavaScript(std::string(source.utf8().data()), &exception);

    if (!exception.empty()) {
        weex_object_holder_v2_->globalObject->js_bridge()->core_side()->ReportException("jsfm", "_initFramework",
                                                                                        exception.c_str());
        return false;
    }
    std::vector<unicorn::ScopeValues> args;
    auto version = weex_object_holder_v2_->globalObject->context->GetEngineContext()->callJavaScriptFunc(
            nullptr,
            std::string("getJSFMVersion"),
            args,
            &exception
    );
    auto versionStr = unicorn::JSRuntimeConversion::JSRunTimeValueToRuntimeValue(
            weex_object_holder_v2_->globalObject->context->GetEngineContext(), nullptr, version
    );


    std::string jsfmVersion;

    if (nullptr == versionStr.get() || !versionStr.get()->IsString() || !versionStr.get()->GetAsString(&jsfmVersion)) {
        LOGE("WeexRuntime getJSFMVersion failed ,version:%s, exception: %s", jsfmVersion.c_str(), exception.c_str());
        return false;
    }
    weex_object_holder_v2_->globalObject->js_bridge()->core_side()->SetJSVersion(jsfmVersion.c_str());
    return true;
}

int WeexRuntimeV2::_initAppFrameworkWithScript(const String &instanceId, const String &appFramework) {
    LOG_RUNTIME("WeexRuntime _initAppFramework implements %s", instanceId.utf8().data());
    auto appWorkerObjectHolder = getLightAppObjectHolderV2(instanceId);
    if (appWorkerObjectHolder == nullptr) {
        LOGE("WeexRuntime _initAppFramework implements appWorkerHolder is null");
        return static_cast<int32_t>(false);
    }
    auto worker_globalObject = appWorkerObjectHolder->globalObject.get();
    worker_globalObject->setScriptBridge(script_bridge_);
    worker_globalObject->id = std::string(instanceId.utf8().data());

    std::string js_exception;
    if (!worker_globalObject->context->ExecuteJavaScript(std::string(appFramework.utf8().data()), &js_exception)) {
        if (!js_exception.empty()) {
            worker_globalObject->js_bridge()->core_side()->ReportException(instanceId.utf8().data(), "initAppFramework",
                                                                           js_exception.c_str());
        }
        LOGE("WeexRuntime run worker failed");
        return static_cast<int32_t>(false);
    }

    LOG_RUNTIME("WeexRuntime _initAppFramework implements complete");
    return static_cast<int32_t>(true);
}

WeexObjectHolderV2 *WeexRuntimeV2::getLightAppObjectHolderV2(const String &instanceId) {
    auto iterator = app_worker_context_holder_map_v2_.find(instanceId.utf8().data());
    if (iterator == app_worker_context_holder_map_v2_.end()) {
        return nullptr;
    }
    return app_worker_context_holder_map_v2_.at(instanceId.utf8().data());
}

WeexObjectHolder *WeexRuntimeV2::getLightAppObjectHolder(const String &instanceId) {
    LOGE("WeexRuntimeV2 getLightAppObjectHolder error func");
    return nullptr;
}

void
WeexRuntimeV2::_geJSRuntimeArgsFromWeexParams(unicorn::EngineContext *context, std::vector<unicorn::ScopeValues> *obj,
                                              std::vector<VALUE_WITH_TYPE *> &params) {
    for (unsigned int i = 0; i < params.size(); i++) {
        VALUE_WITH_TYPE *paramsObject = params[i];
        obj->push_back(weex::jsengine::WeexConversionUtils::WeexValueToRuntimeValue(context, paramsObject));
    }
}

int
WeexRuntimeV2::exeTimerFunctionForRunTimeApi(const String &instanceId, uint32_t timerFunction, bool is_from_instance) {

    auto weexObject = findWeexObj(instanceId, is_from_instance);
    LOG_WEEX_BINDING("exeTimerFunctionForRunTimeApi :findWeexObj %p", weexObject);

    if (weexObject == nullptr) {
        LOGE("exeTimerFunctionForRunTimeApi  nullptr == weexObject");
        return 0;
    }
    auto func = weexObject->getTimerFunction(timerFunction);
    LOG_WEEX_BINDING("exeTimerFunctionForRunTimeApi :getTimerFunction %p", func);
    std::vector<unicorn::ScopeValues> args;
    if (nullptr == func) {
        LOGE("api: timer callback func is null");
        return 0;
    }

    LOG_WEEX_BINDING("exeTimerFunctionForRunTimeApi :before GetAsFunction ");
    auto function = func->GetAsFunction();
    LOG_WEEX_BINDING("exeTimerFunctionForRunTimeApi :after GetAsFunction function:%p", function);
    auto globalContext = weexObject->context->GetEngineContext();
    LOG_WEEX_BINDING("exeTimerFunctionForRunTimeApi :GetEngineContext %p", globalContext);
    auto jsContext = globalContext->GetContext();
    LOG_WEEX_BINDING("exeTimerFunctionForRunTimeApi :jsContext %p", jsContext);

    //function->SetJSContext(static_cast<JSRunTimeContext>(jsContext));
    //LOG_WEEX_BINDING("run native timer func at instnace :%s, taskId:%d",globalObject->id.c_str(),timerFunction);

    function->Call(
            static_cast<JSRunTimeContext>(jsContext),
            0,
            args
    );
    LOG_WEEX_BINDING("exeTimerFunctionForRunTimeApi :function->Call");
    return 0;
}

void WeexRuntimeV2::removeTimerFunctionForRunTimeApi(const String &instanceId, const uint32_t timerFunction,
                                                     bool is_from_instance) {
    auto weexObject = findWeexObj(instanceId, is_from_instance);
    LOG_WEEX_BINDING("removeTimerFunctionForRunTimeApi weexObj:%p", weexObject);
    if (nullptr == weexObject) {
        LOGE("removeTimerFunctionForRunTimeApi   nullptr == weexObject");
        return;
    }
    LOG_WEEX_BINDING("removeTimerFunctionForRunTimeApi");
    unicorn::RuntimeValues *targetFuncValue = weexObject->removeTimer(timerFunction);

    if (nullptr != targetFuncValue) {
        LOG_WEEX_BINDING("before delete fucn ,instance:%s, taskId:%d ,succeed", instanceId.utf8().data(),
                         timerFunction);
        delete targetFuncValue;
        LOG_WEEX_BINDING("delete fucn ,instance:%s, taskId:%d ,succeed", instanceId.utf8().data(), timerFunction);
        targetFuncValue = nullptr;
    }
}

WeexGlobalObjectV2 *WeexRuntimeV2::findWeexObj(const String &instanceId, bool is_instance) {
    WeexGlobalObjectV2 *weexObject = nullptr;
    if (is_instance) {
        weexObject = weex_object_holder_v2_->m_jsInstanceGlobalObjectMap[std::string(instanceId.utf8().data())];
    } else {
        auto appWorkerHolder = getLightAppObjectHolderV2(instanceId);
        if (appWorkerHolder == nullptr) {
            return nullptr;
        }
        weexObject = appWorkerHolder->globalObject.get();
    }
    return weexObject;
}
