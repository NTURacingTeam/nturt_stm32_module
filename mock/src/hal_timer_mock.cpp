#include "mock/hal_timer_mock.hpp"

// stl include
#include <cstdint>

extern "C" {
// stm32 include
#include "stm32_module/stm32_hal.h"
}

// gtest include
#include "cmock/cmock.h"

HAL_TIMMock::HAL_TIMMock() {}

HAL_TIMMock::~HAL_TIMMock() {}

CMOCK_MOCK_FUNCTION(HAL_TIMMock, HAL_StatusTypeDef, HAL_TIM_PWM_Start,
                    (TIM_HandleTypeDef *, uint32_t));
