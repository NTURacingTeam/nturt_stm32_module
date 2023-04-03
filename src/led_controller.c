#include "stm32_module/led_controller.h"

// glibc include
#include <stdint.h>

// stm32 include
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
inline ModuleRet LedController_start(LedController* const self) {
  return self->super_.vptr_->start((Task*)self);
}

/* virtual function definition -----------------------------------------------*/
// from Task base class
ModuleRet __LedController_start(Task* const _self) {
  module_assert(IS_NOT_NULL(_self));

  LedController* const self = (LedController*)_self;
  if (self->super_.state_ == TaskRunning) {
    return ModuleBusy;
  }

  for (int i = 0; i < self->num_led_; i++) {
    if (self->led_control_block_[i].state == LED_RESET) {
      return ModuleError;
    }
  }

  Task_create_freertos_task((Task*)self, "led_controller", TaskPriorityLow,
                            self->task_stack_,
                            sizeof(self->task_stack_) / sizeof(StackType_t));
  return ModuleOK;
}

/* constructor ---------------------------------------------------------------*/
void LedController_ctor(LedController* const self, const int num_led,
                        LedControlBlock* const led_control_block_mem) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_POSTIVE(num_led));
  module_assert(IS_NOT_NULL(led_control_block_mem));

  // construct inherited class and redirect virtual function
  Task_ctor((Task*)self, LedController_task_code);
  static struct TaskVtbl vtbl = {
      .start = __LedController_start,
  };
  self->super_.vptr_ = &vtbl;

  // initialize member variable
  self->num_led_ = num_led;
  self->led_control_block_ = led_control_block_mem;
}

/* member function -----------------------------------------------------------*/
void LedController_init_led(LedController* const self, const int led_num,
                            GPIO_TypeDef* const led_port,
                            const uint16_t led_pin) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_LESS(led_num, self->num_led_));
  module_assert(IS_GPIO_ALL_INSTANCE(led_port));
  module_assert(IS_GPIO_PIN(led_pin));

  self->led_control_block_[led_num].led_port = led_port;
  self->led_control_block_[led_num].led_pin = led_pin;
  self->led_control_block_[led_num].ms_to_light = 0;
  self->led_control_block_[led_num].state = LED_OFF;
}

ModuleRet LedController_turn_on(LedController* const self, const int led_num) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_LESS(led_num, self->num_led_));

  if (self->super_.state_ != TaskRunning) {
    return ModuleError;
  }

  if (self->led_control_block_[led_num].state != LED_ON) {
    if (self->led_control_block_[led_num].state == LED_OFF) {
      HAL_GPIO_WritePin(self->led_control_block_[led_num].led_port,
                        self->led_control_block_[led_num].led_pin,
                        GPIO_PIN_SET);
    }

    self->led_control_block_[led_num].state = LED_ON;
  }

  return ModuleOK;
}

ModuleRet LedController_turn_off(LedController* const self, const int led_num) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_LESS(led_num, self->num_led_));

  if (self->super_.state_ != TaskRunning) {
    return ModuleError;
  }

  if (self->led_control_block_[led_num].state != LED_OFF) {
    self->led_control_block_[led_num].ms_to_light = 0;
    HAL_GPIO_WritePin(self->led_control_block_[led_num].led_port,
                      self->led_control_block_[led_num].led_pin,
                      GPIO_PIN_RESET);
    self->led_control_block_[led_num].state = LED_OFF;
  }

  return ModuleOK;
}

ModuleRet LedController_blink(LedController* const self, const int led_num,
                              const int period) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_LESS(led_num, self->num_led_));
  module_assert(IS_POSTIVE(period));

  if (self->super_.state_ != TaskRunning) {
    return ModuleError;
  } else if (self->led_control_block_[led_num].state == LED_ON) {
    return ModuleBusy;
  }

  if (self->led_control_block_[led_num].state != LED_ON &&
      self->led_control_block_[led_num].ms_to_light < period) {
    self->led_control_block_[led_num].ms_to_light = period;
  }

  if (self->led_control_block_[led_num].state == LED_OFF) {
    HAL_GPIO_WritePin(self->led_control_block_[led_num].led_port,
                      self->led_control_block_[led_num].led_pin, GPIO_PIN_SET);
    self->led_control_block_[led_num].state = LED_BLINKING;
  }

  return ModuleOK;
}

void LedController_task_code(void* _self) {
  LedController* const self = (LedController*)_self;
  TickType_t last_wake = xTaskGetTickCount();

  while (1) {
    for (int i = 0; i < self->num_led_; i++) {
      if (self->led_control_block_[i].state == LED_BLINKING) {
        if (self->led_control_block_[i].ms_to_light <= 0) {
          HAL_GPIO_WritePin(self->led_control_block_[i].led_port,
                            self->led_control_block_[i].led_pin,
                            GPIO_PIN_RESET);
          self->led_control_block_[i].state = LED_OFF;
        } else {
          self->led_control_block_[i].ms_to_light -= 10;
        }
      }
    }

    vTaskDelayUntil(&last_wake, 10);
  }
}
