/**
 * @file button_monitor.h
 * @author QuantumSpawner jet22854111@gmail.com
 * @brief STM32 module for monitoring button.
 */

#ifndef BUTTON_MONITOR_H
#define BUTTON_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

// stm32 include
#include "stm32_module/stm32_hal.h"

// freertos include
#include "FreeRTOS.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* macro ---------------------------------------------------------------------*/
#define BUTTON_DEBOUNCE_TIMES 5

/* type ----------------------------------------------------------------------*/
struct button_cb {
  GPIO_TypeDef* button_port;

  uint16_t button_pin;

  GPIO_PinState state;

  int debounce_count;

  /// @brief List control block for tracking the list of buttons.
  struct list_cb button_list_cb;
};

/* class ---------------------------------------------------------------------*/
typedef struct button_monitor {
  // inherited class
  Task super_;

  // member variable
  List button_list_;

  StackType_t task_stack_[128];
} ButtonMonitor;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for ButtonMonitor.
 *
 * @param[in,out] self The instance of the class.
 * @return None.
 */
void ButtonMonitor_ctor(ButtonMonitor* const self);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function to add button monitor to freertos task.
 *
 * @param[in,out] self The instance of the class.
 * @return ModuleRet Error code.
 */
ModuleRet ButtonMonitor_start(ButtonMonitor* const self);

/**
 * @brief Function for adding button to mutton monitor.
 *
 * @param[in,out] self The instance of the class.
 * @param[in,out] button_cb Button control block for the button.
 * @param[in] button_port The port of the button.
 * @param[in] button_pin The pin of the button.
 * @return ModuleRet Error code.
 * @note User is resposible for allocating memory for button_cb.
 */
ModuleRet ButtonMonitor_add_button(ButtonMonitor* const self,
                                   struct button_cb* const button_cb,
                                   GPIO_TypeDef* const button_port,
                                   const uint16_t button_pin);

/**
 * @brief Function for reading button state.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] led_num The number of button to read, which is the order of adding
 * button, starting from 0.
 * @param[out] state The state of the button.
 * @return ModuleRet Error code.
 */
ModuleRet ButtonMonitor_read_state(ButtonMonitor* const self,
                                   const int button_num,
                                   GPIO_PinState* const state);

/**
 * @brief Function to run in freertos task.
 *
 * @param[in,out] _self The instance of the class.
 * @return None.
 * @warning For internal use only.
 */
void ButtonMonitor_task_code(void* const _self);

#ifdef __cplusplus
}
#endif

#endif  // BUTTON_MONITOR_H
