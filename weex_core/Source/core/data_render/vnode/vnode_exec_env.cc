//
// Created by Xu Jiacheng on 2018/7/20.
//

#include "core/data_render/vnode/vnode_exec_env.h"
#include "core/data_render/object.h"
#include "core/data_render/table_factory.h"
#include "core/data_render/table.h"
#include <sstream>

namespace weex {
namespace core {
namespace data_render {
static Value Log(ExecState* exec_state) {
  size_t length = exec_state->GetArgumentCount();
  for (int i = 0; i < length; ++i) {
    Value* a = exec_state->GetArgument(i);
    switch (a->type) {
      case Value::Type::NUMBER:
        std::cout << a->n << "\n";
        break;
      case Value::Type::INT:
        std::cout << a->i << "\n";
        break;
      case Value::Type::STRING:
        std::cout << a->str->c_str() << "\n";
        break;
      default:
        break;
    }
  }
  return Value();
}

static Value CreateElement(ExecState* exec_state) {//createElement("tagName","id");
//  const std::string& page_name = exec_state->page_id();
  VNode* node = new VNode(
      exec_state->GetArgument(1)->str->c_str(),
      exec_state->GetArgument(0)->str->c_str()
  );
  if (exec_state->context()->root() == nullptr) {
    //set root
    exec_state->context()->SetVNodeRoot(node);
  }
  exec_state->context()->InsertNode(node);
  return Value();
}

static Value AppendChild(ExecState* exec_state) {//appendChild("tag","id",""parent_id");todo
  VNode* parent = exec_state->context()->FindNode(exec_state->GetArgument(2)->str->c_str());
  VNode* child = exec_state->context()->FindNode(exec_state->GetArgument(1)->str->c_str());
  if (parent == nullptr || child == nullptr) {
    return Value();
  }
  parent->AddChild(child);

  return Value();
}

static Value SetAttr(ExecState* exec_state) {//setAttr("id","key","value");
  VNode* node = exec_state->context()->FindNode(exec_state->GetArgument(0)->str->c_str());
  if (node == nullptr) {
    return Value();
  }

  char* key = exec_state->GetArgument(1)->str->c_str();
  Value* p_value = exec_state->GetArgument(2);
  if (p_value->type == Value::STRING) {
    node->SetAttribute(key, p_value->str->c_str());
  } else if (p_value->type == Value::INT) {//todo use uniform type conversion.
    std::stringstream ss;
    ss << p_value->i;
    std::string str = ss.str();
    node->SetAttribute(key, str);
  }


  return Value();
}

static Value SetClassList(ExecState* exec_state) {
  VNode* node = exec_state->context()->FindNode(exec_state->GetArgument(0)->str->c_str());
  char* key = exec_state->GetArgument(1)->str->c_str();

  if (node == nullptr) {
    return Value();
  }

  json11::Json& json = exec_state->context()->raw_json();
  const json11::Json& styles = json["styles"];
  const json11::Json& style = styles[key];
  if (style.is_null()) {
    return Value();
  }
  const json11::Json::object& items = style.object_items();
  for (auto it = items.begin(); it != items.end(); it++) {
    node->SetStyle(it->first, it->second.string_value());
  }
  return Value();
}

void RegisterCFunc(ExecState* state, const std::string& name, CFunction function) {
  Value func;
  func.type = Value::Type::CFUNC;
  func.cf = reinterpret_cast<void*>(function);
  state->global()->Add(name, func);
}


void VNodeExecEnv::InitCFuncEnv(ExecState* state) {
  //log
  RegisterCFunc(state, "log", Log);
  RegisterCFunc(state, "createElement", CreateElement);
  RegisterCFunc(state, "appendChild", AppendChild);
  RegisterCFunc(state, "setAttr", SetAttr);
  RegisterCFunc(state, "setClassList", SetClassList);
}

Value ParseJson2Value(ExecState* state, const json11::Json& json) {
  if (json.is_null()) {
    return Value();
  } else if (json.is_bool()) {
    return Value(json.bool_value());
  } else if (json.is_number()) {
    //todo check which is int or double.
    return Value(json.number_value());
  } else if (json.is_string()) {
    String* p_str = state->string_table()->StringFromUTF8(json.string_value());
    return Value(p_str);
  } else if (json.is_array()) {
    Value* value = TableFactory::Instance()->CreateTable();
    const json11::Json::array& data_objects = json.array_items();
    int64_t index = 0;
    for (auto it = data_objects.begin(); it != data_objects.end(); it++, index++) {
      //will be free by table
      SetTabValue(TableValue(value),
                  new Value(index),
                  new Value(ParseJson2Value(state, *it))
      );
    }
    return Value(*value);
  } else if (json.is_object()) {
    Value* value = TableFactory::Instance()->CreateTable();
    const json11::Json::object& data_objects = json.object_items();
    for (auto it = data_objects.begin(); it != data_objects.end(); it++) {
      //will be free by table
      SetTabValue(TableValue(value),
                  new Value(state->string_table()->StringFromUTF8(it->first)),
                  new Value(ParseJson2Value(state, it->second))
      );
    }
    return Value(*value);
  } else {
    return Value();
  }
};

void VNodeExecEnv::InitGlobalValue(ExecState* state) {
  const json11::Json& json = state->context()->raw_json();
  Global* global = state->global();
  const json11::Json& data = json["data"];
  if (data.is_null()) {
    return;
  }
  const json11::Json::object& data_objects = data.object_items();
  for (auto it = data_objects.begin(); it != data_objects.end(); it++) {
    const Value& value = ParseJson2Value(state, it->second);
    global->Add(it->first, value);
  }

}
}
}
}