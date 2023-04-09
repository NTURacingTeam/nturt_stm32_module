#ifndef HAL_CAN_MOCK_HPP
#define HAL_CAN_MOCK_HPP

// stl include
#include <cstdint>

extern "C" {
// stm32 include
#include "stm32_module/stm32_hal.h"
}

// gtest include
#include "cmock/cmock.h"

/// @brief Class for mocking stm32 HAL_CAN function using google test framework.
class HAL_CANMock : public CMockMocker<HAL_CANMock> {
 public:
  HAL_CANMock();

  ~HAL_CANMock();

#if defined(HAL_CAN_MODULE_ENABLED)
  CMOCK_MOCK_METHOD(HAL_StatusTypeDef, HAL_CAN_AddTxMessage,
                    (CAN_HandleTypeDef *, CAN_TxHeaderTypeDef *, uint8_t *,
                     uint32_t *));

  CMOCK_MOCK_METHOD(HAL_StatusTypeDef, HAL_CAN_GetRxMessage,
                    (CAN_HandleTypeDef *, uint32_t, CAN_RxHeaderTypeDef *,
                     uint8_t *));
  CMOCK_MOCK_METHOD(uint32_t, HAL_CAN_GetRxFifoFillLevel,
                    (CAN_HandleTypeDef *, uint32_t));
#elif defined(HAL_FDCAN_MODULE_ENABLED)
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
#endif  // HAL_FDCAN_MODULE_ENABLED
};

#endif  // HAL_CAN_MOCK_HPP
