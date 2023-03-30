#ifndef HAL_GPIO_MOCK_HPP
#define HAL_GPIO_MOCK_HPP

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

/// @brief Class for mocking stm32 HAL_GPIO function using google test
/// framework.
class HAL_GPIO_Mock : public CMockMocker<HAL_GPIO_Mock> {
 public:
  HAL_GPIO_Mock();

  virtual ~HAL_GPIO_Mock();

  CMOCK_MOCK_METHOD(GPIO_PinState, HAL_GPIO_ReadPin,
                    (GPIO_TypeDef *, uint16_t));

  CMOCK_MOCK_METHOD(void, HAL_GPIO_WritePin,
                    (GPIO_TypeDef *, uint16_t, GPIO_PinState));

  CMOCK_MOCK_METHOD(void, HAL_GPIO_TogglePin, (GPIO_TypeDef *, uint16_t));
};

#endif  // HAL_GPIO_MOCK_HPP
