//
// Created by Xu Jiacheng on 2019-01-30.
//

#include "base/qking_common_logger.h"
#include "rax_props_holder.h"
#include <regex>

RAX_NAME_SPACE_BEGIN

#ifdef QKING_PLATFORM_IOS
static const char *IOS_POST_FIX = "\t\n\t\r";
#endif

void RaxPropsHolder::SetNativeProps(
    qking_value_t props,
    std::function<void(const std::string& key, const std::string& value)>
        style_hook,
    std::function<void(const std::string& key, const std::string& value)>
        attr_hook,
    std::function<void(const std::string& event, qking_value_t value)>
        event_hook) {
  // style
  struct Passer {
    std::function<void(const std::string& key, const std::string& value)>*
        style_hook_;
    std::function<void(const std::string& key, const std::string& value)>*
        attr_hook_;
    std::function<void(const std::string& event, qking_value_t value)>*
        event_hook_;
  } tmp{&style_hook, &attr_hook, &event_hook};

  qking_value_t style =
      qking_get_property_by_name_lit(props, QKING_LIT_MAGIC_STRING_EX_STYLE);
  if (!qking_value_is_error(style)) {
    // travel style;
    qking_foreach_object_property_of(
        style,
        [](const qking_value_t property_name,
           const qking_value_t property_value, void* user_data_p) -> bool {
          if (!qking_value_is_string(property_name)) {
            return true;
          }
          std::string style_key = string_from_qking_string_value(property_name);
          std::string style_str;
          if (qking_value_is_null_or_undefined(property_value) ||
              qking_value_is_nan(property_value)) {
            style_str = "";
          } else if (!qking_value_is_string(property_value)) {
            if (qking_value_is_object(property_value)) {
              style_str = string_from_qking_json_stringify(property_value);
#ifdef QKING_PLATFORM_IOS
              // IOS need this postfix to parse str back to object
              style_str.append(IOS_POST_FIX);
#endif
            } else {
              style_str = string_from_qking_to_string(property_value);
            }
          } else {
            style_str = string_from_qking_string_value(property_value);
          }
          (*(((Passer*)user_data_p)->style_hook_))(style_key, style_str);
          return true;
        },
        &tmp, false, true, false);
  }
  qking_release_value(style);

  // attr & event
  if (qking_value_is_object(props)) {
    // travel style;
    qking_foreach_object_property_of(
        props,
        [](const qking_value_t property_name,
           const qking_value_t property_value, void* user_data_p) -> bool {
          static std::regex EVENT_REG("^on[A-Z]");
          Passer* passer = ((Passer*)user_data_p);

          if (!qking_value_is_string(property_name)) {
            return true;
          }
          std::string props_key = string_from_qking_string_value(property_name);
          if (props_key == "children" || props_key == "style") {
            // skip children and style
            return true;
          }
          if (std::regex_search(props_key, EVENT_REG)) {
            // a event
            std::string event_key = props_key.substr(2, props_key.size());
            std::transform(event_key.begin(), event_key.end(),
                           event_key.begin(), ::tolower);
            (*(passer->event_hook_))(event_key, property_value);
          } else {
            std::string props_value;
            if (qking_value_is_null_or_undefined(property_value) ||
                qking_value_is_nan(property_value)) {
              props_value = "";
            } else if (!qking_value_is_string(property_value)) {
              if (qking_value_is_object(property_value)) {
                props_value = string_from_qking_json_stringify(property_value);
#ifdef QKING_PLATFORM_IOS
                // IOS need this postfix to parse str back to object
                props_value.append(IOS_POST_FIX);
#endif
              } else {
                props_value = string_from_qking_to_string(property_value);
              }
            } else {
              props_value = string_from_qking_string_value(property_value);
            }
            (*(passer->attr_hook_))(props_key, props_value);
          }
          return true;
        },
        &tmp, false, true, false);
  }
}

RAX_NAME_SPACE_END
