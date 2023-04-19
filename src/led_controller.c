#include "stm32_module/led_controller.h"

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
inline ModuleRet LedController_start(LedController* const self) {
  return self->super_.vptr_->start((Task*)self);
}

/* virtual function definition -----------------------------------------------*/
// from Task base class
ModuleRet __LedController_start(Task* const _self) {
  module_assert(IS_NOT_NULL(_self));

  LedController* const self = (LedController*)_self;
  return Task_create_freertos_task((Task*)self, "led_controller",
                                   TaskPriorityLow, self->task_stack_,
                                   LED_CONTROLLER_TASK_STACK_SIZE);
}

/* constructor ---------------------------------------------------------------*/
void LedController_ctor(LedController* const self) {
  module_assert(IS_NOT_NULL(self));

  // construct inherited class and redirect virtual function
  Task_ctor((Task*)self, LedController_task_code);
  static struct TaskVtbl vtbl = {
      .start = __LedController_start,
  };
  self->super_.vptr_ = &vtbl;

  // initialize member variable
  List_ctor(&self->led_list_);
}

/* member function -----------------------------------------------------------*/
ModuleRet LedController_add_led(LedController* const self,
                                struct led_cb* const led_cb,
                                GPIO_TypeDef* const led_port,
                                const uint16_t led_pin) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(led_cb));
  module_assert(IS_GPIO_ALL_INSTANCE(led_port));
  module_assert(IS_GPIO_PIN(led_pin));

  if (self->super_.state_ != TaskReset) {
    return ModuleError;
  }

  led_cb->led_port = led_port;
  led_cb->led_pin = led_pin;
  led_cb->ms_to_light = 0;
  led_cb->state = LedOff;

  taskENTER_CRITICAL();
  List_push_back(&self->led_list_, &led_cb->led_list_cb, (void*)led_cb);
  taskEXIT_CRITICAL();
  return ModuleOK;
}

ModuleRet LedController_turn_on(LedController* const self, const int led_num) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NEGATIVE(led_num));

  if (self->super_.state_ != TaskRunning ||
      led_num >= List_size(&self->led_list_)) {
    return ModuleError;
  }

  taskENTER_CRITICAL();
  struct led_cb* led_cb = (struct led_cb*)List_at(&self->led_list_, led_num);
  if (led_cb->state != LedON) {
    if (led_cb->state == LedOff) {
      HAL_GPIO_WritePin(led_cb->led_port, led_cb->led_pin, GPIO_PIN_SET);
    }

    led_cb->state = LedON;
  }
  taskEXIT_CRITICAL();

  return ModuleOK;
}

ModuleRet LedController_turn_off(LedController* const self, const int led_num) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NEGATIVE(led_num));

  if (self->super_.state_ != TaskRunning ||
      List_size(&self->led_list_) <= led_num) {
    return ModuleError;
  }

  taskENTER_CRITICAL();
  struct led_cb* led_cb = (struct led_cb*)List_at(&self->led_list_, led_num);
  if (led_cb->state != LedOff) {
    led_cb->ms_to_light = 0;
    HAL_GPIO_WritePin(led_cb->led_port, led_cb->led_pin, GPIO_PIN_RESET);
    led_cb->state = LedOff;
  }
  taskEXIT_CRITICAL();

  return ModuleOK;
}

ModuleRet LedController_blink(LedController* const self, const int led_num,
                              const int period) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NEGATIVE(led_num));
  module_assert(IS_POSTIVE(period));

  if (self->super_.state_ != TaskRunning ||
      List_size(&self->led_list_) <= led_num) {
    return ModuleError;
  }

  taskENTER_CRITICAL();
  struct led_cb* led_cb = (struct led_cb*)List_at(&self->led_list_, led_num);
  if (led_cb->state == LedON) {
    taskEXIT_CRITICAL();
    return ModuleBusy;
  }

  if (led_cb->state != LedON && led_cb->ms_to_light < period) {
    led_cb->ms_to_light = period;
  }

  if (led_cb->state == LedOff) {
    HAL_GPIO_WritePin(led_cb->led_port, led_cb->led_pin, GPIO_PIN_SET);
    led_cb->state = LedBlinking;
  }
  taskEXIT_CRITICAL();

  return ModuleOK;
}

void LedController_task_code(void* _self) {
  LedController* const self = (LedController*)_self;
  TickType_t last_wake = xTaskGetTickCount();

  while (1) {
    ListIter led_iter;
    ListIter_ctor(&led_iter, &self->led_list_);

    taskENTER_CRITICAL();
    struct led_cb* led_cb = (struct led_cb*)ListIter_next(&led_iter);
    while (led_cb != NULL) {
      if (led_cb->state == LedBlinking) {
        if (led_cb->ms_to_light <= 0) {
          HAL_GPIO_WritePin(led_cb->led_port, led_cb->led_pin, GPIO_PIN_RESET);
          led_cb->state = LedOff;
        } else {
          led_cb->ms_to_light -= 10;
        }
      }

      led_cb = (struct led_cb*)ListIter_next(&led_iter);
    }
    taskEXIT_CRITICAL();

    vTaskDelayUntil(&last_wake, LED_CONTROLLER_TASK_PERIOD);
  }
}
