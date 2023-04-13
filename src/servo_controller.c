#include "stm32_module/servo_controller.h"

// glibc include
#include <stdint.h>

// stm32 include
#include "stm32_module/stm32_hal.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* constructor ---------------------------------------------------------------*/
void ServoController_ctor(ServoController* self) {
  module_assert(IS_NOT_NULL(self));

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

  if (HAL_TIM_PWM_Start(timer, channel) != HAL_OK) {
    return ModuleError;
  }

  servo_cb->CCR = (uint32_t*)&timer->Instance->CCR1 + channel;

  taskENTER_CRITICAL();
  List_push_back(&self->servo_list_, &servo_cb->servo_list_cb, (void*)servo_cb);
  taskEXIT_CRITICAL();
  return ModuleOK;
}

ModuleRet ServoController_set(ServoController* const self, const int servo_num,
                              const float value) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_SERVO_VALUE(value));

  if (List_size(&self->servo_list_) <= servo_num) {
    return ModuleError;
  }

  taskENTER_CRITICAL();
  struct servo_cb* servo_cb =
      (struct servo_cb*)List_at(&self->servo_list_, servo_num);
  *servo_cb->CCR = (uint32_t)((0.075 + 0.025 * value) * TIMER_PRESCALER);
  taskEXIT_CRITICAL();
}
