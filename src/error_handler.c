#include "stm32_module/error_handler.h"

// glibc include
#include <stdint.h>

// stm32 include
#include "stm32_module/stm32_hal.h"

// freertos include
#include "FreeRTOS.h"
#include "task.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* virtual function redirection ----------------------------------------------*/
inline ModuleRet ErrorHandler_start(ErrorHandler* const self) {
  return self->super_.vptr_->start((Task*)self);
}

/* virtual function definition -----------------------------------------------*/
// from Task base class
ModuleRet __ErrorHandler_start(Task* const _self) {
  module_assert(IS_NOT_NULL(_self));

  ErrorHandler* const self = (ErrorHandler*)_self;
  return Task_create_freertos_task(
      (Task*)self, "error_handler", ERROR_HANDLER_TASK_PRIORITY,
      self->task_stack_, ERROR_HANDLER_TASK_STACK_SIZE);
}

/* constructor ---------------------------------------------------------------*/
void ErrorHandler_ctor(ErrorHandler* self) {
  module_assert(IS_NOT_NULL(self));

  // construct inherited class and redirect virtual function
  Task_ctor((Task*)self, ErrorHandler_task_code);
  static struct TaskVtbl vtbl = {
      .start = __ErrorHandler_start,
  };
  self->super_.vptr_ = &vtbl;

  // initialize member variable
  self->error_code_ = 0;
}

/* member function -----------------------------------------------------------*/
ModuleRet ErrorHandler_add_error_callback(
    ErrorHandler* const self, struct error_callback_cb* const error_callback_cb,
    ErrorCallback_t callback, void* const arg, const uint32_t error_code) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(error_callback_cb));
  module_assert(IS_NOT_NULL(callback));
  module_assert(IS_ERROR_CODE(error_code));

  if (self->super_.state_ != TaskReset) {
    return ModuleError;
  }

  error_callback_cb->callback = callback;
  error_callback_cb->arg = arg;

  taskENTER_CRITICAL();
  List_push_back(&self->error_callback_list_,
                 &error_callback_cb->error_callback_list_cb, error_callback_cb);
  taskEXIT_CRITICAL();

  return ModuleOK;
}
ModuleRet ErrorHandler_write_error(ErrorHandler* const self,
                                   const uint32_t error_code,
                                   const uint32_t option) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_ERROR_CODE(error_code));
  module_assert(IS_ERROR_OPTION(option));

  if (self->super_.state_ != TaskRunning) {
    return ModuleError;
  }

  if (xPortIsInsideInterrupt()) {
    BaseType_t require_contex_switch = pdFALSE;
    xTaskNotifyFromISR((TaskHandle_t)&self->super_.task_control_block_,
                       error_code | option, eSetBits, &require_contex_switch);
    portYIELD_FROM_ISR(require_contex_switch);
  } else {
    xTaskNotify((TaskHandle_t)&self->super_.task_control_block_,
                error_code | option, eSetBits);
  }

  return ModuleOK;
}

ModuleRet ErrorHandler_get_error(const ErrorHandler* const self,
                                 uint32_t* code) {
  if (self->super_.state_ != TaskRunning) {
    return ModuleError;
  }

  *code = self->error_code_;
  return ModuleOK;
}

void ErrorHandler_task_code(void* const _self) {
  ErrorHandler* const self = (ErrorHandler*)_self;

  while (1) {
    uint32_t error_code;
    xTaskNotifyWait(0, ERROR_CODE_ALL | ERROR_SET, &error_code, portMAX_DELAY);
    if (error_code & ERROR_SET) {
      self->error_code_ |= error_code & ERROR_CODE_ALL;
    } else {
      self->error_code_ &= ~(error_code & ERROR_CODE_ALL);
    }

    ListIter error_callback_iter;
    ListIter_ctor(&error_callback_iter, &self->error_callback_list_);

    taskENTER_CRITICAL();
    struct error_callback_cb* error_callback_cb =
        (struct error_callback_cb*)ListIter_next(&error_callback_iter);
    while (error_callback_cb != NULL) {
      if (error_callback_cb->error_code & error_code) {
        error_callback_cb->callback(error_callback_cb->arg, error_code);
      }

      error_callback_cb =
          (struct error_callback_cb*)ListIter_next(&error_callback_iter);
    }
    taskEXIT_CRITICAL();
  }
}
