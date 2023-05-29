/**
 * @file can_transceiver.h
 * @author QuantumSpawner jet22854111@gmail.com
 * @brief STM32 mcu module for transceiving can signal.
 */

#ifndef STM32_MODULE_CAN_TRANSCEIVER_H
#define STM32_MODULE_CAN_TRANSCEIVER_H

#ifdef __cplusplus
extern "C" {
#endif

// glibc include
#include <stdbool.h>
#include <stdint.h>

// stm32 include
#include "stm32_module/stm32_hal.h"

// freertos include
#include "FreeRTOS.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* macro ---------------------------------------------------------------------*/
// parmeter
#define CAN_TRANSCEIVER_TASK_PRIORITY TaskPriorityAboveNormal
#define CAN_TRANSCEIVER_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define CAN_TRANSCEIVER_TASK_PERIOD 5

// assert macro
#define IS_DLC(DLC) ((DLC) <= 8U)

/* type ----------------------------------------------------------------------*/
#if defined(HAL_CAN_MODULE_ENABLED)
typedef CAN_HandleTypeDef CanHandle;
#elif defined(HAL_FDCAN_MODULE_ENABLED)
typedef FDCAN_HandleTypeDef CanHandle;
#endif

/* abstract class inherited from Task ----------------------------------------*/
// forward declaration
struct CanTransceiverVtbl;

/**
 * @brief Abstract class for transceiving can signal.
 *
 */
typedef struct can_transceiver {
  // inherited class
  Task super_;

  // virtual table
  struct CanTransceiverVtbl* vptr_;

  // member variable
  CanHandle* can_handle_;

  StackType_t task_stack_[CAN_TRANSCEIVER_TASK_STACK_SIZE];

  /// @brief List control block for tracking the list of can transceivers.
  struct list_cb can_transceiver_list_cb;
} CanTransceiver;

/// @brief Virtual table for CanTransceiver class.
struct CanTransceiverVtbl {
  ModuleRet (*configure)(CanTransceiver*);

  ModuleRet (*receive)(CanTransceiver*, bool, uint32_t, uint8_t,
                       const uint8_t*);

  ModuleRet (*receive_hp)(CanTransceiver*, bool, uint32_t, uint8_t,
                          const uint8_t*);

  ModuleRet (*periodic_update)(CanTransceiver*, TickType_t);
};

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for CanTransceiver.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] can_handle Fdcan handler.
 * @return None.
 */
void CanTransceiver_ctor(CanTransceiver* const self,
                         CanHandle* const can_handle);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function to add can transceiver to freertos task.
 *
 * @param[in,out] self The instance of the class.
 * @return ModuleRet Error code.
 */
ModuleRet CanTransceiver_start(CanTransceiver* const self);

/**
 * @brief Function to configure can peripherial settings before starting.
 *
 * @param[in,out] self The instance of the class.
 * @return ModuleRet Error code.
 * @note This function is virtual.
 */
ModuleRet CanTransceiver_configure(CanTransceiver* const self);

/**
 * @brief Function for receiving can frame.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] is_extended If the frame is extended.
 * @param[in] id ID of the frame.
 * @param[in] dlc Data length code.
 * @param[in] data Data of the frame.
 * @return ModuleRet Error code.
 * @note This function is virtual.
 */
ModuleRet CanTransceiver_receive(CanTransceiver* const self,
                                 const bool is_extended, const uint32_t id,
                                 const uint8_t dlc, const uint8_t* const data);

/**
 * @brief Function for receiving high priority can frame.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] is_extended If the frame is extended.
 * @param[in] id ID of the frame.
 * @param[in] dlc Data length code.
 * @param[in] data Data of the frame.
 * @return ModuleRet Error code.
 * @note This function is virtual.
 */
ModuleRet CanTransceiver_receive_hp(CanTransceiver* const self,
                                    const bool is_extended, const uint32_t id,
                                    const uint8_t dlc,
                                    const uint8_t* const data);

/**
 * @brief Function for transmitting can frame.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] is_extended If the frame is extended.
 * @param[in] id ID of the frame.
 * @param[in] dlc Data length code.
 * @param[in] data Data of the frame.
 * @return ModuleRet Error code.
 */
ModuleRet CanTransceiver_transmit(CanTransceiver* const self,
                                  const bool is_extended, const uint32_t id,
                                  const uint8_t dlc, uint8_t* const data);

/**
 * @brief Function for doing periodic chores, e.g. checking for timeout, sending
 * periodic message.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] current_tick Current tick time.
 * @return ModuleRet Error code.
 * @note This function is virtual.
 */
ModuleRet CanTransceiver_periodic_update(CanTransceiver* const self,
                                         const TickType_t current_tick);

void CanTransceiver_task_code(void* const self);

#ifdef __cplusplus
}
#endif

#endif  // STM32_MODULE_CAN_TRANSCEIVER_H
