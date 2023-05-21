#include "stm32_module/servo_controller.h"

// glibc include
#include <math.h>
#include <stdint.h>

// stm32 include
#include "stm32_module/stm32_hal.h"

// freertos include
#include "FreeRTOS.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* virtual function redirection ----------------------------------------------*/
inline ModuleRet ServoController_start(ServoController* const self) {
  return self->super_.vptr_->start((Task*)self);
}

/* virtual function definition -----------------------------------------------*/
// from Task base class
ModuleRet __ServoController_start(Task* const _self) {
  module_assert(IS_NOT_NULL(_self));

  ServoController* const self = (ServoController*)_self;
  return Task_create_freertos_task(
      (Task*)self, "servo_controller", SERVO_CONTROLLER_TASK_PRIORITY,
      self->task_stack_, SERVO_CONTROLLER_TASK_STACK_SIZE);
}

/* constructor ---------------------------------------------------------------*/
void ServoController_ctor(ServoController* self) {
  module_assert(IS_NOT_NULL(self));

  // construct inherited class and redirect virtual function
  Task_ctor((Task*)self, ServoController_task_code);
  static struct TaskVtbl vtbl = {
      .start = __ServoController_start,
  };
  self->super_.vptr_ = &vtbl;

  // initialize member variable
  List_ctor(&self->servo_list_);
}

/* member function -----------------------------------------------------------*/
ModuleRet ServoController_add_servo(ServoController* const self,
                                    struct servo_cb* const servo_cb,
                                    TIM_HandleTypeDef* const timer,
                                    const uint32_t channel) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(servo_cb));
  module_assert(IS_TIM_CCX_INSTANCE(timer->Instance, channel));

  if (self->super_.state_ != TaskReset ||
      HAL_TIM_PWM_Start(timer, channel) != HAL_OK) {
    return ModuleError;
  }

  servo_cb->timer = timer;
  servo_cb->channel = channel;
  servo_cb->current_duty = 0.0;
  servo_cb->direction = SERVO_COUNTER_CLOCKWISE;

  taskENTER_CRITICAL();
  List_push_back(&self->servo_list_, &servo_cb->servo_list_cb, (void*)servo_cb);
  taskEXIT_CRITICAL();
  return ModuleOK;
}

ModuleRet ServoController_set_direction(ServoController* const self,
                                        const int servo_num,
                                        const int direction) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NEGATIVE(servo_num));
  module_assert(IS_SERVO_DIRECTION(direction));

  if (self->super_.state_ != TaskRunning ||
      List_size(&self->servo_list_) <= servo_num) {
    return ModuleError;
  }

  taskENTER_CRITICAL();
  struct servo_cb* servo_cb =
      (struct servo_cb*)List_at(&self->servo_list_, servo_num);
  if (direction != servo_cb->direction) {
    servo_cb->direction = direction;
    servo_cb->current_duty = 0.0;
  }
  taskEXIT_CRITICAL();
  return ModuleOK;
}

ModuleRet ServoController_set_duty(ServoController* const self,
                                   const int servo_num, const float duty) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NEGATIVE(servo_num));
  module_assert(IS_SERVO_DUTY(duty));

  if (self->super_.state_ != TaskRunning ||
      List_size(&self->servo_list_) <= servo_num) {
    return ModuleError;
  }

  taskENTER_CRITICAL();
  struct servo_cb* servo_cb =
      (struct servo_cb*)List_at(&self->servo_list_, servo_num);
  servo_cb->target_duty = duty;
  taskEXIT_CRITICAL();
  return ModuleOK;
}

void ServoController_task_code(void* const _self) {
  ServoController* const self = (ServoController*)_self;
  TickType_t last_wake = xTaskGetTickCount();

  while (1) {
    ListIter servo_iter;
    ListIter_ctor(&servo_iter, &self->servo_list_);

    taskENTER_CRITICAL();
    struct servo_cb* servo_cb = (struct servo_cb*)ListIter_next(&servo_iter);
    while (servo_cb != NULL) {
      if (servo_cb->current_duty != servo_cb->target_duty) {
        if (servo_cb->current_duty <
            servo_cb->target_duty - SERVO_ACCELERATE_SLOPE) {
          servo_cb->current_duty += SERVO_ACCELERATE_SLOPE;
        } else {
          servo_cb->current_duty = servo_cb->target_duty;
        }

        __HAL_TIM_SET_COMPARE(servo_cb->timer, servo_cb->channel,
                              (uint32_t)((0.075 + 0.025 * servo_cb->direction *
                                                      servo_cb->current_duty) *
                                         SERVO_TIMER_PRESCALER));
      }

      servo_cb = (struct servo_cb*)ListIter_next(&servo_iter);
    }
    taskEXIT_CRITICAL();

    vTaskDelayUntil(&last_wake, SERVO_CONTROLLER_TASK_PERIOD);
  }
}
