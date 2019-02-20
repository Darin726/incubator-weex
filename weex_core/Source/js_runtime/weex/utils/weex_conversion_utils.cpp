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
// Created by 陈佩翰 on 2019/2/12.
//

#include <wson/wson_parser.h>
#include "weex_jsc_utils.h"
#include "weex_conversion_utils.h"
#include "js_runtime/utils/log_utils.h"
#include "wson_for_runtime.h"
#include "android/jsengine/object/weex_env.h"

namespace weex {
    namespace jsengine {

//        json11::Json WeexConversionUtils::convertElementToJSon(const Element *element) {
//            json11::Json::object styles;
//            json11::Json::object attributes;
//            json11::Json::array events;
//
//            for (auto attrItem:element->attributes) {
//                attributes.insert({attrItem.first, attrItem.second});
//            }
//
//            for (auto styleItem:element->style) {
//                if (styleItem.second->type == style_type_string) {
//                    styles.insert({styleItem.first, styleItem.second->data.string_value});
//                } else if (styleItem.second->type == style_type_string) {
//                    styles.insert({styleItem.first, styleItem.second->data.int_value});
//                } else if (styleItem.second->type == style_type_double) {
//                    styles.insert({styleItem.first, styleItem.second->data.double_value});
//                } else {
//                    LOGE("[WeexConversionUtils] unsupport style type %d for key:%s", styleItem.second->type,
//                         styleItem.first.c_str());
//                }
//            }
//
//            for (auto eventLisener: element->mEventListenList) {
//                events.push_back(json11::Json(eventLisener->type));
//            }
//
//            json11::Json::array children;
//
//            for (auto childNode:element->childNodes) {
//                if (childNode->nodeType != childNode->ELEMENT_NODE) {
//                    LOG_TEST("[DomUtils] convert child element exception, child is not a element ! node ref:%s",
//                             childNode->ref.c_str());
//                    continue;
//                }
//                auto childElement = static_cast<Element *>(childNode);
//                children.push_back(WeexConversionUtils::convertElementToJSon(childElement));
//            }
//
//            json11::Json elementJson = json11::Json::object{
//                    {"ref",      element->ref.c_str()},
//                    {"type",     element->localName.c_str()},
//                    {"attr",     attributes},
//                    {"style",    styles},
//                    {"event",    events},
//                    {"children", children}
//            };
//
//            return elementJson;
//        }

//        bool WeexConversionUtils::convertStyleToJSon(const std::string &name, StyleVale *value, std::string &result) {
//            json11::Json data;
//            if (value->type == style_type_string) {
//                data = json11::Json::object{
//                        {name, value->data.string_value}
//                };
//
//            } else if (value->type == style_type_int) {
//                data = json11::Json::object{
//                        {name, value->data.int_value}
//                };
//            } else if (value->type == style_type_double) {
//                data = json11::Json::object{
//                        {name, value->data.double_value}
//                };
//            } else {
//                return false;
//            }
//            data.dump(result);
//            return true;
//        }

        bool
        WeexConversionUtils::convertKVToJSon(const std::string &name, const ::std::string &value, std::string &result) {
            json11::Json data = json11::Json::object{
                    {name, value}
            };
            data.dump(result);
            return true;
        }

        json11::Json WeexConversionUtils::RunTimeValuesOfObjectToJson(unicorn::RuntimeValues *vars) {
            if (nullptr == vars || vars->IsNull() || vars->IsUndefined()) {
                LOG_CONVERSION("arg is not object, json return null json str");
                return json11::Json(nullptr);
            }

            if (vars->IsInt()) {
                int value;
                vars->GetAsInteger(&value);
                return json11::Json(value);
            } else if (vars->IsDouble()) {
                double value;
                vars->GetAsDouble(&value);
                return json11::Json(value);
            } else if (vars->IsBool()) {
                bool value;
                vars->GetAsBoolean(&value);
                return json11::Json(value);
            } else if (vars->IsString()) {
                std::string value;
                vars->GetAsString(&value);
                return json11::Json(value);
            } else if (vars->IsMap()) {
                auto map = vars->GetAsMap()->GetMap();
                json11::Json::object mapJson;
                for (auto item : map) {
                    mapJson.insert({item.first, WeexConversionUtils::RunTimeValuesOfObjectToJson(item.second)});
                }
                return mapJson;
            } else if (vars->IsArray()) {
                json11::Json::array arrayJson;
                auto array = vars->GetAsArray()->GetArray();
                for (auto item: array) {
                    arrayJson.push_back(WeexConversionUtils::RunTimeValuesOfObjectToJson(item));
                }
                return arrayJson;
            } else {
                LOGE("unknow parser json type:%d", vars->GetType());
                return json11::Json(nullptr);
            }
        }

        unicorn::ScopeValues
        WeexConversionUtils::WeexValueToRuntimeValue(unicorn::EngineContext *context, VALUE_WITH_TYPE *paramsObject) {
            // LOGE("WeexRuntime: WeexValueToRuntimeValue type is %d", paramsObject->type);
            switch (paramsObject->type) {
                case ParamsType::DOUBLE: {
                    LOG_CONVERSION("WeexValueToRuntimeValue double :%d", paramsObject->value.doubleValue);
                    return unicorn::RuntimeValues::MakeDouble(paramsObject->value.doubleValue);
                }
                case ParamsType::STRING: {
                    WeexString *ipcstr = paramsObject->value.string;
                    const String &string2String = weexString2String(ipcstr);
                   // LOG_CONVERSION("WeexValueToRuntimeValue string :%s", string2String.utf8().data());
                    return unicorn::RuntimeValues::MakeString(std::string(string2String.utf8().data()));
                }
                case ParamsType::JSONSTRING: {
                    const WeexString *ipcJsonStr = paramsObject->value.string;
                    const String &string = weexString2String(ipcJsonStr);
                    LOG_CONVERSION("WeexValueToRuntimeValue JSONSTRING length:%d",string.length());
                    auto res = unicorn::RuntimeValues::MakeObjectFromJsonStr(std::string(string.utf8().data()));
                    LOG_CONVERSION("WeexValueToRuntimeValue JSONSTRING succeed");
                    return res;
                }
                case ParamsType::BYTEARRAY: {
                    //       LOG_TEST("WeexValueToRuntimeValue BYTEARRAY");
                    //tips: close wson case
                    //  const WeexByteArray *array = paramsObject->value.byteArray;
                    //  JSValue o = wson::toJSValue(state, (void *) array->content, array->length);

//                obj->append(o);
                    //  obj->push_back(unicorn::RuntimeValues::MakeObjectFromWson(static_cast<void *>(array->content),array->length));
                    //  LOG_TEST("WeexValueToRuntimeValue wson bbyte array");
                    LOG_CONVERSION("WeexValueToRuntimeValue wson");
                    const WeexByteArray *array = paramsObject->value.byteArray;
                    return wson::toRunTimeValueFromWson(context, (void *) array->content, array->length);
                }
                default:
                    //obj->append(jsUndefined());
                    //  break;
                    LOGE("WeexValueToRuntimeValue unkonw value type :%d",paramsObject->type);
                    return unicorn::RuntimeValues::MakeUndefined();
            }
        }

        void WeexConversionUtils::ConvertRunTimeVaueToWson(unicorn::RuntimeValues *value, Args &args) {
            wson_buffer *buffer = wson::runTimeValueToWson(value);
            args.setWson(buffer);
        }

        void
        WeexConversionUtils::GetStringFromArgsDefaultUndefined(const std::vector<unicorn::ScopeValues> &vars, int index,
                                                               std::string &result) {

            if (index >= vars.size()) {
                result.assign("undefined");
                return;
            }
            vars[index]->GetAsString(&result);
        }


        void
        WeexConversionUtils::GetStringFromArgsDefaultEmpty(const std::vector<unicorn::ScopeValues> &vars, int index,
                                                           std::string &result) {
            if (index >= vars.size()) {
                result.assign("");
                return;
            }
            if (vars[index]->IsUndefined() || vars[index]->IsNull()) {
                result.assign("undefined");
                return;
            }
            if (vars[index]->IsInt()) {
                int val = 0;
                vars[index]->GetAsInteger(&val);
                result.assign(std::to_string(val));
                return;;
            }
            vars[index]->GetAsString(&result);
        }

        bool WeexConversionUtils::GetJsonStrFromArgs(const std::vector<unicorn::ScopeValues> &vars, int index,
                                                     std::string &result) {
            if (index >= vars.size() || vars[index].get() == nullptr || vars[index]->IsNull() ||
                vars[index]->IsUndefined()) {
                return false;
            }

            WeexConversionUtils::RunTimeValuesOfObjectToJson(vars[index].get()).dump(result);

            return true;
        }

        void WeexConversionUtils::GetWsonFromArgs(const std::vector<unicorn::ScopeValues> &vars, int index,
                                                  Args &args) {
            if (index >= vars.size()) {
                args.setWson((wson_buffer *) nullptr);
                return;
            }
            ConvertRunTimeVaueToWson(vars[index].get(), args);
        }

        void
        WeexConversionUtils::ConvertRunTimeValueToWeexJSResult(unicorn::ScopeValues &value, WeexJSResult *jsResult) {
            if (!value->IsArray() || nullptr == jsResult) {
                LOGE("!value->IsArray() ");
                return;
            }
            bool isAllNull = true;
            const unicorn::Array *array = value->GetAsArray();
            if (nullptr == array) {
                LOGE("nullptr == array");
                return;
            }
            for (size_t i = 0; i < array->Size(); i++) {
                auto item = array->atIndex(i);
                if (nullptr != item && !item->IsUndefined() && !item->IsNull()) {
                    isAllNull = false;
                    break;
                }
            }
            if (isAllNull) {
                LOGE("isAllNull ");
                return;
            }
            char *buf = nullptr;
            if (WeexEnv::getEnv()->useWson()) {
                wson_buffer *buffer = wson::runTimeValueToWson(value.get());
                char *data = (char *) buffer->data;
                jsResult->length = buffer->position;
                buf = new char[jsResult->length + 1];
                memcpy(buf, data, jsResult->length);
                wson_parser parser((char *) buffer->data);
                LOGW("[exeJSWithResult] result wson :%s", parser.toStringUTF8().c_str());

                wson_buffer_free(buffer);
            } else {
                std::string json_str;
                WeexConversionUtils::RunTimeValuesOfObjectToJson(value.get()).dump(json_str);
                jsResult->length = json_str.length();
                buf = new char[jsResult->length + 1];
                memcpy(buf, json_str.c_str(), jsResult->length);
            }
            buf[jsResult->length] = '\0';
            jsResult->data.reset(buf);
        }
    }
}