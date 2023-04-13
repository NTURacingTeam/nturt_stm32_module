#ifndef HAL_TIMER_MOCK_HPP
#define HAL_TIMER_MOCK_HPP

// stl include
#include <cstdint>

extern "C" {
// stm32 include
#include "stm32_module/stm32_hal.h"
}

// gtest include
#include "cmock/cmock.h"

/// @brief Class for mocking stm32 HAL_TIM function using google test framework.
class HAL_TIMMock : public CMockMocker<HAL_TIMMock> {
 public:
  HAL_TIMMock();

  ~HAL_TIMMock();

  CMOCK_MOCK_METHOD(HAL_StatusTypeDef, HAL_TIM_PWM_Start,
                    (TIM_HandleTypeDef *, uint32_t));
};

#endif  // HAL_TIMER_MOCK_HPP
