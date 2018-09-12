// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     weex_core_debug/Source/WeexCore/platform/android/jniprebuild/jni_generator.py
// For
//     com/taobao/weex/bridge/JsFunctions

#ifndef WEEX_PROJECT_WXJSFUNCTIONS_JNI_H
#define WEEX_PROJECT_WXJSFUNCTIONS_JNI_H

#include <jni.h>
#include "../../base/jni/android_jni.h"

namespace {
    const char kWXJsFunctionClassPath[] = "com/taobao/weex/bridge/WXJsFunctions";
// Leaking this jclass as we cannot use LazyInstance from some threads.
    jclass g_WXJsFunction_clazz = NULL;
#define WXJsFunction_clazz(env) g_WXJsFunction_clazz

}  // namespace

static void resetWXBridge(JNIEnv *env, jobject object, jobject bridge, jstring className);

static void jsHandleSetJSVersion(JNIEnv *env, jobject object, jstring jsVersion);

static void jsHandleReportException(JNIEnv *env, jobject object, jstring instanceId, jstring func,
                                    jstring exceptionjstring);

static void jsHandleCallNative(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                               jstring callback);

static void
jsHandleCallNativeModule(JNIEnv *env, jobject object, jstring instanceId, jstring module,
                         jstring method, jbyteArray
                         arguments, jbyteArray options,jboolean from);

static void
jsHandleCallNativeComponent(JNIEnv *env, jobject object, jstring instanceId, jstring componentRef,
                            jstring method,
                            jbyteArray arguments, jbyteArray options, jboolean from);

static void
jsHandleCallAddElement(JNIEnv *env, jobject object, jstring instanceId, jstring ref, jbyteArray dom,
                       jstring index);

static void jsHandleSetTimeout(JNIEnv *env, jobject object, jstring callbackId, jstring time);

static void jsHandleCallNativeLog(JNIEnv *env, jobject object, jbyteArray str_array);

static void jsFunctionCallCreateBody(JNIEnv *env, jobject object, jstring pageId, jbyteArray domStr, jboolean from);

static void
jsFunctionCallUpdateFinish(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                           jstring callback);

static void jsFunctionCallCreateFinish(JNIEnv *env, jobject object, jstring pageId);

static void
jsFunctionCallRefreshFinish(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                            jstring callback);

static void
jsFunctionCallUpdateAttrs(JNIEnv *env, jobject object, jstring pageId, jstring ref, jbyteArray data, jboolean from);

static void
jsFunctionCallUpdateStyle(JNIEnv *env, jobject object, jstring pageId, jstring ref, jbyteArray data, jboolean from);

static void jsFunctionCallRemoveElement(JNIEnv *env, jobject object, jstring pageId, jstring ref);

static void
jsFunctionCallMoveElement(JNIEnv *env, jobject object, jstring pageId, jstring ref,
                          jstring parentRef, jstring index_str);

static void
jsFunctionCallAddEvent(JNIEnv *env, jobject object, jstring pageId, jstring ref, jstring event);

static void
jsFunctionCallRemoveEvent(JNIEnv *env, jobject object, jstring pageId, jstring ref, jstring event);

static void jsHandleSetInterval(JNIEnv *env, jobject object, jstring instanceId, jstring callbackId,
                                jstring time);

static void
jsHandleClearInterval(JNIEnv *env, jobject object, jstring instanceId, jstring callbackId);

static void jsHandleCallGCanvasLinkNative(JNIEnv *env, jobject object, jstring contextId, int type,
                                          jstring val);


static const JNINativeMethod kMethodsWXJsFunctions[] = {
        {"resetWXBridge",
                "(Ljava/lang/Object;Ljava/lang/String;)V",
                reinterpret_cast<void *>(resetWXBridge)},
        {"jsHandleSetJSVersion",
                "(Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleSetJSVersion)},
        {"jsHandleReportException",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleReportException)},
        {"jsHandleCallNative",
                "(Ljava/lang/String;[BLjava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleCallNative)},
        {"jsHandleCallNativeModule",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[BZ)V",
                reinterpret_cast<void *>(jsHandleCallNativeModule)},
        {"jsHandleCallNativeComponent",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B[BZ)V",
                reinterpret_cast<void *>(jsHandleCallNativeComponent)},
        {"jsHandleCallAddElement",
                "(Ljava/lang/String;Ljava/lang/String;[BLjava/lang/String;Z)V",
                reinterpret_cast<void *>(jsHandleCallAddElement)},
        {"jsHandleSetTimeout",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleSetTimeout)},
        {"jsHandleCallNativeLog",
                "([B)V",
                reinterpret_cast<void *>(jsHandleCallNativeLog)},
        {"jsFunctionCallCreateBody",
                "(Ljava/lang/String;[BZ)V",
                reinterpret_cast<void *>(jsFunctionCallCreateBody)},
        {"jsFunctionCallUpdateFinish",
                "(Ljava/lang/String;[BLjava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallUpdateFinish)},
        {"jsFunctionCallCreateFinish",
                "(Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallCreateFinish)},
        {"jsFunctionCallRefreshFinish",
                "(Ljava/lang/String;[BLjava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallRefreshFinish)},
        {"jsFunctionCallUpdateAttrs",
                "(Ljava/lang/String;Ljava/lang/String;[BZ)V",
                reinterpret_cast<void *>(jsFunctionCallUpdateAttrs)},
        {"jsFunctionCallUpdateStyleNative",
                "(Ljava/lang/String;Ljava/lang/String;[BZ)V",
                reinterpret_cast<void *>(jsFunctionCallUpdateStyle)},
        {"jsFunctionCallRemoveElement",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallRemoveElement)},
        {"jsFunctionCallMoveElement",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallMoveElement)},
        {"jsFunctionCallAddEvent",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallAddEvent)},
        {"jsFunctionCallRemoveEvent",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsFunctionCallRemoveEvent)},
        {"jsHandleSetInterval",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleSetInterval)},
        {"jsHandleClearInterval",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleClearInterval)},
        {"jsHandleCallGCanvasLinkNative",
                "(Ljava/lang/String;ILjava/lang/String;)V",
                reinterpret_cast<void *>(jsHandleCallGCanvasLinkNative)}
};


static bool RegisterNativesImpl(JNIEnv *env) {

    g_WXJsFunction_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
            base::android::GetClass(env, kWXJsFunctionClassPath).Get()));

    const int kMethodsWXJsFunctionsSize =
            sizeof(kMethodsWXJsFunctions) / sizeof(kMethodsWXJsFunctions[0]);

    return 0 <= env->RegisterNatives(WXJsFunction_clazz(env),
                                     kMethodsWXJsFunctions,
                                     kMethodsWXJsFunctionsSize);

}

#endif //WEEX_PROJECT_WXJSFUNCTIONS_JNI_H
