/**
 * @file error_handler.h
 * @author QuantumSpawner jet22854111@gmail.com
 * @brief STM32 mcu module for handling error.
 */

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

// glibc include
#include <stdint.h>

// freertos include
#include "FreeRTOS.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* macro ---------------------------------------------------------------------*/
// parmeter
#define ERROR_HANDLER_TASK_PRIORITY TaskPriorityRealtime
#define ERROR_HANDLER_TASK_STACK_SIZE (8 * configMINIMAL_STACK_SIZE)

#define MAX_ERROR_CODE_BITS 31UL
#define ERROR_CODE_INVALID_BITS (~((1UL << MAX_ERROR_CODE_BITS) - 1UL))

// error_code
#define ERROR_CODE_NO_ERROR 0UL
#define ERROR_CODE_ALL ((1UL << MAX_ERROR_CODE_BITS) - 1UL)

#define ERROR_CODE_CAN_TX 0x00000001UL
#define ERROR_CODE_CAN_RX_CRITICAL 0x00000002UL
#define ERROR_CODE_CAN_RX_OPTIONAL 0x00000004UL

#define ERROR_CODE_APPS1_LOW 0x00000010UL
#define ERROR_CODE_APPS1_HIGH 0x00000020UL
#define ERROR_CODE_APPS2_LOW 0x00000040UL
#define ERROR_CODE_APPS2_HIGH 0x00000080UL
#define ERROR_CODE_APPS_DIVERGE 0x00000100UL

#define ERROR_CODE_APPS_MASK                                             \
  (ERROR_CODE_APPS1_LOW | ERROR_CODE_APPS1_HIGH | ERROR_CODE_APPS2_LOW | \
   ERROR_CODE_APPS2_HIGH | ERROR_CODE_APPS_DIVERGE)

#define ERROR_CODE_BSE_LOW 0x00000200UL
#define ERROR_CODE_BSE_HIGH 0x00000400UL

#define ERROR_CODE_BSE_MASK (ERROR_CODE_BSE_LOW | ERROR_CODE_BSE_HIGH)

#define ERROR_CODE_PEDAL_IMPLAUSIBILITY 0x00000800UL

#define ERROR_CODE_ADC 0x00001000UL
#define ERROR_CODE_AMT22 0x00002000UL
#define ERROR_CODE_D6T 0x00004000UL

// error_code_option
#define ERROR_SET (1UL << MAX_ERROR_CODE_BITS)
#define ERROR_CLEAR 0UL

// assert macro
#define IS_ERROR_CODE(CODE)                                      \
  ((CODE) & (ERROR_CODE_CAN_TX | ERROR_CODE_CAN_RX_CRITICAL |    \
             ERROR_CODE_CAN_RX_OPTIONAL | ERROR_CODE_APPS_MASK | \
             ERROR_CODE_ADC | ERROR_CODE_AMT22 | ERROR_CODE_D6T | \
             ERROR_CODE_BSE_MASK | ERROR_CODE_PEDAL_IMPLAUSIBILITY)) 
#define IS_ERROR_OPTION(CODE_WRITE) \
  (((CODE_WRITE) == ERROR_SET) || ((CODE_WRITE) == ERROR_CLEAR))

/* type ----------------------------------------------------------------------*/
typedef void (*ErrorCallback_t)(void*, uint32_t);

/// @brief Struct for error callback control block.
struct error_callback_cb {
  ErrorCallback_t callback;

  void* arg;

  uint32_t error_code;

  /// @brief List control block for tracking the list of error callback.
  struct list_cb error_callback_list_cb;
};

/* class inherited from Task -------------------------------------------------*/
/**
 * @brief Class for handling error.
 *
 */
typedef struct error_handler {
  // inherited class
  Task super_;

  // member variable
  uint32_t error_code_;

  List error_callback_list_;

  StackType_t task_stack_[ERROR_HANDLER_TASK_STACK_SIZE];
} ErrorHandler;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for ErrorHandler.
 *
 * @param[in,out] self The instance of the class.
 * @return None.
 */
void ErrorHandler_ctor(ErrorHandler* const self);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function to add error handler to freertos task.
 *
 * @param[in,out] self The instance of the class.
 * @return ModuleRet Error code.
 */
ModuleRet ErrorHandler_start(ErrorHandler* const self);

/**
 * @brief Function to add error callback.
 *
 * @param[in,out] self The instance of the class.
 * @param[in,out] error_callback_cb Error callback control block for the
 * callback.
 * @param[in] callback The callback function.
 * @param[in] arg The argument of the callback function.
 * @param[in] error_code The error code when matched to call the callback.
 * @return ModuleRet Error code.
 * @note User is resposible for managing memory for error_callback_cb.
 */
ModuleRet ErrorHandler_add_error_callback(
    ErrorHandler* const self, struct error_callback_cb* const error_callback_cb,
    ErrorCallback_t callback, void* const arg, const uint32_t error_code);

/**
 * @brief Function to set or clear error.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] error_code Code correspond to the error.
 * @param[in] option Set or clear the error.
 * @return ModuleRet Error code.
 */
ModuleRet ErrorHandler_write_error(ErrorHandler* const self,
                                   const uint32_t error_code,
                                   const uint32_t option);

/**
 * @brief Function to get current error code.
 *
 * @param[in,out] self The instance of the class.
 * @param[out] uint32_t Error code.
 * @return ModuleRet Error code.
 */
ModuleRet ErrorHandler_get_error(const ErrorHandler* const self,
                                 uint32_t* code);

/**
 * @brief Function to run in freertos task.
 *
 * @param[in,out] _self The instance of the class.
 * @return None.
 * @warning For internal use only.
 */
void ErrorHandler_task_code(void* const _self);

#ifdef __cplusplus
}
#endif

#endif  // ERROR_HANDLER_H
