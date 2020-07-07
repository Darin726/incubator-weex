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
#include <object/weex_env.h>
#include <wson_parser.h>
#include <base/string_util.h>
#include "log_defines.h"
#include "weex_jsc_utils.h"
#include <fstream>

#define countof(x) (sizeof(x) / sizeof((x)[0]))
static std::string toJSON(JSContext *ctx, JSValue value);

static void printWson(std::string msg, wson_buffer *buffer) {
  wson_parser parser((char *) buffer->data);
  const std::string &string = parser.toStringUTF8();
  LOGE("dyyLog printWson %s %s", msg.c_str(), string.c_str());
}

static uint64_t microTime_new() {
  struct timeval tv;

  gettimeofday(&tv, nullptr);

  return (((uint64_t) tv.tv_sec) * MICROSEC + tv.tv_usec)/1000;
}


static JSValue js_print(JSContext *ctx, JSValueConst this_val, int argc,
                        JSValueConst *argv) {
  int i;
  const char *str;

  for (i = 0; i < argc; i++) {
    if (i != 0) putchar(' ');
    str = JS_ToCString(ctx, argv[i]);
    if (!str) return JS_EXCEPTION;
    // fputs(str, stdout);
    LOGE("dyyLog js_CallConsoleLog %s", str);
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
  LOGE("dyyLog useSavedParams %d and size is %d and save is %d and params size is %d", useSavedParams, m_initFrameworkParams.size(), isSave, params.size());
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
          (INIT_FRAMEWORK_PARAMS *) malloc(sizeof(INIT_FRAMEWORK_PARAMS));
      if (init_framework_params == nullptr) {
        return;
      }
      memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));
      init_framework_params->type =
          genWeexByteArraySS(param->type->content, param->type->length);
      init_framework_params->value =
          genWeexByteArraySS(param->value->content, param->value->length);
      m_initFrameworkParams.push_back(init_framework_params);
    }
  }
  isSave = false;
  JS_SetProperty(ctx, object, JS_NewAtom(ctx, "WXEnvironment"), jsValue);
}

int WeexRuntimeQuickJS::initFramework(
    const std::string &script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  initFrameworkParams(m_jsContextFramework, params);
  bindGlobalContextFunctions(m_jsContextFramework);

  uint64_t start = microTime_new();
   const JSValue &value =
       JS_Eval(m_jsContextFramework, script.c_str(), script.length(),
               "jsFramework", JS_EVAL_TYPE_GLOBAL);

// const JSValue &value= JS_EvalBinary(m_jsContextFramework, weex_main_jsfm, weex_main_jsfm_size, JS_EVAL_TYPE_GLOBAL);

  LOGE("calTime initFramework %lld", (microTime_new() - start));


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

int WeexRuntimeQuickJS::exeJsService(const std::string &source) {
  auto ret = JS_Eval(m_jsContextFramework, source.c_str(), source.length(),
                     "exeJsService", JS_EVAL_TYPE_GLOBAL);

  if (JS_IsException(ret)) {
    const JSValue &jsValue = JS_GetException(m_jsContextFramework);

    const char *string = JS_ToCString(m_jsContextFramework, jsValue);

    LOGE("dyyLog exeJsService %s", string);
    return 0;
  } else {
    LOGE("dyyLog exeJsService finish");
    return 1;
  }
}

int WeexRuntimeQuickJS::exeCTimeCallback(const std::string &source) {
  return 0;
}

int WeexRuntimeQuickJS::exeJS(const std::string &instanceId,
                              const std::string &nameSpace,
                              const std::string &func,
                              std::vector<VALUE_WITH_TYPE *> &params) {
  std::string newFunc = func;
  JSContext *thisContext = nullptr;

  if (std::strcmp("callJS", func.c_str()) == 0) {
    thisContext = m_contextMap[instanceId.c_str()];
    if (thisContext == nullptr) {
      LOGE("dyyLog  js_exeJS is running use global ctx1 %s",instanceId.c_str());
      thisContext = m_jsContextFramework;
    } else {
      LOGE("dyyLog  js_exeJS is running use sandbox ctx2 %s",instanceId.c_str());
      newFunc = "__WEEX_CALL_JAVASCRIPT__";
    }
  } else {
    LOGE("dyyLog  js_exeJS is running use global ctx %s",instanceId.c_str());
    thisContext = m_jsContextFramework;
  }

  auto thisObject = JS_GetGlobalObject(thisContext);

  const int size = params.size();
  JSValue *a = new JSValue[size];

  for (size_t i = 0; i < size; i++) {
    VALUE_WITH_TYPE *paramsObject = params[i];
    switch (paramsObject->type) {
      case ParamsType::DOUBLE:a[i] = JS_NewInt64(thisContext, paramsObject->value.doubleValue);
        break;
      case ParamsType::STRING: {
        WeexString *ipcstr = paramsObject->value.string;
        auto string2String = weexString2String(ipcstr);
        LOGE("dyyLog ss2 %s", string2String.c_str());
        a[i] = JS_NewString(thisContext, string2String.c_str());

      }
        break;
      case ParamsType::JSONSTRING: {
        const WeexString *ipcstr = paramsObject->value.string;
        auto string2String = weexString2String(ipcstr);
        LOGE("dyyLog ss1 %s", string2String.c_str());
        auto jsvalue = JS_ParseJSON(thisContext, string2String.c_str(),
                                    string2String.length(), "t");

        a[i] = jsvalue;
      }
        break;
      case ParamsType::BYTEARRAY: {
        const WeexByteArray *array = paramsObject->value.byteArray;
        wson_parser w(array->content, array->length);
        auto string2String = w.toStringUTF8();
        LOGE("dyyLog ss %s", string2String.c_str());
        auto jsvalue = JS_ParseJSON(thisContext, string2String.c_str(),
                                    string2String.length(), "t");
        a[i] = jsvalue;

        //                msg.append(":");
        //                msg.append(JSONStringify(state, o, 0).utf8().data());
      }
        break; {
            LOGE("dyyLog sss undefined");
            default:a[i] = JS_UNDEFINED;
            break;
        }
    }
  }

  JS_Call(thisContext,
          JS_GetProperty(thisContext, thisObject,
                         JS_NewAtom(thisContext, newFunc.c_str())),
          thisObject, size, a);

  return 1;
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


  std::ofstream outfile;
  outfile.open("/data/local/tmp/weex/index_read_exec.js");
  outfile << script.c_str() << std::endl;
  outfile.close();

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
    args[1] = JS_ParseJSON(globalContext, opts.c_str(), opts.length(),"");
    args[2] = JS_ParseJSON(globalContext, initData.c_str(), initData.length(),"");

    JSValue ret = JS_Call(globalContext, createInstanceContextFunc,
                          JS_GetGlobalObject(globalContext), 3, args);

    if (JS_IsException(ret)) {
      const JSValue &exception = JS_GetException(globalContext);
      const char *string = JS_ToCString(globalContext, exception);
      LOGE("dyyLog js_createInstance Create Instance JS_IsException %s", string);
      return 0;
    }

    JSPropertyEnum *tab_atom;
    uint32_t tab_atom_count;
    if (JS_GetOwnPropertyNames(
        globalContext, &tab_atom, &tab_atom_count, ret,
        JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK)) {
      return 0;
    }

    for (size_t i = 0; i < tab_atom_count; i++) {
      auto atom = tab_atom[i].atom;
      auto propertyValue = JS_GetProperty(globalContext, ret, atom);
      const char *name = JS_AtomToCString(thisContext, atom);
      auto newAtom = JS_NewAtom(thisContext, name);
      auto thisObject = JS_GetGlobalObject(thisContext);
      int aProperty = JS_SetProperty(thisContext, thisObject, newAtom, propertyValue);

      LOGE("dyyLog  js_createInstance  propertyValue %s is Func %d is Object %d result is %d",
          name, JS_IsFunction(globalContext, propertyValue), JS_IsObject(propertyValue), aProperty);
      if (strcmp(name, "Vue") == 0) {
        LOGE("dyyLog  js_createInstance  set propertyValue Vue");
        JSValue val = JS_GetPrototype(thisContext, thisObject);
        if(val == JS_NULL || val == JS_UNDEFINED) {
          LOGE("dyyLog  js_createInstance  set propertyValue Vue undefined before");
        }
        JS_SetPrototype(thisContext, propertyValue,
                        val);
        if(val == JS_NULL || val == JS_UNDEFINED) {
          LOGE("dyyLog  js_createInstance  set propertyValue Vue undefined after");
        }
      }
    }
    LOGE("dyyLog  js_createInstance  pub str %s", instanceId.c_str());
    this->m_contextMap[instanceId.c_str()] = thisContext;
  }

  if (!extendsApi.empty()) {
    LOGE("dyyLog  js_createInstance  Create Instance extendsApi extendsApi");
    auto ret = JS_Eval(thisContext, extendsApi.c_str(), extendsApi.length(),
                       "extendsApi", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(ret)) {
      const JSValue &exception = JS_GetException(thisContext);
      const char *string = JS_ToCString(thisContext, exception);
      LOGE("dyyLog  js_createInstance  Create Instance extendsApi JS_IsException %s", string);
      return 0;
    }
  }
  LOGE("dyyLog  js_createInstance  create Instance start %d %d", script.length(), script.size());
  auto createInstanceRet = JS_Eval(thisContext, script.c_str(), script.length(),
                                   "createInstance", JS_EVAL_TYPE_GLOBAL);

  if (JS_IsException(createInstanceRet)) {
    JSValue name, stack;
    const JSValue &exception = JS_GetException(thisContext);
    const char *string = JS_ToCString(thisContext, exception);
    name = JS_GetPropertyStr(thisContext, exception, "name");
    const char *error_name = JS_ToCString(thisContext, name);
    stack = JS_GetPropertyStr(thisContext, exception, "stack");
    const char *stack_str = JS_ToCString(thisContext, stack);
    LOGE("dyyLog  js_createInstance Create Instance createInstanceRet JS_IsException %s %s %s",error_name, string, stack_str);
    return 0;
  }
  LOGE("dyyLog  js_createInstance create Instance Finish");
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
  JSValue i = JS_GetGlobalObject(ctx);
  JS_SetProperty(ctx, i, JS_NewAtom(ctx, "global"), i);
  JS_SetMaxStackSize(ctx, 1024*10000);
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
  return JS_UNDEFINED;
}

// add for wson
static void pushStringToWsonBuffer(wson_buffer *buffer, std::string str_utf_8) {
  std::u16string s = weex::base::to_utf16(const_cast<char *>(str_utf_8.c_str()),
                                          str_utf_8.length());
  size_t length = s.length();
  wson_push_type(buffer, WSON_STRING_TYPE);
  wson_push_uint(buffer, length * sizeof(uint16_t));
  wson_push_bytes(buffer, s.c_str(), s.length() * sizeof(uint16_t));
}

static void pushMapKeyToBuffer(wson_buffer *buffer, std::string str_utf_8) {
  std::u16string s = weex::base::to_utf16(const_cast<char *>(str_utf_8.c_str()),
                                          str_utf_8.length());
  size_t length = s.length();
  wson_push_uint(buffer, length * sizeof(uint16_t));
  wson_push_bytes(buffer, s.c_str(), s.length() * sizeof(uint16_t));
}

static void putValuesToWson(JSContext *ctx, JSValue value,
                            wson_buffer *buffer) {
  int tag = JS_VALUE_GET_TAG(value);

  switch (tag) {
    case JS_TAG_INT: {
      int num = -1;
      JS_ToInt32(ctx, &num, value);
      wson_push_type_int(buffer, num);
      break;
    }
    case JS_TAG_BOOL: {
      wson_push_type_boolean(buffer, JS_ToBool(ctx, value));
      break;
    }
    case JS_TAG_FLOAT64: {
      double numd;
      JS_ToFloat64(ctx, &numd, value);
      wson_push_double(buffer, numd);
      break;
    }
    case JS_TAG_UNDEFINED: {
      wson_push_type_null(buffer);
      break;
    }
    case JS_TAG_STRING: {
      const char *str = JS_ToCString(ctx, value);
      pushStringToWsonBuffer(buffer, str);
      break;
    }
    case JS_TAG_OBJECT: {
      if (JS_IsArray(ctx, value)) {
        JSValue length = JS_GetPropertyStr(ctx, value, "length");
        int len = JS_VALUE_GET_INT(length);
        JS_FreeValue(ctx, length);
        wson_push_type_array(buffer, len);
        for (size_t i = 0; i < len; i++) {
          JSValue item = JS_GetPropertyUint32(ctx, value, i);
          putValuesToWson(ctx, item, buffer);
        }
      } else {
        JSPropertyEnum *tab_atom;
        uint32_t tab_atom_count;
        JS_GetOwnPropertyNames(
            ctx, &tab_atom, &tab_atom_count, value,
            JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK | JS_GPN_ENUM_ONLY);
        wson_push_type_map(buffer, tab_atom_count);
        for (size_t i = 0; i < tab_atom_count; i++) {
          JSAtom atom = tab_atom[i].atom;
          JSValue item = JS_GetProperty(ctx, value, atom);
          JSValue key = JS_AtomToValue(ctx, atom);
 
          const char *str = JS_ToCString(ctx, key);
          pushMapKeyToBuffer(buffer, str);
          putValuesToWson(ctx, item, buffer);
        }
      }
      /* code */
      break;
    }
    default: {
      LOGE("dyyLog default");
      break;
    }
  }
}

static wson_buffer *toWsonBuffer(JSContext *ctx, JSValue value) {
  // const char *result = JS_ToCString(ctx, JS_ToString(ctx, value));
  wson_buffer *buffer = wson_buffer_new();
  putValuesToWson(ctx, value, buffer);
  wson_parser parser((char *) buffer->data);
  const std::string &string = parser.toStringUTF8();
  LOGE("dyyLog toWsonBuffer %s", string.empty() ? "put empty" : string.c_str());
  return buffer;
}

static std::string toJSON(JSContext *ctx, JSValue value) {
  wson_buffer *buffer = wson_buffer_new();
  putValuesToWson(ctx, value, buffer);
  wson_parser parser((char *) buffer->data);
  LOGE("dyyLog toJSON %s", parser.toStringUTF8().c_str());
  return parser.toStringUTF8();
}

static JSValue js_CallNative(JSContext *ctx, JSValueConst this_val, int argc,
                             JSValueConst *argv) {
  LOGE("dyyLog js_CallNative");
  const char *id = JS_ToCString(ctx, argv[0]);
  const char *task = toJSON(ctx, argv[1]).c_str();
  const char *callback = JS_ToCString(ctx, argv[2]);
  WeexEnv::getEnv()->scriptBridge()->core_side()->CallNative(
      id, task, callback);
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallNativeModule(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  const char *id = JS_ToCString(ctx, argv[0]);
  const char *module = JS_ToCString(ctx, argv[1]);
  const char *method = JS_ToCString(ctx, argv[2]);

  wson_buffer *arguments = toWsonBuffer(ctx, argv[3]);
  wson_buffer *options = toWsonBuffer(ctx, argv[4]);

  printWson("callNativeModule arguments", arguments);
  LOGE("dyyLog js_CallNativeModule %s %s", module, method);
  auto result =
      WeexEnv::getEnv()->scriptBridge()->core_side()->CallNativeModule(
          id, module, method, (char *) (arguments->data), arguments->length,
          (char *) (options->data), options->length);

  JSValue ret;
  switch (result->type) {
    case ParamsType::DOUBLE: {
      ret = JS_NewInt64(ctx, result->value.doubleValue);
    }
      break;
    case ParamsType::STRING: {
      const std::string &string = jString2String(result->value.string->content,
                                                 result->value.string->length);
      ret = JS_NewString(ctx, string.c_str());
    }

      break;
    case ParamsType::JSONSTRING: {
      const std::string &string = jString2String(result->value.string->content,
                                                 result->value.string->length);
      ret = JS_ParseJSON(ctx, string.c_str(), string.length(), "");
      free(result->value.string);
    }
      break;
    case ParamsType::BYTEARRAY: {
      wson_parser w(result->value.byteArray->content,
                    result->value.byteArray->length);
      auto string2String = w.toStringUTF8();
      LOGE("dyyLog ss %s", string2String.c_str());
      auto jsvalue =
          JS_ParseJSON(ctx, string2String.c_str(), string2String.length(), "t");
      free(result->value.byteArray);
      ret = jsvalue;
    }
      break;
    default: { ret = JS_UNDEFINED; }
      break;
  }
  LOGE("dyyLog js_CallNativeModule %s : %s : %s", id, module, method);
  return ret;
}

static JSValue js_CallNativeComponent(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallNativeComponent");
  wson_buffer *arguments = toWsonBuffer(ctx, argv[3]);
  wson_buffer *options = toWsonBuffer(ctx, argv[4]);
  WeexEnv::getEnv()->scriptBridge()->core_side()->CallNativeComponent(
      JS_ToCString(ctx, argv[0]), JS_ToCString(ctx, argv[1]),
      JS_ToCString(ctx, argv[2]), (char *) arguments->data, arguments->length,
      (char *) options->data, options->length);

  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallAddElement(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {

  wson_buffer *pBuffer = toWsonBuffer(ctx, argv[2]);
  const char *id = JS_ToCString(ctx, argv[0]);
  const char *ref = JS_ToCString(ctx, argv[1]);
  LOGE("dyyLog js_CallAddElement %s %s", id, ref);
  WeexEnv::getEnv()->scriptBridge()->core_side()->AddElement(
      id, ref,
      (char *) pBuffer->data, pBuffer->length, JS_ToCString(ctx, argv[3]));

  return JS_NewInt32(ctx, 0);
}

static JSValue js_SetTimeoutNative(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_SetTimeoutNative");
  return JS_UNDEFINED;
}

static JSValue js_NativeLog(JSContext *ctx, JSValueConst this_val, int argc,
                            JSValueConst *argv) {
  LOGE("dyyLog jsBindingMethod js_NativeLog");

  std::string result = "dyyLog";

  for (int i = 0; i < argc; ++i) {
    const char *string = JS_ToCString(ctx, argv[i]);
    result += string;
  }

  WeexEnv::getEnv()->scriptBridge()->core_side()->NativeLog(result.c_str());

  return JS_TRUE;
}

static JSValue js_NotifyTrimMemory(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NotifyTrimMemory");
  return JS_UNDEFINED;
}

static JSValue js_MarkupState(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv) {
  return LOGE("dyyLog js_MarkupState");
  JS_UNDEFINED;
}

static JSValue js_Atob(JSContext *ctx, JSValueConst this_val, int argc,
                       JSValueConst *argv) {
  return LOGE("dyyLog js_Atob");
  JS_UNDEFINED;
}

static JSValue js_Btoa(JSContext *ctx, JSValueConst this_val, int argc,
                       JSValueConst *argv) {
  return LOGE("dyyLog js_Btoa");
  JS_UNDEFINED;
}

static JSValue js_CallCreateBody(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallCreateBody %d", argc);
  wson_buffer *pBuffer = toWsonBuffer(ctx, argv[1]);
  WeexEnv::getEnv()->scriptBridge()->core_side()->CreateBody(
      JS_ToCString(ctx, argv[0]), (char *) pBuffer->data, pBuffer->length);

  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallUpdateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallUpdateFinish");
  wson_buffer *taskChar = toWsonBuffer(ctx, argv[1]);
  wson_buffer *callBackChar = toWsonBuffer(ctx, argv[2]);
  int a = WeexEnv::getEnv()->scriptBridge()->core_side()->UpdateFinish(
      JS_ToCString(ctx, argv[0]), (char *) taskChar->data, callBackChar->length,
      (char *) callBackChar->data, callBackChar->length);
  return JS_NewInt32(ctx, a);
}

static JSValue js_CallCreateFinish(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallCreateFinish");
  WeexEnv::getEnv()->scriptBridge()->core_side()->CreateFinish(
      JS_ToCString(ctx, argv[0]));
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallRefreshFinish(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallRefreshFinish");

  int i = WeexEnv::getEnv()->scriptBridge()->core_side()->RefreshFinish(
      JS_ToCString(ctx, argv[0]), JS_ToCString(ctx, argv[1]),
      JS_ToCString(ctx, argv[2]));

  return JS_NewInt32(ctx, i);
}

static JSValue js_CallUpdateAttrs(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallUpdateAttrs");

  wson_buffer *pBuffer = toWsonBuffer(ctx, argv[2]);

  WeexEnv::getEnv()->scriptBridge()->core_side()->UpdateAttrs(
      JS_ToCString(ctx, argv[0]), JS_ToCString(ctx, argv[1]),
      (char *) (pBuffer->data), pBuffer->length);
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallUpdateStyle(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallUpdateStyle");
  wson_buffer *pBuffer = toWsonBuffer(ctx, argv[2]);
  WeexEnv::getEnv()->scriptBridge()->core_side()->UpdateStyle(
      JS_ToCString(ctx, argv[0]), JS_ToCString(ctx, argv[1]),
      (char *) (pBuffer->data), pBuffer->length);
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallRemoveElement(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallRemoveElement");

  WeexEnv::getEnv()->scriptBridge()->core_side()->RemoveElement(
      JS_ToCString(ctx, argv[0]), JS_ToCString(ctx, argv[1]));
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallMoveElement(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallMoveElement");
  WeexEnv::getEnv()->scriptBridge()->core_side()->MoveElement(
      JS_ToCString(ctx, argv[0]), JS_ToCString(ctx, argv[1]),
      JS_ToCString(ctx, argv[2]), atoi(JS_ToCString(ctx, argv[3])));
  return JS_NewInt32(ctx, 0);
}

static JSValue js_CallAddEvent(JSContext *ctx, JSValueConst this_val, int argc,
                               JSValueConst *argv) {
  LOGE("dyyLog js_CallAddEvent");
  WeexEnv::getEnv()->scriptBridge()->core_side()->AddEvent(
      JS_ToCString(ctx, argv[0]), JS_ToCString(ctx, argv[1]),
      JS_ToCString(ctx, argv[2]));
  return JS_NewInt32(ctx, 0);;
}

static JSValue js_CallRemoveEvent(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  LOGE("dyyLog js_CallRemoveEvent");
  WeexEnv::getEnv()->scriptBridge()->core_side()->RemoveEvent(
      JS_ToCString(ctx, argv[0]), JS_ToCString(ctx, argv[1]),
      JS_ToCString(ctx, argv[2]));
  return JS_NewInt32(ctx, 0);;
}

static JSValue js_GCanvasLinkNative(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  LOGE("dyyLog js_GCanvasLinkNative");
  return JS_UNDEFINED;
}

static JSValue js_SetIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
  LOGE("dyyLog js_SetIntervalWeex");

  int i = WeexEnv::getEnv()->scriptBridge()->core_side()->SetInterval(
      JS_ToCString(ctx, argv[0]), JS_ToCString(ctx, argv[1]),
      JS_ToCString(ctx, argv[2]));
  return JS_NewInt32(ctx, i);
}

static JSValue js_ClearIntervalWeex(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  LOGE("dyyLog js_ClearIntervalWeex");
  return JS_UNDEFINED;
}

static JSValue js_T3DLinkNative(JSContext *ctx, JSValueConst this_val, int argc,
                                JSValueConst *argv) {
  LOGE("dyyLog js_T3DLinkNative");
  return JS_UNDEFINED;
}

static JSValue js_NativeLogContext(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NativeLogContext");
  return JS_UNDEFINED;
}

static JSValue js_DisPatchMeaage(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {
  LOGE("dyyLog js_DisPatchMeaage");
  return JS_UNDEFINED;
}

static JSValue js_DispatchMessageSync(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  LOGE("dyyLog js_DispatchMessageSync");
  return JS_UNDEFINED;
}

static JSValue js_PostMessage(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv) {
  LOGE("dyyLog js_PostMessage");
  return JS_UNDEFINED;
}

static JSValue js_NativeSetTimeout(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NativeSetTimeout");
  return JS_UNDEFINED;
}

static JSValue js_NativeSetInterval(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NativeSetInterval");
  return JS_UNDEFINED;
}

static JSValue js_NativeClearTimeout(JSContext *ctx, JSValueConst this_val,
                                     int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NativeClearTimeout");
  return JS_UNDEFINED;
}

static JSValue js_NativeClearInterval(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  LOGE("dyyLog js_NativeClearInterval");
  return JS_UNDEFINED;
}

// For data render
static JSValue js_UpdateComponentData(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  LOGE("dyyLog js_UpdateComponentData");
  return JS_UNDEFINED;
}
