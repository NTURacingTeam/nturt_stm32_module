#include "mock/hal_gpio_mock.hpp"

extern "C" {
// stm32 include
#include "stm32_module/stm32_hal.h"
}

// gtest include
#include "cmock/cmock.h"

HAL_GPIOMock::HAL_GPIOMock() {}

HAL_GPIOMock::~HAL_GPIOMock() {}

CMOCK_MOCK_FUNCTION(HAL_GPIOMock, GPIO_PinState, HAL_GPIO_ReadPin,
                    (GPIO_TypeDef *, uint16_t));

CMOCK_MOCK_FUNCTION(HAL_GPIOMock, void, HAL_GPIO_WritePin,
                    (GPIO_TypeDef *, uint16_t, GPIO_PinState));

CMOCK_MOCK_FUNCTION(HAL_GPIOMock, void, HAL_GPIO_TogglePin,
                    (GPIO_TypeDef *, uint16_t));
