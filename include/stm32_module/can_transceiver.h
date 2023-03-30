/**
 * @file can_transceiver.h
 * @author QuantumSpawner jet22854111@gmail.com
 * @brief STM32 mcu module for transceiving can signal.
 */

#ifndef CAN_TRANSCEIVER_H
#define CAN_TRANSCEIVER_H

#ifdef __cplusplus
extern "C" {
#endif

// glibc include
#include <stdbool.h>
#include <stdint.h>

// stm32 include
#if defined(STM32G431xx)
#include "stm32g4xx_hal.h"
#elif defined(STM32H723xx)
#include "stm32h7xx_hal.h"
#endif

// freertos include
#include "FreeRTOS.h"
#include "semphr.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* abstract class ------------------------------------------------------------*/
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
  FDCAN_HandleTypeDef* can_handle_;

  StackType_t task_stack_[256];
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
                         FDCAN_HandleTypeDef* const can_handle);

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

/* class ---------------------------------------------------------------------*/
typedef struct can_frame {
  void* frame_;

  SemaphoreHandle_t mutex_handle_;

  StaticSemaphore_t mutex_control_block_;
} CanFrame;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for CanFrame.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] frame Pointer to the frame struct.
 * @return None.
 */
void CanFrame_ctor(CanFrame* const self, void* const frame);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function for accessing the frame struct.
 *
 * @param[in,out] self The instance of the class.
 * @return void* Pointer to frame struct.
 * @warning This function must be accompanied by CanFrame_end_access() after
 * accessing the frame.
 */
void* CanFrame_access(CanFrame* const self);

/**
 * @brief Function for ending access to the frame struct.
 *
 * @param[in,out] self The instance of the class.
 * @return None.
 * @warning This function must be called after finished accessing frame by
 * CanFrame_access().
 */
void CanFrame_end_access(CanFrame* const self);

#ifdef __cplusplus
}
#endif

#endif  // CAN_TRANSCEIVER_H
