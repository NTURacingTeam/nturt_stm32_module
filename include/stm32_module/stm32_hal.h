/**
 * @file stm32_hal.h
 * @author QuantumSpawner jet22854111@gmail.com
 * @brief Convenient header files for STM32 HAL library.
 */

#ifndef STM32_HAL_H
#define STM32_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(STM32F446xx)
#include "stm32f4xx_hal.h"
#elif defined(STM32G431xx)
#include "stm32g4xx_hal.h"
#elif defined(STM32H723xx)
#include "stm32h7xx_hal.h"
#endif

#ifdef __cplusplus
}
#endif

#endif  // STM32_HAL_H
