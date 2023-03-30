#include "mock/hal_can_mock.hpp"

// gtest include
#include "cmock/cmock.h"

// stm32 include
#if defined(STM32G431xx)
#include "stm32g4xx_hal.h"
#elif defined(STM32H723xx)
#include "stm32h7xx_hal.h"
#endif

HAL_CAN_Mock::HAL_CAN_Mock() {}

HAL_CAN_Mock::~HAL_CAN_Mock() {}

CMOCK_MOCK_FUNCTION(HAL_CAN_Mock, HAL_StatusTypeDef,
                    HAL_FDCAN_AddMessageToTxFifoQ,
                    (FDCAN_HandleTypeDef *, FDCAN_TxHeaderTypeDef *,
                     uint8_t *));

CMOCK_MOCK_FUNCTION(HAL_CAN_Mock, HAL_StatusTypeDef, HAL_FDCAN_GetRxMessage,
                    (FDCAN_HandleTypeDef *, uint32_t, FDCAN_RxHeaderTypeDef *,
                     uint8_t *));

CMOCK_MOCK_FUNCTION(HAL_CAN_Mock, uint32_t, HAL_FDCAN_GetRxFifoFillLevel,
                    (FDCAN_HandleTypeDef *, uint32_t));
