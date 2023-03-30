#ifndef HAL_CAN_MOCK_HPP
#define HAL_CAN_MOCK_HPP

// glibc include
#include <stdint.h>

// gtest include
#include "cmock/cmock.h"

// stm32 include
#if defined(STM32G431xx)
#include "stm32g4xx_hal.h"
#elif defined(STM32H723xx)
#include "stm32h7xx_hal.h"
#endif

/// @brief Class for mocking stm32 HAL_CAN function using google test framework.
class HAL_CAN_Mock : public CMockMocker<HAL_CAN_Mock> {
 public:
  HAL_CAN_Mock();

  ~HAL_CAN_Mock();

  CMOCK_MOCK_METHOD(HAL_StatusTypeDef, HAL_FDCAN_AddMessageToTxFifoQ,
                    (FDCAN_HandleTypeDef *, FDCAN_TxHeaderTypeDef *,
                     uint8_t *));

  CMOCK_MOCK_METHOD(HAL_StatusTypeDef, HAL_FDCAN_GetRxMessage,
                    (FDCAN_HandleTypeDef *, uint32_t, FDCAN_RxHeaderTypeDef *,
                     uint8_t *));

  CMOCK_MOCK_METHOD(uint32_t, HAL_FDCAN_GetRxFifoFillLevel,
                    (FDCAN_HandleTypeDef *, uint32_t));
};

#endif  // HAL_CAN_MOCK_HPP
