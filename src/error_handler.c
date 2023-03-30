#include "stm32_module/error_handler.h"

// glibc include
#include <stdint.h>

// stm32 include
#include "cmsis_os.h"
#if defined(STM32G431xx)
#include "stm32g4xx_hal.h"
#elif defined(STM32H723xx)
#include "stm32h7xx_hal.h"
#endif

// freertos include
#include "FreeRTOS.h"
#include "task.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* virtual function redirection ----------------------------------------------*/
inline ModuleRet ErrorHandler_start(ErrorHandler* const self) {
  return self->super_.vptr->start((Task*)self);
}

/* virtual function definition -----------------------------------------------*/
// from Task base class
ModuleRet __ErrorHandler_start(Task* const _self) {
  module_assert(IS_NOT_NULL(_self));

  ErrorHandler* const self = (ErrorHandler*)_self;
  if (self->super_.state_ == TASK_RUNNING) {
    return MODULE_BUSY;
  }

  Task_create_freertos_task((Task*)self, "error_handler", osPriorityRealtime,
                            self->task_stack_,
                            sizeof(self->task_stack_) / sizeof(StackType_t));
  return MODULE_OK;
}

/* constructor ---------------------------------------------------------------*/
void ErrorHandler_ctor(ErrorHandler* self) {
  module_assert(IS_NOT_NULL(self));

  // construct inherited class and redirect virtual function
  Task_ctor((Task*)self, ErrorHandler_task_code);
  static struct TaskVtbl vtbl = {
      .start = __ErrorHandler_start,
  };
  self->super_.vptr = &vtbl;

  // initialize member variable
  self->error_code_ = 0;
}

/* member function -----------------------------------------------------------*/
ModuleRet ErrorHandler_write_error(ErrorHandler* const self,
                                   const uint32_t error_code,
                                   const uint32_t option) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_ERROR_CODE(error_code));
  module_assert(IS_ERROR_OPTION(option));

  if (self->super_.state_ != TASK_RUNNING) {
    return MODULE_ERROR;
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

  return MODULE_OK;
}

uint32_t ErrorHandler_get_error(const ErrorHandler* const self) {
  return self->error_code_;
}

void ErrorHandler_task_code(void* const _self) {
  ErrorHandler* const self = (ErrorHandler*)_self;
  TickType_t last_wake = xTaskGetTickCount();

  while (1) {
    uint32_t error_code;
    xTaskNotifyWait(0, ERROR_CODE_ALL | ERROR_OPTION_SET, &error_code,
                    portMAX_DELAY);
    if (error_code & ERROR_OPTION_SET) {
      self->error_code_ |= error_code & ERROR_CODE_ALL;
    } else {
      self->error_code_ &= ~(error_code & ERROR_CODE_ALL);
    }

    vTaskDelayUntil(&last_wake, 1);
  }
}
