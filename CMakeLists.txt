cmake_minimum_required(VERSION 3.16)
project(stm32_module LANGUAGES C CXX)

################################################################################
# setting
################################################################################
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# compile in 32-bit since the driver of stm32 is designed and written in 32-bit,
# compiling in 64-bit will result in compilation error
add_compile_options(-m32)
add_link_options(-m32)

# build for testing
add_compile_definitions(TESTING)

# set stm32 family and type to use
if(NOT DEFINED STM32FAMILY)
    set(STM32FAMILY H7 CACHE STRING "STM32 family to use")
endif()
if(NOT DEFINED STM32TYPE)
    set(STM32TYPE H723 CACHE STRING "STM32 type to use")
endif()

################################################################################
# dependency
################################################################################
# configure lib
include(cmake/configure_stm32cube.cmake)
include(cmake/configure_freertos.cmake)
include(cmake/configure_can_config.cmake)
include(cmake/configure_googletest.cmake)

# mock library
add_subdirectory(mock)

################################################################################
# build
################################################################################
# library: stm32_module
# build as a dynamic library for c-mock to mock out at link time
add_library(stm32_module SHARED
    src/button_monitor.c
    src/can_transceiver.c
    src/error_handler.c
    src/led_controller.c
    src/module_common.c
)
target_include_directories(stm32_module
    PUBLIC include
)

configure_stm32_include(stm32_module)
configure_freertos_kernel_include(stm32_module)
configure_can_config_include(stm32_module)

link_freertos_kernel_library(stm32_module)
link_can_config_library(stm32_module)
link_mock_library(stm32_module)

################################################################################
# function
################################################################################
# function for configuring include directories from stm32 modules
function(configure_stm32_module_include name)
    target_include_directories(${name} PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    )
endfunction()

# function to link library from stm32 module
function(link_stm32_module_library name)
    target_link_libraries(${name}
        stm32_module
    )
    add_dependencies(${name}
        stm32_module
    )
endfunction()

################################################################################
# testing
################################################################################
add_subdirectory(test)
