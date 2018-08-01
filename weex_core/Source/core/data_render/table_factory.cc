//
// Created by chad on 2018/7/19.
//

#include "core/data_render/table_factory.h"
#include "core/data_render/table.h"
#include "core/data_render/vm_mem.h"

namespace weex {
namespace core {
namespace data_render {

Value TableFactory::CreateTable() {
  Table *t = NewTable();
  Value v;
  SetTValue(&v, reinterpret_cast<GCObject *>(t));
  tablePool.emplace_back(t);
  return v;
}

TableFactory::~TableFactory() {

  for (auto it = tablePool.begin(); it != tablePool.end(); it++) {
//    FreeValue(*it);
    delete *it;
  }
  tablePool.clear();
}

}
}
}