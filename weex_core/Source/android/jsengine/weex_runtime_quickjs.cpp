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
#include "log_defines.h"
#include "weex_jsc_utils.h"

#define countof(x) (sizeof(x) / sizeof((x)[0]))

static JSValue js_print(JSContext *ctx, JSValueConst this_val, int argc,
                        JSValueConst *argv) {
  int i;
  const char *str;

  for (i = 0; i < argc; i++) {
    if (i != 0) putchar(' ');
    str = JS_ToCString(ctx, argv[i]);
    if (!str) return JS_EXCEPTION;
    // fputs(str, stdout);
    LOGE("dyyLog ConsoleLog %s", str);
    JS_FreeCString(ctx, str);
  }
  putchar('\n');
  return JS_TRUE;
}

static JSValue js_GCAndSweep(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv);

static JSValue js_CallNative(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv);

static JSValue js_CallNativeModule(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_CallNativeComponent(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv);

static JSValue js_CallAddElement(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv);

static JSValue js_SetTimeoutNative(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_NativeLog(JSContext *ctx, JSValueConst this_val, int argc,
                            JSValueConst *argv);

static JSValue js_NotifyTrimMemory(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_MarkupState(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv);

static JSValue js_Atob(JSContext *ctx, JSValueConst this_val, int argc,
                       JSValueConst *argv);

static JSValue js_Btoa(JSContext *ctx, JSValueConst this_val, int argc,
                       JSValueConst *argv);

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

static JSValue js_CallAddEvent(JSContext *ctx, JSValueConst this_val, int argc,
                               JSValueConst *argv);

static JSValue js_CallRemoveEvent(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_GCanvasLinkNative(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_SetIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv);

static JSValue js_ClearIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv);

static JSValue js_T3DLinkNative(JSContext *ctx, JSValueConst this_val, int argc,
                                JSValueConst *argv);

static JSValue js_NativeLogContext(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv);

static JSValue js_DisPatchMeaage(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv);

static JSValue js_DispatchMessageSync(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv);

static JSValue js_PostMessage(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv);

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
                                       bool isMultiProgress)
    : WeexRuntime(script_bridge) {
  m_jsRuntime = JS_NewRuntime();
  m_jsContextFramework = this->createContext();
  LOGE("dyyLog");
}

bool WeexRuntimeQuickJS::hasInstanceId(std::string &id) { return false; }

void WeexRuntimeQuickJS::initFrameworkParams(
    JSContext *ctx, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  static bool isSave = true;
  static std::vector<INIT_FRAMEWORK_PARAMS *> m_initFrameworkParams;
  bool useSavedParams = params.empty();
  int size = useSavedParams ? m_initFrameworkParams.size() : params.size();

  const JSValue &jsValue = JS_NewObject(ctx);
  const JSValue &object = JS_GetGlobalObject(ctx);
  for (int i = 0; i < size; i++) {
    INIT_FRAMEWORK_PARAMS *param =
        useSavedParams ? m_initFrameworkParams[i] : params[i];

    std::string &&type = std::string(param->type->content);
    std::string &&value = std::string(param->value->content);

    JSCFunctionListEntry jsEntry = JS_PROP_STRING_DEF(
        type.c_str(), value.c_str(), JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    JS_SetPropertyStr(ctx, jsValue, type.c_str(),
                      JS_NewString(ctx, value.c_str()));
    if (isSave) {
      auto init_framework_params =
          (INIT_FRAMEWORK_PARAMS *)malloc(sizeof(INIT_FRAMEWORK_PARAMS));
      if (init_framework_params == nullptr) {
        return;
      }
      memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));
      init_framework_params->type =
          genWeexByteArraySS(param->type->content, param->type->length);
      init_framework_params->value =
          genWeexByteArraySS(param->value->content, param->value->length);
      m_initFrameworkParams.push_back(init_framework_params);
      isSave = false;
    }
  }

  JS_SetProperty(ctx, object, JS_NewAtom(ctx, "WXEnvironment"), jsValue);
}

int WeexRuntimeQuickJS::initFramework(
    const std::string &script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  initFrameworkParams(m_jsContextFramework, params);
  bindGlobalContextFunctions(m_jsContextFramework);
  const JSValue &value =
      JS_Eval(m_jsContextFramework, script.c_str(), script.length(),
              "jsFramework", JS_EVAL_TYPE_GLOBAL);

  if (JS_IsException(value)) {
    const JSValue &jsValue = JS_GetException(m_jsContextFramework);

    const char *string = JS_ToCString(m_jsContextFramework, jsValue);

    LOGE("dyyLog JS_IsException %s", string);
    return 0;
  } else {
    LOGE("dyyLog finish");
    return 1;
  }
}

int WeexRuntimeQuickJS::initAppFramework(
    const std::string &instanceId, const std::string &appFramework,
    std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  return 0;
}

int WeexRuntimeQuickJS::createAppContext(const std::string &instanceId,
                                         const std::string &jsBundle) {
  return 0;
}

std::unique_ptr<WeexJSResult> WeexRuntimeQuickJS::exeJSOnAppWithResult(
    const std::string &instanceId, const std::string &jsBundle) {
  return NULL;
}

int WeexRuntimeQuickJS::callJSOnAppContext(
    const std::string &instanceId, const std::string &func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  return 0;
}

int WeexRuntimeQuickJS::destroyAppContext(const std::string &instanceId) {
  return 0;
}

int WeexRuntimeQuickJS::exeJsService(const std::string &source) { return 0; }

int WeexRuntimeQuickJS::exeCTimeCallback(const std::string &source) {
  return 0;
}

int WeexRuntimeQuickJS::exeJS(const std::string &instanceId,
                              const std::string &nameSpace,
                              const std::string &func,
                              std::vector<VALUE_WITH_TYPE *> &params) {
  return 0;
}

std::unique_ptr<WeexJSResult> WeexRuntimeQuickJS::exeJSWithResult(
    const std::string &instanceId, const std::string &nameSpace,
    const std::string &func, std::vector<VALUE_WITH_TYPE *> &params) {
  return NULL;
}

void WeexRuntimeQuickJS::exeJSWithCallback(
    const std::string &instanceId, const std::string &nameSpace,
    const std::string &func, std::vector<VALUE_WITH_TYPE *> &params,
    long callback_id) {}

int WeexRuntimeQuickJS::createInstance(
    const std::string &instanceId, const std::string &func,
    const std::string &script, const std::string &opts,
    const std::string &initData, const std::string &extendsApi,
    std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  JSContext *thisContext;
  if (instanceId.empty()) {
    thisContext = m_jsContextFramework;
  } else {
    thisContext = createContext();
    bindInstanceContextFunctions(thisContext);
    std::vector<INIT_FRAMEWORK_PARAMS *> params;
    initFrameworkParams(thisContext, params);
    JSContext *globalContext = m_jsContextFramework;

    JSValue createInstanceContextFunc =
        JS_GetProperty(globalContext, JS_GetGlobalObject(globalContext),
                       JS_NewAtom(globalContext, "createInstanceContext"));

    JSValue args[3];
    args[0] = JS_NewString(globalContext, instanceId.c_str());
    args[1] = JS_NewString(globalContext, opts.c_str());
    args[2] = JS_NewString(globalContext, initData.c_str());

    JSValue ret = JS_Call(globalContext, createInstanceContextFunc,
                          JS_GetGlobalObject(globalContext), 3, args);

    if (JS_IsException(ret)) {
      const JSValue &exception = JS_GetException(globalContext);
      const char *string = JS_ToCString(globalContext, exception);
      LOGE("dyyLog Create Instance JS_IsException %s", string);
      return 0;
    }

    JSPropertyEnum *tab_atom;
    uint32_t tab_atom_count;
    if (JS_GetOwnPropertyNames(
            globalContext, &tab_atom, &tab_atom_count, ret,
            JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK | JS_GPN_ENUM_ONLY)) {
      return 0;
    }

    for (size_t i = 0; i < tab_atom_count; i++) {
      auto atom = tab_atom[i].atom;

      auto propertyValue = JS_GetProperty(globalContext, ret, atom);

      const char *name = JS_AtomToCString(thisContext, atom);

      auto newAtom = JS_NewAtom(thisContext, name);

      auto thisObject = JS_GetGlobalObject(thisContext);
      JS_SetProperty(thisContext, thisObject, newAtom, propertyValue);

      if (strcmp(name, "Vue") == 0) {
        JS_SetPrototype(thisContext, propertyValue,
                        JS_GetPrototype(thisContext, thisObject));
      }
    }

    this->m_contextMap[instanceId.c_str()] = thisContext;
  }

  if (!extendsApi.empty()) {
    auto ret = JS_Eval(thisContext, extendsApi.c_str(), extendsApi.length(),
                       "extendsApi", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(ret)) {
      const JSValue &exception = JS_GetException(thisContext);
      const char *string = JS_ToCString(thisContext, exception);
      LOGE("dyyLog Create Instance extendsApi JS_IsException %s", string);
      return 0;
    }
  }

  auto createInstanceRet = JS_Eval(thisContext, script.c_str(), script.length(),
                                   "createInstance", JS_EVAL_TYPE_GLOBAL);

  if (JS_IsException(createInstanceRet)) {
    const JSValue &exception = JS_GetException(thisContext);
    const char *string = JS_ToCString(thisContext, exception);
    LOGE("dyyLog Create Instance createInstanceRet JS_IsException %s", string);
    return 0;
  }

  return 1;
}

std::unique_ptr<WeexJSResult> WeexRuntimeQuickJS::exeJSOnInstance(
    const std::string &instanceId, const std::string &script) {
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
  JSContext *ctx = JS_NewContext(rt);
  bindConsoleLog(ctx);
  //    JS_AddIntrinsicBaseObjects(ctx);
  //    JS_AddIntrinsicRegExp(ctx);
  //    JS_AddIntrinsicEval(ctx);
  //    JS_AddIntrinsicRegExpCompiler(ctx);
  /* loader for ES6 modules */
  //  JS_SetModuleLoaderFunc(rt, NULL, jsc_module_loader, NULL);
  return ctx;
}

void WeexRuntimeQuickJS::bindConsoleLog(JSContext *ctx) {
  JSValue global_obj, console;
  global_obj = JS_GetGlobalObject(ctx);
  console = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, console, "log",
                    JS_NewCFunction(ctx, js_print, "log", 1));

  JS_SetPropertyStr(ctx, console, "error",
                    JS_NewCFunction(ctx, js_print, "error", 1));

  JS_SetPropertyStr(ctx, console, "info",
                    JS_NewCFunction(ctx, js_print, "info", 1));

  JS_SetPropertyStr(ctx, console, "debug",
                    JS_NewCFunction(ctx, js_print, "debug", 1));

  JS_SetPropertyStr(ctx, global_obj, "console", console);
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

  JS_SetPropertyFunctionList(ctx, JS_GetGlobalObject(ctx), js_fib_funcs,
                             countof(js_fib_funcs));
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

  JS_SetPropertyFunctionList(ctx, JS_GetGlobalObject(ctx), js_fib_funcs,
                             countof(js_fib_funcs));
}

int WeexRuntimeQuickJS::exeTimerFunctionForRunTimeApi(
    const std::string &instanceId, uint32_t timerFunction,
    bool is_from_instance) {
  return 0;
}

void WeexRuntimeQuickJS::removeTimerFunctionForRunTimeApi(
    const std::string &instanceId, const uint32_t timerFunction,
    bool is_from_instance) {}

static JSValue js_GCAndSweep(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv) {
  LOGE("dyyLog js_GCAndSweep");
  return JS_NULL;
}

static JSValue js_CallNative(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv) {
  LOGE("dyyLog js_CallNative");
  return JS_NULL;
}

static JSValue js_CallNativeModule(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallNativeModule");
  return JS_NULL;
}

static JSValue js_CallNativeComponent(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallNativeComponent");
  return JS_NULL;
}

static JSValue js_CallAddElement(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallAddElement");
  return JS_NULL;
}

static JSValue js_SetTimeoutNative(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_SetTimeoutNative");
  return JS_NULL;
}

static JSValue js_NativeLog(JSContext *ctx, JSValueConst this_val, int argc,
                            JSValueConst *argv) {
  LOGE("dyyLog js_NativeLog");
  return JS_NULL;
}

static JSValue js_NotifyTrimMemory(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NotifyTrimMemory");
  return JS_NULL;
}

static JSValue js_MarkupState(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv) {
  return LOGE("dyyLog js_MarkupState");
  JS_NULL;
}

static JSValue js_Atob(JSContext *ctx, JSValueConst this_val, int argc,
                       JSValueConst *argv) {
  return LOGE("dyyLog js_Atob");
  JS_NULL;
}

static JSValue js_Btoa(JSContext *ctx, JSValueConst this_val, int argc,
                       JSValueConst *argv) {
  return LOGE("dyyLog js_Btoa");
  JS_NULL;
}

static JSValue js_CallCreateBody(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallCreateBody");
  return JS_NULL;
}

static JSValue js_CallUpdateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallUpdateFinish");
  return JS_NULL;
}

static JSValue js_CallCreateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallCreateFinish");
  return JS_NULL;
}

static JSValue js_CallRefreshFinish(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallRefreshFinish");
  return JS_NULL;
}

static JSValue js_CallUpdateAttrs(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallUpdateAttrs");
  return JS_NULL;
}

static JSValue js_CallUpdateStyle(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallUpdateStyle");
  return JS_NULL;
}

static JSValue js_CallRemoveElement(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallRemoveElement");
  return JS_NULL;
}

static JSValue js_CallMoveElement(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallMoveElement");
  return JS_NULL;
}

static JSValue js_CallAddEvent(JSContext *ctx, JSValueConst this_val, int argc,
                               JSValueConst *argv) {
  LOGE("dyyLog js_CallAddEvent");
  return JS_NULL;
}

static JSValue js_CallRemoveEvent(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallRemoveEvent");
  return JS_NULL;
}

static JSValue js_GCanvasLinkNative(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  LOGE("dyyLog js_GCanvasLinkNative");
  return JS_NULL;
}

static JSValue js_SetIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  LOGE("dyyLog js_SetIntervalWeex");
  return JS_NULL;
}

static JSValue js_ClearIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  LOGE("dyyLog js_ClearIntervalWeex");
  return JS_NULL;
}

static JSValue js_T3DLinkNative(JSContext *ctx, JSValueConst this_val, int argc,
                                JSValueConst *argv) {
  LOGE("dyyLog js_T3DLinkNative");
  return JS_NULL;
}

static JSValue js_NativeLogContext(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NativeLogContext");
  return JS_NULL;
}

static JSValue js_DisPatchMeaage(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {
  LOGE("dyyLog js_DisPatchMeaage");
  return JS_NULL;
}

static JSValue js_DispatchMessageSync(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  LOGE("dyyLog js_DispatchMessageSync");
  return JS_NULL;
}

static JSValue js_PostMessage(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv) {
  LOGE("dyyLog js_PostMessage");
  return JS_NULL;
}

static JSValue js_NativeSetTimeout(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NativeSetTimeout");
  return JS_NULL;
}

static JSValue js_NativeSetInterval(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NativeSetInterval");
  return JS_NULL;
}

static JSValue js_NativeClearTimeout(JSContext *ctx, JSValueConst this_val,
                                     int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NativeClearTimeout");
  return JS_NULL;
}

static JSValue js_NativeClearInterval(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NativeClearInterval");
  return JS_NULL;
}

// For data render
static JSValue js_UpdateComponentData(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  LOGE("dyyLog js_UpdateComponentData");
  return JS_NULL;
}
