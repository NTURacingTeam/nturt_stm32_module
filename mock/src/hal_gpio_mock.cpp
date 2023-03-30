#include "mock/hal_gpio_mock.hpp"

// gtest include
#include "cmock/cmock.h"

// stm32 include
#if defined(STM32G431xx)
#include "stm32g4xx_hal.h"
#elif defined(STM32H723xx)
#include "stm32h7xx_hal.h"
#endif

HAL_GPIO_Mock::HAL_GPIO_Mock() {}

HAL_GPIO_Mock::~HAL_GPIO_Mock() {}

CMOCK_MOCK_FUNCTION(HAL_GPIO_Mock, GPIO_PinState, HAL_GPIO_ReadPin,
                    (GPIO_TypeDef *, uint16_t));

CMOCK_MOCK_FUNCTION(HAL_GPIO_Mock, void, HAL_GPIO_WritePin,
                    (GPIO_TypeDef *, uint16_t, GPIO_PinState));

CMOCK_MOCK_FUNCTION(HAL_GPIO_Mock, void, HAL_GPIO_TogglePin,
                    (GPIO_TypeDef *, uint16_t));
