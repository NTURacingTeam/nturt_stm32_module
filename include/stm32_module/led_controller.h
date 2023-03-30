/**
 * @file led_controller.h
 * @author QuantumSpawner jet22854111@gmail.com
 * @brief STM32 mcu module for controlling led, and uses freertos task to blink.
 */

#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

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

// stm32_module include
#include "stm32_module/module_common.h"

/* type ----------------------------------------------------------------------*/
typedef enum led_state {
  LED_RESET = 0,
  LED_OFF,
  LED_ON,
  LED_BLINKING,
} LedState;

typedef struct led_control_block {
  GPIO_TypeDef* led_port;

  uint16_t led_pin;

  volatile int ms_to_light;

  LedState state;
} LedControlBlock;

/* class ---------------------------------------------------------------------*/
/**
 * @brief Class for controlling led.
 *
 */
typedef struct led_controller {
  // inherited class
  Task super_;

  // member variable
  int num_led_;

  LedControlBlock* led_control_block_;

  StackType_t task_stack_[128];
} LedController;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for LedController.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] num_led Number of led(s) to control by LedController.
 * @param[in] led_control_block_mem The memory address of the led control
 * block array. The array should be able to contain at least `led_num` of led
 * control block.
 * @return None.
 */
void LedController_ctor(LedController* const self, const int num_led,
                        LedControlBlock* const led_control_block_mem);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function for initializing led control block for each led to controlby
 * this class.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] led_num The number of the led to initialize.
 * @param[in] led_port The port of the led.
 * @param[in] led_pin The pin of the led.
 * @return None.
 */
void LedController_init_led(LedController* const self, const int led_num,
                            GPIO_TypeDef* const led_port,
                            const uint16_t led_pin);

/**
 * @brief Function to add led controller to freertos task. This function should
 * only be called after all leds are initialized.
 *
 * @param[in,out] self The instance of the class.
 * @return ModuleRet Error code.
 */
ModuleRet LedController_start(LedController* const self);

/**
 * @brief Function for turning led on.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] led_num The number of led to turn on.
 * @return ModuleRet Error code.
 */
ModuleRet LedController_turn_on(LedController* const self, const int led_num);

/**
 * @brief Function to turning led off.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] led_num The number of led to turn off.
 * @return ModuleRet Error code.
 */
ModuleRet LedController_turn_off(LedController* const self, const int led_num);

/**
 * @brief Function for blinking led.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] led_num The number of led to blink.
 * @param[in] period How long the led should blink in ms.
 * @return ModuleRet Error code.
 *
 * @note The time quantum to blink led is 10ms, unconditional carry.
 * @note When the led is already turned on, this function has no effect.
 */
ModuleRet LedController_blink(LedController* const self, const int led_num,
                              const int period);

/**
 * @brief Function to run in freertos task.
 *
 * @param[in,out] _self The instance of the class.
 * @return None.
 * @warning For internal use only.
 */
void LedController_task_code(void* const _self);

#ifdef __cplusplus
}
#endif

#endif  // LED_CONTROLLER_H
