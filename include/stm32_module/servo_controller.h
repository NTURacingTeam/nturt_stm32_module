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

// parameter
#define SERVO_CONTROLLER_TASK_PERIOD 10
#define SERVO_ACCELERATE_SLOPE 0.02F

// servo direction
#define SERVO_CLOCKWISE -1
#define SERVO_COUNTER_CLOCKWISE 1

// assert macro
#define IS_SERVO_DUTY(VAL) (((VAL) >= 0.0) && ((VAL) <= 1.0))
#define IS_SERVO_DIRECTION(VAL) \
  (((VAL) == SERVO_CLOCKWISE) || ((VAL) == SERVO_COUNTER_CLOCKWISE))

/* type ----------------------------------------------------------------------*/

struct servo_cb {
  TIM_HandleTypeDef* timer;

  uint32_t channel;

  int direction;

  float current_duty;

  float target_duty;

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
  // inherited class
  Task super_;

  // member variable
  List servo_list_;

  StackType_t task_stack_[128];
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
 * @brief Function to add servo controller to freertos task. This function
 * should only be called after all servos are added.
 *
 * @param[in,out] self The instance of the class.
 * @return ModuleRet Error code.
 */
ModuleRet ServoController_start(ServoController* const self);

/**
 * @briefFunction for setting the direction of the servo.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] servo_num Number of the servo to set direction.
 * @param[in] direction The direction to set the servo to.
 * @return ModuleRet Error code.
 */
ModuleRet ServoController_set_direction(ServoController* const self,
                                        const int servo_num,
                                        const int direction);

/**
 * @brief Function for setting the servo duty.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] servo_num Number of the servo to set duty.
 * @param[in] duty The duty to set the servo to in [0, 1].
 * @return ModuleRet Error code.
 */
ModuleRet ServoController_set_duty(ServoController* const self,
                                   const int servo_num, const float duty);

/**
 * @brief Function to run in freertos task.
 *
 * @param[in,out] _self The instance of the class.
 * @return None.
 * @warning For internal use only.
 */
void ServoController_task_code(void* const _self);

#ifdef __cplusplus
}
#endif

#endif  // SERVO_CONTROLLER_H
