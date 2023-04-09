/**
 * @file led_controller.h
 * @author QuantumSpawner jet22854111@gmail.com
 * @brief STM32 mcu module for controlling led.
 */

#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

// glibc include
#include <stdint.h>

// stm32 include
#include "stm32_module/stm32_hal.h"

// freertos include
#include "FreeRTOS.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* type ----------------------------------------------------------------------*/
/// @brief Enumerator for state of led.
typedef enum led_state {
  LedReset = 0,
  LedOff,
  LedON,
  LedBlinking,
} LedState;

/// @brief Struct for led control block.
struct led_cb {
  GPIO_TypeDef* led_port;

  uint16_t led_pin;

  LedState state;

  volatile int ms_to_light;

  /// @brief List control block for tracking the list of leds.
  struct list_cb led_list_cb;
};

/* class ---------------------------------------------------------------------*/
/**
 * @brief Class for controlling led.
 *
 */
typedef struct led_controller {
  // inherited class
  Task super_;

  // member variable
  List led_list_;

  StackType_t task_stack_[128];
} LedController;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for LedController.
 *
 * @param[in,out] self The instance of the class.
 * @return None.
 */
void LedController_ctor(LedController* const self);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function for adding led to led controller.
 *
 * @param[in,out] self The instance of the class.
 * @param[in,out] led_cb Led control block for the led.
 * @param[in] led_port The port of the led.
 * @param[in] led_pin The pin of the led.
 * @return ModuleRet Error code.
 * @note User is resposible for allocating memory for led_cb.
 */
ModuleRet LedController_add_led(LedController* const self,
                                struct led_cb* const led_cb,
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
 * @param[in] led_num The number of led to turn on, which is the order of adding
 * led, starting from 0.
 * @return ModuleRet Error code.
 */
ModuleRet LedController_turn_on(LedController* const self, const int led_num);

/**
 * @brief Function to turning led off.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] led_num The number of led to turn off, which is the order of
 * adding led, starting from 0.
 * @return ModuleRet Error code.
 */
ModuleRet LedController_turn_off(LedController* const self, const int led_num);

/**
 * @brief Function for blinking led.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] led_num The number of led to blink, which is the order of addind
 * led, starting from 0.
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
