#include "stm32_module/button_monitor.h"

// stm32 include
#include "stm32_module/stm32_hal.h"

// freertos include
#include "FreeRTOS.h"
#include "task.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* virtual function redirection ----------------------------------------------*/
inline ModuleRet ButtonMonitor_start(ButtonMonitor* const self) {
  return self->super_.vptr_->start((Task*)self);
}

/* virtual function definition -----------------------------------------------*/
// from Task base class
ModuleRet __ButtonMonitor_start(Task* const _self) {
  module_assert(IS_NOT_NULL(_self));

  ButtonMonitor* const self = (ButtonMonitor*)_self;
  return Task_create_freertos_task(
      (Task*)self, "button_monitor", BUTTON_MONITOR_TASK_PRIORITY,
      self->task_stack_, BUTTON_MONITOR_TASK_STACK_SIZE);
}

/* constructor ---------------------------------------------------------------*/
void ButtonMonitor_ctor(ButtonMonitor* const self) {
  module_assert(IS_NOT_NULL(self));

  // construct inherited class and redirect virtual function
  Task_ctor((Task*)self, ButtonMonitor_task_code);
  static struct TaskVtbl vtbl = {
      .start = __ButtonMonitor_start,
  };
  self->super_.vptr_ = &vtbl;

  // initialize member variable
  List_ctor(&self->button_list_);
}

/* member function -----------------------------------------------------------*/
ModuleRet ButtonMonitor_add_button(ButtonMonitor* const self,
                                   struct button_cb* const button_cb,
                                   GPIO_TypeDef* const button_port,
                                   const uint16_t button_pin) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(button_cb));
  module_assert(IS_GPIO_ALL_INSTANCE(button_port));
  module_assert(IS_GPIO_PIN(button_pin));

  if (self->super_.state_ != TaskReset) {
    return ModuleError;
  }

  button_cb->button_port = button_port;
  button_cb->button_pin = button_pin;
  button_cb->debounce_count = 0;
  button_cb->callback = NULL;

  taskENTER_CRITICAL();
  List_push_back(&self->button_list_, &button_cb->button_list_cb,
                 (void*)button_cb);
  taskEXIT_CRITICAL();

  return ModuleOK;
}

ModuleRet ButtonMonitor_register_callback(ButtonMonitor* const self,
                                          const int button_num,
                                          const ButtonCallback_t callback,
                                          void* const arg) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NEGATIVE(button_num));
  module_assert(IS_NOT_NULL(callback));

  if (self->super_.state_ != TaskReset ||
      button_num >= List_size(&self->button_list_)) {
    return ModuleError;
  }

  taskENTER_CRITICAL();
  struct button_cb* button_cb =
      (struct button_cb*)List_at(&self->button_list_, button_num);
  button_cb->callback = callback;
  button_cb->arg = arg;
  taskEXIT_CRITICAL();

  return ModuleOK;
}

ModuleRet ButtonMonitor_read_state(ButtonMonitor* const self,
                                   const int button_num,
                                   GPIO_PinState* const state) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NEGATIVE(button_num));
  module_assert(IS_NOT_NULL(state));

  if (self->super_.state_ != TaskRunning ||
      button_num >= List_size(&self->button_list_)) {
    return ModuleError;
  }

  taskENTER_CRITICAL();
  struct button_cb* button_cb =
      (struct button_cb*)List_at(&self->button_list_, button_num);
  *state = button_cb->state;
  taskEXIT_CRITICAL();

  return ModuleOK;
}

void ButtonMonitor_task_code(void* const _self) {
  ButtonMonitor* const self = (ButtonMonitor*)_self;

  TickType_t last_wake = xTaskGetTickCount();

  while (1) {
    ListIter button_iter;
    ListIter_ctor(&button_iter, &self->button_list_);

    taskENTER_CRITICAL();
    struct button_cb* button_cb =
        (struct button_cb*)ListIter_next(&button_iter);
    while (button_cb != NULL) {
      GPIO_PinState current_state =
          HAL_GPIO_ReadPin(button_cb->button_port, button_cb->button_pin);
      if (current_state != button_cb->state) {
        if (button_cb->debounce_count < BUTTON_MONITOR_DEBOUNCE_TIMES) {
          button_cb->debounce_count++;
        } else {
          button_cb->state = current_state;
          button_cb->debounce_count = 0;
          if (button_cb->callback != NULL) {
            taskEXIT_CRITICAL();
            button_cb->callback(button_cb->arg, current_state);
            taskENTER_CRITICAL();
          }
        }
      } else if (button_cb->debounce_count != 0) {
        button_cb->debounce_count = 0;
      }

      button_cb = (struct button_cb*)ListIter_next(&button_iter);
    }
    taskEXIT_CRITICAL();

    vTaskDelayUntil(&last_wake, BUTTON_MONITOR_TASK_PERIOD);
  }
}
