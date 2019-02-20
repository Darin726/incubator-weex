//
// Created by Xu Jiacheng on 2019-01-15.
//

#include "rax_err_queue_holder.h"

RAX_NAME_SPACE_BEGIN

qking_value_t RaxErrQueueHolder::err_queue_pop() {
  qking_value_t err = qking_acquire_value(err_queue_.front()->get());
  err_queue_.pop();
  return err;
}

RAX_NAME_SPACE_END