#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

// glibc include
#include <stdint.h>

// stm32 include
#include "stm32_module/stm32_hal.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* macro ---------------------------------------------------------------------*/
#define TIMER_PRESCALER 50000

// assert macro
#define IS_SERVO_VALUE(VAL) (((VAL) >= -1.0) && ((VAL) <= 1.0))
#define IS_PWM_CHANNEL(VAL)                                \
  (((VAL) == TIM_CHANNEL_1) || ((VAL) == TIM_CHANNEL_2) || \
   ((VAL) == TIM_CHANNEL_3) || ((VAL) == TIM_CHANNEL_4))

/* type ----------------------------------------------------------------------*/
struct servo_cb {
  /// @brief Pointer to the timer capture / compare register. When timer counter
  /// value is greater than this register value, the output signal is high, or
  /// low otherwise.
  uint32_t* CCR;

  /// @brief List control block for tracking the list of servos.
  struct list_cb servo_list_cb;
};

/* class ---------------------------------------------------------------------*/
/**
 * @brief Class for controlling servo.
 *
 * @note In this module, we assume that the hardware is configured using
 * stm32cubemx, and this module will only start the pwm timer without any other
 * configuration. The user is also responsible for configuring the timer counter
 * period to be 50000, and the prescaler can be calculated using the following
 * formula:
 * \f$\text{prescaler}=0.4\times\text{timer clock in MHz}\f$
 */
typedef struct servo_controller {
  // member variable
  List servo_list_;
} ServoController;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for ServoController class.
 *
 * @param[in,out] self The instance of the class.
 * @return None.
 */
void ServoController_ctor(ServoController* self);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function for adding servo to servo controller.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] servo_cb The servo control block.
 * @param[in] timer The timer for the servo.
 * @param[in] channel The timer channel for the servo.
 * @return ModuleRet Error code.
 * @note User is responsible for allocating memory for servo_cb.
 */
ModuleRet ServoController_add_servo(ServoController* const self,
                                    struct servo_cb* const servo_cb,
                                    TIM_HandleTypeDef* const timer,
                                    const uint32_t channel);

/**
 * @brief Function for setting the servo value.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] servo_num Number of the servo to set value.
 * @param[in] value The value to set the servo to in [-1, 1].
 * @return ModuleRet Error code.
 */
ModuleRet ServoController_set(ServoController* const self, const int servo_num,
                              const float value);

#ifdef __cplusplus
}
#endif

#endif  // SERVO_CONTROLLER_H
