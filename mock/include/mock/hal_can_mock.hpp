#ifndef HAL_CAN_MOCK_HPP
#define HAL_CAN_MOCK_HPP

// stl include
#include <cstdint>

extern "C" {
// stm32 include
#if defined(STM32G431xx)
#include "stm32g4xx_hal.h"
#elif defined(STM32H723xx)
#include "stm32h7xx_hal.h"
#endif
}

// gtest include
#include "cmock/cmock.h"

/// @brief Class for mocking stm32 HAL_CAN function using google test framework.
class HAL_CANMock : public CMockMocker<HAL_CANMock> {
 public:
  HAL_CANMock();

  ~HAL_CANMock();

  CMOCK_MOCK_METHOD(HAL_StatusTypeDef, HAL_FDCAN_AddMessageToTxFifoQ,
                    (FDCAN_HandleTypeDef *, FDCAN_TxHeaderTypeDef *,
                     uint8_t *));

  CMOCK_MOCK_METHOD(HAL_StatusTypeDef, HAL_FDCAN_AddMessageToTxBuffer,
                    (FDCAN_HandleTypeDef *, uint32_t, FDCAN_TxHeaderTypeDef *,
                     uint8_t *));

  CMOCK_MOCK_METHOD(HAL_StatusTypeDef, HAL_FDCAN_GetRxMessage,
                    (FDCAN_HandleTypeDef *, uint32_t, FDCAN_RxHeaderTypeDef *,
                     uint8_t *));

  CMOCK_MOCK_METHOD(uint32_t, HAL_FDCAN_GetRxFifoFillLevel,
                    (FDCAN_HandleTypeDef *, uint32_t));
};

#endif  // HAL_CAN_MOCK_HPP
