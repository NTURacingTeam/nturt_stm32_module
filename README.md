# nturt_stm32_module

![build and test status](https://github.com/NTURacingTeam/nturt_stm32_module/actions/workflows/nturt_stm32_module-ci.yml/badge.svg)

Modules designed for use in stm32 microcontrollers written in object-oriented style c.

## Introduction

TBD

## Test

All modules are unit tested with google test framework. For details, please checkout: [test](test/README.md).

### Dependencies

The flollowing test dependencies are included in the repository as submodules, so don't need to install them.

- googletest
- C-Mock
- FreeRTOS-Kernel

However, since this package have to be built in 32-bit, the following packages have to be installed.

- gcc-multilib
- g++-multilib
