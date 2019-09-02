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


#include "weex_runtime_quickjs.h"
#include "weex_jsc_utils.h"

#define countof(x) (sizeof(x) / sizeof((x)[0]))

static JSValue js_GCAndSweep(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv);

static JSValue js_CallNative(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv);

static JSValue js_CallNativeModule(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_CallNativeComponent(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv);

static JSValue js_CallAddElement(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv);

static JSValue js_SetTimeoutNative(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_NativeLog(JSContext *ctx, JSValueConst this_val,
                            int argc, JSValueConst *argv);

static JSValue js_NotifyTrimMemory(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_MarkupState(JSContext *ctx, JSValueConst this_val,
                              int argc, JSValueConst *argv);

static JSValue js_Atob(JSContext *ctx, JSValueConst this_val,
                       int argc, JSValueConst *argv);

static JSValue js_Btoa(JSContext *ctx, JSValueConst this_val,
                       int argc, JSValueConst *argv);

static JSValue js_CallCreateBody(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv);

static JSValue js_CallUpdateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_CallCreateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_CallRefreshFinish(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_CallUpdateAttrs(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_CallUpdateStyle(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_CallRemoveElement(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_CallMoveElement(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_CallAddEvent(JSContext *ctx, JSValueConst this_val,
                               int argc, JSValueConst *argv);

static JSValue js_CallRemoveEvent(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_GCanvasLinkNative(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_SetIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_ClearIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_T3DLinkNative(JSContext *ctx, JSValueConst this_val,
                                int argc, JSValueConst *argv);

static JSValue js_NativeLogContext(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_DisPatchMeaage(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv);

static JSValue js_DispatchMessageSync(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv);

static JSValue js_PostMessage(JSContext *ctx, JSValueConst this_val,
                              int argc, JSValueConst *argv);

static JSValue js_NativeSetTimeout(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_NativeSetInterval(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_NativeClearTimeout(JSContext *ctx, JSValueConst this_val,
                                     int argc, JSValueConst *argv);

static JSValue js_NativeClearInterval(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv);

// For data render
static JSValue js_UpdateComponentData(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv);


WeexRuntimeQuickJS::WeexRuntimeQuickJS(TimerQueue *timeQueue,
                                       WeexCore::ScriptBridge *script_bridge,
                                       bool isMultiProgress) : WeexRuntime(script_bridge) {
    m_jsRuntime = JS_NewRuntime();
    m_jsContextFramework = this->createContext();
}

bool WeexRuntimeQuickJS::hasInstanceId(std::string &id) {
    return false;
}


void WeexRuntimeQuickJS::initFrameworkParams(JSContext *ctx,
                                             std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    static bool isSave = true;
    static std::vector<INIT_FRAMEWORK_PARAMS *> m_initFrameworkParams;
    bool useSavedParams = params.empty();
    int size = useSavedParams ? m_initFrameworkParams.size() : params.size();

    const JSValue &jsValue = JS_NewObject(ctx);
    const JSValue &object = JS_GetGlobalObject(ctx);
    for (int i = 0; i < size; i++) {
        INIT_FRAMEWORK_PARAMS *param = useSavedParams ? m_initFrameworkParams[i] : params[i];

        std::string &&type = std::string(param->type->content);
        std::string &&value = std::string(param->value->content);

        JSCFunctionListEntry jsEntry = JS_PROP_STRING_DEF(type.c_str(),
                                                     value.c_str(),
                                                     JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
        JS_SetPropertyStr(ctx, jsValue, type.c_str(), JS_NewString(ctx, value.c_str()));
        if (isSave) {
            auto init_framework_params = (INIT_FRAMEWORK_PARAMS *) malloc(sizeof(INIT_FRAMEWORK_PARAMS));
            if (init_framework_params == nullptr) {
                return;
            }
            memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));
            init_framework_params->type = genWeexByteArraySS(param->type->content, param->type->length);
            init_framework_params->value =
                    genWeexByteArraySS(param->value->content, param->value->length);
            m_initFrameworkParams.push_back(init_framework_params);
            isSave = false;
        }
    }

    JS_SetProperty(ctx, object, JS_NewAtom(ctx, "WXEnvironment"), jsValue);

}

int WeexRuntimeQuickJS::initFramework(const std::string &script,
                                      std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    initFrameworkParams(m_jsContextFramework, params);
    bindGlobalContextFunctions(m_jsContextFramework);
    const JSValue &value = JS_Eval(m_jsContextFramework, script.c_str(), script.length(), "jsFramework",
                                   JS_EVAL_TYPE_GLOBAL);

    if (JS_IsException(value)) {
        const JSValue &jsValue = JS_GetException(m_jsContextFramework);
        return 0;
    } else {
        return 1;
    }
}

int WeexRuntimeQuickJS::initAppFramework(const std::string &instanceId,
                                         const std::string &appFramework,
                                         std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    return 0;
}

int WeexRuntimeQuickJS::createAppContext(const std::string &instanceId,
                                         const std::string &jsBundle) {
    return 0;
}

std::unique_ptr<WeexJSResult> WeexRuntimeQuickJS::exeJSOnAppWithResult(const std::string &instanceId,
                                                                       const std::string &jsBundle) {
    return NULL;
}

int WeexRuntimeQuickJS::callJSOnAppContext(const std::string &instanceId,
                                           const std::string &func,
                                           std::vector<VALUE_WITH_TYPE *> &params) {
    return 0;
}

int WeexRuntimeQuickJS::destroyAppContext(const std::string &instanceId) {
    return 0;
}

int WeexRuntimeQuickJS::exeJsService(const std::string &source) {
    return 0;
}

int WeexRuntimeQuickJS::exeCTimeCallback(const std::string &source) {
    return 0;
}

int WeexRuntimeQuickJS::exeJS(const std::string &instanceId,
                              const std::string &nameSpace,
                              const std::string &func,
                              std::vector<VALUE_WITH_TYPE *> &params) {
    return 0;
}

std::unique_ptr<WeexJSResult> WeexRuntimeQuickJS::exeJSWithResult(const std::string &instanceId,
                                                                  const std::string &nameSpace,
                                                                  const std::string &func,
                                                                  std::vector<VALUE_WITH_TYPE *> &params) {
    return NULL;
}

void WeexRuntimeQuickJS::exeJSWithCallback(const std::string &instanceId,
                                           const std::string &nameSpace,
                                           const std::string &func,
                                           std::vector<VALUE_WITH_TYPE *> &params,
                                           long callback_id) {

}

int WeexRuntimeQuickJS::createInstance(const std::string &instanceId,
                                       const std::string &func,
                                       const std::string &script,
                                       const std::string &opts,
                                       const std::string &initData,
                                       const std::string &extendsApi,
                                       std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    return 0;
}

std::unique_ptr<WeexJSResult> WeexRuntimeQuickJS::exeJSOnInstance(const std::string &instanceId,
                                                                  const std::string &script) {
    return NULL;
}

int WeexRuntimeQuickJS::destroyInstance(const std::string &instanceId) {
    return 0;
}

int WeexRuntimeQuickJS::updateGlobalConfig(const std::string &config) {
    return 0;
}

JSContext *WeexRuntimeQuickJS::createContext() {
    JSRuntime *rt = this->m_jsRuntime;
    JSContext *ctx = JS_NewContextRaw(rt);
    JS_AddIntrinsicEval(ctx);
    JS_AddIntrinsicRegExpCompiler(ctx);
    /* loader for ES6 modules */
//  JS_SetModuleLoaderFunc(rt, NULL, jsc_module_loader, NULL);
    return ctx;
}


void WeexRuntimeQuickJS::bindGlobalContextFunctions(JSContext *ctx) {

    static const JSCFunctionListEntry js_fib_funcs[] = {
            JS_CFUNC_DEF("callNative", 3, js_CallNative),
            JS_CFUNC_DEF("callNativeModule", 5, js_CallNativeModule),
            JS_CFUNC_DEF("callNativeComponent", 5, js_CallNativeComponent),
            JS_CFUNC_DEF("callAddElement", 5, js_CallAddElement),
            JS_CFUNC_DEF("setTimeoutNative", 2, js_SetTimeoutNative),
            JS_CFUNC_DEF("nativeLog", 5, js_NativeLog),
            JS_CFUNC_DEF("notifyTrimMemory", 0, js_NotifyTrimMemory),
            JS_CFUNC_DEF("markupState", 0, js_MarkupState),
            JS_CFUNC_DEF("atob", 1, js_Atob),
            JS_CFUNC_DEF("btoa", 1, js_Btoa),
            JS_CFUNC_DEF("callCreateBody", 3, js_CallCreateBody),
            JS_CFUNC_DEF("callUpdateFinish", 3, js_CallUpdateFinish),
            JS_CFUNC_DEF("callCreateFinish", 3, js_CallCreateFinish),
            JS_CFUNC_DEF("callRefreshFinish", 3, js_CallRefreshFinish),
            JS_CFUNC_DEF("callUpdateAttrs", 4, js_CallUpdateAttrs),
            JS_CFUNC_DEF("callUpdateStyle", 4, js_CallUpdateStyle),
            JS_CFUNC_DEF("callRemoveElement", 5, js_CallRemoveElement),
            JS_CFUNC_DEF("callMoveElement", 5, js_CallMoveElement),
            JS_CFUNC_DEF("callAddEvent", 4, js_CallAddEvent),
            JS_CFUNC_DEF("callRemoveEvent", 4, js_CallRemoveEvent),
            JS_CFUNC_DEF("callGCanvasLinkNative", 3, js_GCanvasLinkNative),
            JS_CFUNC_DEF("setIntervalWeex", 3, js_SetIntervalWeex),
            JS_CFUNC_DEF("clearIntervalWeex", 1, js_ClearIntervalWeex),
            JS_CFUNC_DEF("callT3DLinkNative", 2, js_T3DLinkNative),
            JS_CFUNC_DEF("__updateComponentData", 3, js_UpdateComponentData),
    };

    JS_SetPropertyFunctionList(ctx, JS_GetGlobalObject(ctx), js_fib_funcs, countof(js_fib_funcs));

}

void WeexRuntimeQuickJS::bindInstanceContextFunctions(JSContext *ctx) {
    static const JSCFunctionListEntry js_fib_funcs[] = {
            JS_CFUNC_DEF("nativeLog", 5, js_NativeLog),
            JS_CFUNC_DEF("atob", 1, js_Atob),
            JS_CFUNC_DEF("btoa", 1, js_Btoa),
            JS_CFUNC_DEF("callGCanvasLinkNative", 3, js_GCanvasLinkNative),
            JS_CFUNC_DEF("setIntervalWeex", 3, js_SetIntervalWeex),
            JS_CFUNC_DEF("clearIntervalWeex", 1, js_ClearIntervalWeex),
            JS_CFUNC_DEF("callT3DLinkNative", 2, js_T3DLinkNative),
            JS_CFUNC_DEF("__updateComponentData", 3, js_UpdateComponentData),
    };

    JS_SetPropertyFunctionList(ctx, JS_GetGlobalObject(ctx), js_fib_funcs, countof(js_fib_funcs));
}
int WeexRuntimeQuickJS::exeTimerFunctionForRunTimeApi(const std::string &instanceId,
                                                      uint32_t timerFunction,
                                                      bool is_from_instance) {
  return 0;
}
void WeexRuntimeQuickJS::removeTimerFunctionForRunTimeApi(const std::string &instanceId,
                                                          const uint32_t timerFunction,
                                                          bool is_from_instance) {

}

static JSValue js_GCAndSweep(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallNative(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallNativeModule(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallNativeComponent(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallAddElement(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_SetTimeoutNative(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_NativeLog(JSContext *ctx, JSValueConst this_val,
                            int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_NotifyTrimMemory(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_MarkupState(JSContext *ctx, JSValueConst this_val,
                              int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_Atob(JSContext *ctx, JSValueConst this_val,
                       int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_Btoa(JSContext *ctx, JSValueConst this_val,
                       int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallCreateBody(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallUpdateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallCreateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallRefreshFinish(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallUpdateAttrs(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallUpdateStyle(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallRemoveElement(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallMoveElement(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallAddEvent(JSContext *ctx, JSValueConst this_val,
                               int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_CallRemoveEvent(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_GCanvasLinkNative(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_SetIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_ClearIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_T3DLinkNative(JSContext *ctx, JSValueConst this_val,
                                int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_NativeLogContext(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_DisPatchMeaage(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_DispatchMessageSync(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_PostMessage(JSContext *ctx, JSValueConst this_val,
                              int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_NativeSetTimeout(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_NativeSetInterval(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_NativeClearTimeout(JSContext *ctx, JSValueConst this_val,
                                     int argc, JSValueConst *argv) {return JS_NULL;}

static JSValue js_NativeClearInterval(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {return JS_NULL;}

// For data render
static JSValue js_UpdateComponentData(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {return JS_NULL;}

