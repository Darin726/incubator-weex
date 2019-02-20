//
// Created by Xu Jiacheng on 2019-01-18.
//

#include "rax_native_transform.h"
#include "base/qking_common_logger.h"
#include "rax_element_factory.h"
#include "rax_text_element.h"

RAX_NAME_SPACE_BEGIN

static qking_value_t p_transform_children(RaxNativeElement *root,
                                          qking_value_t children);
static qking_value_t p_transform_child(RaxNativeElement *root,
                                       qking_value_t child);
static qking_value_t p_transform_string(qking_value_t ele);
static void p_add_typography_elements(qking_value_t &style, qking_value_t &type,
                                      std::string &str_type);

void rax_transform_props_to_attr(RaxNativeElement *element,
                                 qking_magic_str_t prop) {
  qking_value_t props = element->get_js_props();
  qking_value_t style =
      get_optional_property(props, QKING_LIT_MAGIC_STRING_EX_STYLE,
                            "rax_transform_props_to_attr", "style");

  qking_release_value(get_optional_property(
      props, prop, "rax_transform_props_to_attr", "$prop",
      [=, &style](qking_value_t result) {
        if (qking_value_is_undefined(result)) {
          return;
        }
        if (qking_value_is_null_or_undefined(style)) {
          style = qking_create_object();
          set_optional_property(props, QKING_LIT_MAGIC_STRING_EX_STYLE, style,
                                "rax_transform_props_to_attr", "style");
        }
        set_optional_property(style, prop, result,
                              "rax_transform_props_to_attr", "$prop");
        qking_value_t to_delete = qking_create_string_lit(prop);
        qking_delete_property(props, to_delete);
        qking_release_value(to_delete);
      }));

  qking_release_value(style);
}

void rax_collect_single_string_child(RaxNativeElement *element) {
  qking_value_t child = element->get_js_children();
  qking_value_t old_value = get_optional_property(
      element->get_js_props(), QKING_LIT_MAGIC_STRING_EX_VALUE,
      "rax_create_element_filter_props", "props");
  if (!qking_value_is_undefined(old_value)) {
    // has old.
    qking_release_value(old_value);
    return;
  }
  qking_release_value(old_value);

  if (qking_value_is_string(child)) {
    // only one. props.value = props.children
    set_optional_property(element->get_js_props(),
                          QKING_LIT_MAGIC_STRING_EX_VALUE, child,
                          "rax_transform_span", "props");
    // props.children = null
    set_optional_property(element->get_js_props(),
                          QKING_LIT_MAGIC_STRING_EX_CHILDREN,
                          qking_create_null(), "rax_transform_span", "props");
  } /* else { skip; } */

  element->set_js_children(qking_create_null());
}

static bool rax_transform_span(RaxNativeElement *element) {
  element->actrual_comp_type() = "text";

  rax_collect_single_string_child(element);

  return false;
}

static bool rax_transform_p(RaxNativeElement *element) {
  element->actrual_comp_type() = "richtext";

  qking_value_t props = element->get_js_props();

  // default style
  {
    qking_value_t style = get_optional_property(
        props, QKING_LIT_MAGIC_STRING_EX_STYLE, "rax_transform_p", "style");
    if (qking_value_is_null_or_undefined(style)) {
      style = qking_create_object();
      set_optional_property(props, QKING_LIT_MAGIC_STRING_EX_STYLE, style,
                            "rax_transform_props_to_attr", "style");
    }

    qking_value_t base_font_size = qking_create_number(28);
    set_optional_property_if_not_present(style, "fontSize", base_font_size,
                                         "rax_transform_p", "fontSize");
    set_optional_property_if_not_present(style, "marginTop", base_font_size,
                                         "rax_transform_p", "marginTop");
    set_optional_property_if_not_present(style, "marginBottom", base_font_size,
                                         "rax_transform_p", "marginBottom");
    qking_release_value(base_font_size);
    qking_release_value(style);
  }

  // props.value = transformChildren(children)
  qking_value_t to_set_value =
      p_transform_children(element, element->get_js_children());
  qking_release_value(qking_set_property_by_name_lit(
      props, QKING_LIT_MAGIC_STRING_EX_VALUE, to_set_value));
  qking_release_value(to_set_value);

  // props.children = null
  element->set_js_children(qking_create_null());
  return false;
}

static bool rax_transform_p_warn(RaxNativeElement *element) {
  RAX_LOGE("You should not update a richtext element aka: <p/>");
  return true;
}

static bool rax_transform_img(RaxNativeElement *element) {
  element->actrual_comp_type() = "image";

  rax_transform_props_to_attr(element, QKING_LIT_MAGIC_STRING_EX_WIDTH);
  rax_transform_props_to_attr(element, QKING_LIT_MAGIC_STRING_EX_HEIGHT);

  return false;
}

static bool rax_transform_button(RaxNativeElement *element) {
  element->actrual_comp_type() = "div";

  return false;
}

static bool rax_transform_textarea(RaxNativeElement *element) {
  rax_collect_single_string_child(element);
  return false;
}

static bool rax_transform_video(RaxNativeElement *element) {
  rax_transform_props_to_attr(element, QKING_LIT_MAGIC_STRING_EX_WIDTH);
  rax_transform_props_to_attr(element, QKING_LIT_MAGIC_STRING_EX_HEIGHT);
  return false;
}

static void rax_setup_heading_style(qking_value_t style, double font_multiplier,
                                    double margin_multiplier) {
  qking_value_t base_font_size = qking_create_number(28 * font_multiplier);
  qking_value_t base_margin_top =
      qking_create_number(28 * font_multiplier * margin_multiplier);
  qking_value_t base_margin_bottom =
      qking_create_number(28 * font_multiplier * margin_multiplier);
  qking_value_t font_weight = qking_create_string_c("bold");

  set_optional_property_if_not_present(style, "fontSize", base_font_size,
                                       "rax_transform_heading", "fontSize");
  set_optional_property_if_not_present(style, "marginTop", base_margin_top,
                                       "rax_transform_heading", "marginTop");
  set_optional_property_if_not_present(style, "marginBottom",
                                       base_margin_bottom,
                                       "rax_transform_heading", "marginBottom");
  set_optional_property_if_not_present(style, "fontWeight", font_weight,
                                       "rax_transform_heading", "fontWeight");

  qking_release_value(base_font_size);
  qking_release_value(base_margin_top);
  qking_release_value(base_margin_bottom);
  qking_release_value(font_weight);
}

static bool rax_transform_heading(RaxNativeElement *element) {
  element->actrual_comp_type() = "text";

  qking_value_t props = element->get_js_props();
  // default style
  {
    qking_value_t style =
        get_optional_property(props, QKING_LIT_MAGIC_STRING_EX_STYLE,
                              "rax_transform_heading", "style");
    if (qking_value_is_null_or_undefined(style)) {
      style = qking_create_object();
      set_optional_property(props, QKING_LIT_MAGIC_STRING_EX_STYLE, style,
                            "rax_transform_headingrops_to_attr", "style");
    }

    if (element->comp_type() == "h1") {
      rax_setup_heading_style(style, 2, 0.67);
    } else if (element->comp_type() == "h2") {
      rax_setup_heading_style(style, 1.5, 0.83);
    } else if (element->comp_type() == "h3") {
      rax_setup_heading_style(style, 1.17, 1);
    } else if (element->comp_type() == "h4") {
      rax_setup_heading_style(style, 1, 1.33);
    } else if (element->comp_type() == "h5") {
      rax_setup_heading_style(style, 0.83, 1.67);
    } else if (element->comp_type() == "h6") {
      rax_setup_heading_style(style, 0.67, 2.33);
    } else {
      rax_setup_heading_style(style, 0.67, 2.33);
    }

    qking_release_value(style);
  }

  rax_collect_single_string_child(element);
  return false;
}

const static std::map<std::string, std::function<bool(RaxNativeElement *)>>
    tag_to_trans{
        {"span", rax_transform_span},         {"p", rax_transform_p},
        {"img", rax_transform_img},           {"button", rax_transform_button},
        {"textarea", rax_transform_textarea}, {"video", rax_transform_video},
        {"h1", rax_transform_heading},        {"h2", rax_transform_heading},
        {"h3", rax_transform_heading},        {"h3", rax_transform_heading},
        {"h4", rax_transform_heading},        {"h5", rax_transform_heading},
        {"h6", rax_transform_heading},
    };

const static std::map<std::string, std::function<bool(RaxNativeElement *)>>
    tag_to_trans_update{
        {"span", rax_transform_span},         {"p", rax_transform_p_warn},
        {"img", rax_transform_img},           {"button", rax_transform_button},
        {"textarea", rax_transform_textarea}, {"video", rax_transform_video},
        {"h1", rax_transform_heading},        {"h2", rax_transform_heading},
        {"h3", rax_transform_heading},        {"h3", rax_transform_heading},
        {"h4", rax_transform_heading},        {"h5", rax_transform_heading},
        {"h6", rax_transform_heading},
    };

bool rax_native_transform(RaxNativeElement *element) {
  const auto &it = tag_to_trans.find(element->comp_type());
  if (it != tag_to_trans.end()) {
    return it->second(element);
  }
  return false;
}

bool rax_native_update_transform(RaxNativeElement *element) {
  const auto &it = tag_to_trans_update.find(element->comp_type());
  if (it != tag_to_trans_update.end()) {
    return it->second(element);
  }
  return false;
}

void rax_create_element_filter_props(RaxElement *element) {
  if (element->type() != RaxElementType::kNative) {
    return;
  }

  RaxNativeElement *native = dynamic_cast<RaxNativeElement *>(element);
  if (native->comp_type() != "text") {
    return;
  }

  qking_value_t child = native->get_js_children();
  if (!qking_value_is_array(child)) {
    if (!qking_value_is_null_or_undefined(child)) {
      // only one.
      RaxElement *child_ele = native->get_factory()->GetRawTypeElement(child);
      if (child_ele == nullptr) {
        throw rax_common_err(
            "CreateElement rax_create_element_filter_props: Not a valid child "
            "type");
      }
      if (child_ele->type() == RaxElementType::kText) {
        qking_value_t value = qking_create_string_c(
            dynamic_cast<RaxTextElement *>(child_ele)->text().c_str());
        set_optional_property(native->get_js_props(),
                              QKING_LIT_MAGIC_STRING_EX_VALUE, value,
                              "rax_create_element_filter_props", "props");
        qking_release_value(value);
      } /* else { skip; } */
    }
    // else { no children }
  } else {
    struct Passer {
      RaxNativeElement *parent_this;
      std::string all_string;
      bool has_text_child;
      bool error;
    } tmp{native, "", false, false};
    qking_foreach_object_property_of(
        child,
        [](const qking_value_t property_name,
           const qking_value_t property_value, void *user_data_p) -> bool {
          Passer *passer = (Passer *)user_data_p;
          RaxNativeElement *parent_this = passer->parent_this;
          RaxElement *child_ele =
              parent_this->get_factory()->GetRawTypeElement(property_value);
          if (child_ele == nullptr) {
            passer->error = true;
            return false;
          }
          if (child_ele->type() == RaxElementType::kText) {
            passer->all_string +=
                dynamic_cast<RaxTextElement *>(child_ele)->text();
            passer->has_text_child = true;
          } /* else { skip; } */
          return true;
        },
        &tmp, true, true, false);

    if (tmp.error) {
      throw rax_common_err(
          "CreateElement rax_create_element_filter_props: Not a valid child "
          "type");
    }
    if (tmp.has_text_child) {
      qking_value_t value = qking_create_string_c(tmp.all_string.c_str());
      set_optional_property(native->get_js_props(),
                            QKING_LIT_MAGIC_STRING_EX_VALUE, value,
                            "rax_create_element_filter_props", "props");
      qking_release_value(value);
    }
  }

  native->set_js_children(qking_create_null());
}

static qking_value_t p_transform_children(RaxNativeElement *root,
                                          qking_value_t children) {
  qking_value_t element_array = qking_create_array(0);

  std::function<void(qking_value_t)> process_child = [=](qking_value_t child) {
    if (qking_value_is_string(child)) {
      qking_value_t to_add = p_transform_string(child);
      qking_release_value(qking_set_property_by_index(
          element_array, qking_get_array_length(element_array), to_add));
      qking_release_value(to_add);
    } else if (qking_value_is_object(child)) {
      qking_value_t to_add = p_transform_child(root, child);
      if (!qking_value_is_undefined(to_add)) {
        qking_release_value(qking_set_property_by_index(
            element_array, qking_get_array_length(element_array), to_add));
      }
      qking_release_value(to_add);
    }
  };

  if (qking_value_is_array(children)) {
    struct Passer {
      std::function<void(qking_value_t)> func;
    } tmp{process_child};

    qking_foreach_object_property_of(
        children,
        [](const qking_value_t property_name,
           const qking_value_t property_value, void *user_data_p) -> bool {
          Passer *passer = (Passer *)user_data_p;
          passer->func(property_value);
          return true;
        },
        &tmp, true, true, false);
  } else {
    process_child(children);
  }

  return element_array;
}

static qking_value_t p_transform_child(RaxNativeElement *root,
                                       qking_value_t child) {
  // child is a ele of children.
  RaxElement *element = root->get_factory()->GetRawTypeElement(child);
  if (!element || element->type() != RaxElementType::kNative) {
    RAX_LOGW("Richtext err: adding none-basic type to richtext");
    return qking_create_undefined();
  }

  RaxNativeElement *native_ele = dynamic_cast<RaxNativeElement *>(element);

  qking_value_t type = qking_acquire_value(element->get_js_type());
  qking_value_t props = element->get_js_props();
  qking_value_t style =
      qking_get_property_by_name_lit(props, QKING_LIT_MAGIC_STRING_EX_STYLE);
  RAX_ASSERT(!qking_value_is_error(style));
  qking_value_t children = element->get_js_children();

  // alias img tag
  std::string str_type = native_ele->comp_type();
  if (str_type == "img") {
    type = qking_create_string_c("image");
    str_type = "image";
  }

  // transform to span
  p_add_typography_elements(style, type, str_type);

  // prepare element of 'type', 'style', 'attr'.
  const qking_value_t element_to_ret = qking_create_object();
  qking_release_value(qking_set_property_by_name(element_to_ret, "type", type));
  qking_release_value(qking_set_property_by_name_lit(
      element_to_ret, QKING_LIT_MAGIC_STRING_EX_STYLE, style));
  qking_value_t final_attr_value;
  if (qking_value_is_object(props)) {
    qking_release_value(
        qking_set_property_by_name(element_to_ret, "attr", props));
    final_attr_value = qking_acquire_value(props);
  } else {
    qking_value_t to_set_props = qking_create_object();
    qking_release_value(
        qking_set_property_by_name(element_to_ret, "attr", to_set_props));
    final_attr_value = to_set_props;
  }

  // props.style = null
  // props.children = null, in origin weex, null will be stringify to "", so we
  // use "" instead.
  qking_value_t empty_string = qking_create_string_c("");
  qking_release_value(qking_set_property_by_name_lit(
      props, QKING_LIT_MAGIC_STRING_EX_STYLE, empty_string));
  qking_release_value(qking_set_property_by_name_lit(
      props, QKING_LIT_MAGIC_STRING_EX_CHILDREN, empty_string));
  qking_release_value(empty_string);

  // prepare children.
  if (!qking_value_is_null_or_undefined(children)) {
    if (str_type == "span" && qking_value_is_string(children)) {
      qking_release_value(qking_set_property_by_name_lit(
          final_attr_value, QKING_LIT_MAGIC_STRING_EX_VALUE, children));
    } else {
      qking_value_t new_children = p_transform_children(root, children);
      qking_release_value(qking_set_property_by_name_lit(
          element_to_ret, QKING_LIT_MAGIC_STRING_EX_CHILDREN, new_children));
      qking_release_value(new_children);
    }
  }

  qking_release_value(final_attr_value);
  qking_release_value(style);
  qking_release_value(type);
  return element_to_ret;
}

static qking_value_t p_transform_string(qking_value_t ele) {
  qking_value_t result = qking_create_object();

  qking_value_t type_value = qking_create_string_c("span");
  qking_release_value(qking_set_property_by_name(result, "type", type_value));
  qking_release_value(type_value);

  qking_value_t attr_value = qking_create_object();
  qking_release_value(qking_set_property_by_name_lit(
      attr_value, QKING_LIT_MAGIC_STRING_EX_VALUE, ele));

  qking_release_value(qking_set_property_by_name(result, "attr", attr_value));
  qking_release_value(attr_value);

  return result;
}

static void p_add_typography_elements(qking_value_t &style, qking_value_t &type,
                                      std::string &str_type) {
  if (str_type == "u") {
    qking_value_t to_set = qking_create_string_c("underline");
    set_optional_property_if_not_present(style, "textDecoration", to_set,
                                         "p_add_typography_elements",
                                         "textDecoration");
    qking_release_value(to_set);
  } else if (str_type == "s") {
    qking_value_t to_set = qking_create_string_c("line-through");
    set_optional_property_if_not_present(style, "textDecoration", to_set,
                                         "p_add_typography_elements",
                                         "textDecoration");
    qking_release_value(to_set);
  } else if (str_type == "i") {
    qking_value_t to_set = qking_create_string_c("italic");
    set_optional_property_if_not_present(
        style, "fontStyle", to_set, "p_add_typography_elements", "fontStyle");
    qking_release_value(to_set);
  } else if (str_type == "b") {
    qking_value_t to_set = qking_create_string_c("bold");
    set_optional_property_if_not_present(
        style, "fontWeight", to_set, "p_add_typography_elements", "fontWeight");
    qking_release_value(to_set);
  } else if (str_type == "del") {
    qking_value_t to_set = qking_create_string_c("line-through");
    set_optional_property_if_not_present(style, "textDecoration", to_set,
                                         "p_add_typography_elements",
                                         "textDecoration");
    qking_release_value(to_set);
  } else if (str_type == "em") {
    qking_value_t to_set = qking_create_string_c("italic");
    set_optional_property_if_not_present(
        style, "fontStyle", to_set, "p_add_typography_elements", "fontStyle");
    qking_release_value(to_set);
  } else if (str_type == "strong") {
    qking_value_t to_set = qking_create_string_c("bold");
    set_optional_property_if_not_present(
        style, "fontWeight", to_set, "p_add_typography_elements", "fontWeight");
    qking_release_value(to_set);
  } else if (str_type == "big") {
    qking_value_t to_set = qking_create_number(28 * 1.2);
    set_optional_property_if_not_present(
        style, "fontSize", to_set, "p_add_typography_elements", "fontSize");
    qking_release_value(to_set);
  } else if (str_type == "small") {
    qking_value_t to_set = qking_create_number(28 * 0.8);
    set_optional_property_if_not_present(
        style, "fontSize", to_set, "p_add_typography_elements", "fontSize");
    qking_release_value(to_set);
  } else {
    // not a typography element
    return;
  }

  qking_release_value(type);
  type = qking_create_string_c("span");
  str_type = "span";
}

RAX_NAME_SPACE_END
