#cmake_minimum_required(VERSION 3.3)
#if (POLICY CMP0042)
#    cmake_policy(SET CMP0042 NEW)
#endif ()

cmake_minimum_required(VERSION 3.4.1)

if (DEFINED ENV{WEEX_CORE_SOURCE_DIR})
    message('WEEX_CORE_SOURCE_DIR has set')
else ()
    message('rewrite WEEX_CORE_SOURCE_DIR')
    set(WEEX_CORE_SOURCE_DIR ../..)
endif ()


if (DEFINED ENV{ANDROID_PLATFORM})
    message('ANDROID_PLATFORM has set')
else ()
    message('rewrite ANDROID_PLATFORM')
    set(ANDROID_PLATFORM " ")
endif ()


set(PORT JSCOnly)
add_definitions(-DPRINT_LOG_CACHEFILE=0)
add_definitions(-DOS_ANDROID)

# log switch for debug in file "js_runtime/utils/log_utils.h"
#add_definitions(-DLOG_JS_RUNTIME_DEBUG=1)
#add_definitions(-DLOG_WEEX_RUNTIME_DEBUG=1)
#add_definitions(-DLOG_CONVERSION_DEBUG=0)
#add_definitions(-DLOG_WEEX_BINDING_DEBUG=1)
add_definitions(-DLOG_JS_DEBUG=0)


add_definitions(-DUSE_SYSTEM_MALLOC=1)
if (${ANDROID_PLATFORM} STREQUAL "android-21")
else ()
    add_definitions(-D_POSIX_THREAD_KEYS_MAX=128)
    add_definitions(-DPTHREAD_KEYS_MAX=_POSIX_THREAD_KEYS_MAX)
endif ()

if ("${ANDROID_ABI}" STREQUAL "armeabi")
    add_definitions(-DWTF_CPU_ARM)
    set(WTF_CPU_ARM 1)

    if ("${ANDROID_TOOLCHAIN}" STREQUAL clang)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a")
    endif ()
elseif ("${ANDROID_ABI}" STREQUAL "armeabi-v7a")
    add_definitions(-DWTF_CPU_ARM)
    set(WTF_CPU_ARM 1)
else ()
    add_definitions(-DWTF_CPU_X86)
    set(WTF_CPU_X86_64 1)
endif ()

set(USE_SYSTEM_MALLOC 1)
set(WEEXJSSERVER_NAME weexjss)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${WEEX_CORE_SOURCE_DIR})
include_directories(${WEEX_CORE_SOURCE_DIR}/include)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore)
#include_directories(${WEEX_CORE_SOURCE_DIR}/android/jsengine/dependence/WTF/)
#include_directories(${WEEX_CORE_SOURCE_DIR}/android/jsengine/dependence/icu)

#set(JS_RUNTIME_API_SRCS
#        ${WEEX_CORE_SOURCE_DIR}/third_party/js_api/runtime_vm.cc
#        ${WEEX_CORE_SOURCE_DIR}/third_party/js_api/runtime_context_android.cc
#        ${WEEX_CORE_SOURCE_DIR}/third_party/js_api/runtime_values.cc
#        ${WEEX_CORE_SOURCE_DIR}/third_party/js_api/binding_macro.h
#        ${WEEX_CORE_SOURCE_DIR}/third_party/js_api/jsc/engine_context_jsc.cc
#        ${WEEX_CORE_SOURCE_DIR}/third_party/js_api/jsc/jsc_utils.cc
#        ${WEEX_CORE_SOURCE_DIR}/third_party/js_api/jsc/runtime_object_jsc.cc
#        ${WEEX_CORE_SOURCE_DIR}/third_party/js_api/jsc/runtime_values_jsc.cc
#        ${WEEX_CORE_SOURCE_DIR}/third_party/js_api/jsc/vm_jsc.cc
#        ${WEEX_CORE_SOURCE_DIR}/third_party/js_api/jsc/binding_macro_jsc.h
#        )


set(JS_RUNTIME_API_SRCS
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/runtime_vm.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/runtime_context_android.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/runtime_values.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/binding_macro.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/engine_context_jsc.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/jsc_utils.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/runtime_object_jsc.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/runtime_values_jsc.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/vm_jsc.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/binding_macro_jsc.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/js_runtime_conversion.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/js_runtime_conversion.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/utils/log_utils.h
        )
set(WEEX_BINDING
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_instance_binding.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_instance_binding.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_global_binding.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_global_binding.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/app_context_binding.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/app_context_binding.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_console_binding.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_console_binding.h

        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/utils/wson_for_runtime.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/utils/wson_for_runtime.h

        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_binding_utils.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_binding_utils.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/utils/weex_conversion_utils.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/utils/weex_conversion_utils.h

        )

set(JSON_SRC
        ${WEEX_CORE_SOURCE_DIR}/third_party/json11/json11.hpp
        ${WEEX_CORE_SOURCE_DIR}/third_party/json11/json11.cc
        )
set(WEEX_RUNTIME_V2
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_runtime_v2.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_runtime_v2.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_object_holder_v2.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_object_holder_v2.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_global_object_v2.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_global_object_v2.h
        )


set(${WEEXJSSERVER_NAME}_SOURCES
        weex_ipc_client.h
        weex_ipc_client.cpp
        weex_ipc_server.h
        weex_ipc_server.cpp
        weex_jsc_utils.h
        weex_jsc_utils.cpp
        weex_runtime_quickjs.cpp
        weex_runtime_quickjs.h


        quickjs/cutils.c
        quickjs/libregexp-opcode.h
        quickjs/libunicode-table.h
        quickjs/list.h
        quickjs/quickjs-libc.c
        quickjs/quickjs.c
        quickjs/repl.c
        quickjs/unicode_gen_def.h
        quickjs/bjson.c
        quickjs/cutils.h
        quickjs/libbf.c
        quickjs/libregexp.c
        quickjs/libunicode.c
        quickjs/qjscalc.c
        quickjs/quickjs-libc.h
        quickjs/quickjs.h
        quickjs/libbf.h
        quickjs/libregexp.h
        quickjs/libunicode.h
        quickjs/quickjs-atom.h
        quickjs/quickjs-opcode.h
        quickjs/repl-bn.c
        quickjs/unicode_gen.c

        bridge/script/script_bridge_in_multi_so.cpp
        bridge/script/script_bridge_in_multi_process.cpp
        bridge/script/core_side_in_multi_so.cpp
        bridge/script/core_side_in_multi_process.cpp
        bridge/script/script_side_in_simple.cpp
        bridge/script/script_side_in_queue.cpp

        object/args/exe_js_args.cpp
        object/args/init_framework_args.cpp
        object/args.h
        object/args.cpp
        object/weex_env.h
        object/weex_env.cpp

        task/impl/init_framework_task.cpp
        task/impl/create_app_context_task.cpp
        task/impl/create_instance_task.cpp
        task/impl/call_js_on_app_context_task.cpp
        task/impl/destory_app_context_task.cpp
        task/impl/destory_instance_task.cpp
        task/impl/exe_js_on_app_with_result.cpp
        task/impl/update_global_config_task.cpp
        task/impl/ctime_callback_task.cpp
        task/impl/exe_js_services_task.cpp
        task/impl/exe_js_on_instance_task.cpp
        task/impl/exe_js_task.cpp
        task/impl/take_heap_snapshot.cpp
        task/impl/native_timer_task.cpp
        task/weex_task.cpp
        task/weex_task_queue.cpp
        task/timer_task.cpp
        task/timer_queue.cpp
        task/back_to_weex_core_queue.cpp

        #  ${JS_RUNTIME_API_SRCS}
        ${JS_RUNTIME_API_SRCS}
        ${DOM_API_SRCS}
        ${WEEX_BINDING}
        ${JSON_SRC}
        ${WEEX_RUNTIME_V2}
        )
link_directories(${LOCAL_LIBRARIES_DIR})
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" CACHE STRING "toolchain_exelinkflags" FORCE)
SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -shared")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a -fno-omit-frame-pointer")# -fsanitize=address -g3")
set(WEEXJSSERVERSTUB_NAME libweexjsb.so)
add_executable(${WEEXJSSERVERSTUB_NAME}
        weex_js_server_main_stub.cpp ../../js_runtime/utils/base64.hpp)

set_target_properties(${WEEXJSSERVERSTUB_NAME}
        PROPERTIES
        LINK_FLAGS
        "-pie -fPIE"
        )

target_link_libraries(${WEEXJSSERVERSTUB_NAME}
        ${WEEXJSSERVER_NAME}
        )

if (APPLE)
    set_target_properties(${WEEXJSSERVERSTUB_NAME}
            PROPERTIES LINK_FLAGS
            "-Wl,--entry=_start"
            )
endif ()


set(${WEEXJSSERVER_NAME}_LIBRARY_TYPE SHARED)

message("${LOCAL_LIBRARIES_DIR}")
set(${WEEXJSSERVER_NAME}_LIBRARIES
        weexbase
        weexipc
        wson
        c++_shared
        jsc
        log
        z
        -lc
        -ldl
        -pthread
        )

add_library(${WEEXJSSERVER_NAME} SHARED
        ${${WEEXJSSERVER_NAME}_SOURCES})


target_link_libraries(${WEEXJSSERVER_NAME}
        ${${WEEXJSSERVER_NAME}_LIBRARIES})
set_target_properties(${WEEXJSSERVER_NAME} PROPERTIES NO_SONAME TRUE)

set_target_properties(${WEEXJSSERVER_NAME}
        PROPERTIES
        LINK_FLAGS
        "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}//weexjsserver_version_script.txt -Wl,--no-undefined -Wl,-soname,libweexjss${SERVERSUFFIX}.so"
        )

#add_custom_command(
#    TARGET ${WEEXJSSERVER_NAME}
#    POST_BUILD
#    COMMAND cp $<TARGET_FILE:${WEEXJSSERVER_NAME}> ${CMAKE_SOURCE_DIR}/libweexjss${SERVERSUFFIX}.so
#    VERBATIM
#)

#add_custom_command(
#    TARGET ${WEEXJSSERVERSTUB_NAME}
#    POST_BUILD
#    COMMAND cp $<TARGET_FILE:${WEEXJSSERVERSTUB_NAME}> ${CMAKE_SOURCE_DIR}/libweexjsb${SERVERSUFFIX}.so
#    VERBATIM
#)
