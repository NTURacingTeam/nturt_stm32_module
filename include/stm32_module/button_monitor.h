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
// parmeter
#define BUTTON_MONITOR_TASK_PRIORITY TaskPriorityLow
#define BUTTON_MONITOR_TASK_STACK_SIZE (8 * configMINIMAL_STACK_SIZE)
#define BUTTON_MONITOR_TASK_PERIOD 2
#define BUTTON_MONITOR_DEBOUNCE_TIMES 5

/* type ----------------------------------------------------------------------*/
typedef void (*ButtonCallback_t)(void*, GPIO_PinState);

/// @brief Struct for button control block.
struct button_cb {
  GPIO_TypeDef* button_port;

  uint16_t button_pin;

  GPIO_PinState state;

  int debounce_count;

  ButtonCallback_t callback;

  void* arg;

  /// @brief List control block for tracking the list of buttons.
  struct list_cb button_list_cb;
};

/* class ---------------------------------------------------------------------*/
/**
 * @brief Class for monitoring button.
 *
 */
typedef struct button_monitor {
  // inherited class
  Task super_;

  // member variable
  List button_list_;

  StackType_t task_stack_[BUTTON_MONITOR_TASK_STACK_SIZE];
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
 * @brief Function for adding button to button monitor.
 *
 * @param[in,out] self The instance of the class.
 * @param[in,out] button_cb Button control block for the button.
 * @param[in] button_port The port of the button.
 * @param[in] button_pin The pin of the button.
 * @return ModuleRet Error code.
 * @note User is resposible for managing memory for button_cb.
 */
ModuleRet ButtonMonitor_add_button(ButtonMonitor* const self,
                                   struct button_cb* const button_cb,
                                   GPIO_TypeDef* const button_port,
                                   const uint16_t button_pin);

/**
 * @brief Function for registering callback function.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] button_num The number of button to register callback, which is the
 * order of adding button, starting from 0.
 * @param[in] callback The callback function.
 * @param[in] arg The argument of the callback function.
 * @return ModuleRet Error code.
 * @warning The callback function is executed inside a critical section.
 * @warning When a callback function is already registered for that button, it
 * will be overwritten by the new one.
 */
ModuleRet ButtonMonitor_register_callback(ButtonMonitor* const self,
                                          const int button_num,
                                          const ButtonCallback_t callback,
                                          void* const arg);

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
