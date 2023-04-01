#include "stm32_module/module_common.h"

// glibc include
#include <stddef.h>
#include <stdint.h>

// freertos include
#include "FreeRTOS.h"
#include "task.h"

/* function ------------------------------------------------------------------*/
void __module_assert_fail(const char *assertion, const char *file,
                          unsigned int line, const char *function) {
  (void)assertion;
  (void)file;
  (void)line;
  (void)function;
}

/* virtual function redirection ----------------------------------------------*/
inline ModuleRet Task_start(Task *const self) {
  return self->vptr->start(self);
}

/* virtual function definition -----------------------------------------------*/
// pure virtual function for Task base class
ModuleRet __Task_start(Task *const self) {
  (void)self;

  module_assert(0);
}

/* constructor ---------------------------------------------------------------*/
void Task_ctor(Task *const self, void (*const task_code)(void *)) {
  module_assert(IS_NOT_NULL(self));

  // assign base virtual function
  static struct TaskVtbl vtbl = {
      .start = __Task_start,
  };
  self->vptr = &vtbl;

  // initialize member variable
  self->state_ = TASK_RESET;
  self->task_handle_ = NULL;
  self->task_code_ = task_code;
}

/* member function -----------------------------------------------------------*/
void Task_create_freertos_task(Task *const self, const char *const task_name,
                               UBaseType_t task_priority,
                               StackType_t *const stack,
                               const uint32_t stack_size) {
  self->task_handle_ =
      xTaskCreateStatic(self->task_code_, task_name, stack_size, (void *)self,
                        task_priority, stack, &self->task_control_block_);
  self->state_ = TASK_RUNNING;
}
