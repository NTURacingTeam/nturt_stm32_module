################################################################################
# setting
################################################################################
if(NOT DEFINED STM32FAMILY)
    message(FATAL_ERROR "STM32FAMILY not defined, should be one of G4, H7")
endif()
if(NOT DEFINED STM32TYPE)
    message(FATAL_ERROR "STM32TYPE not defined, should be one of G4xx, H7xx, e.g. G431")
endif()

#library path
set(STM32CUBE_PATH ${PROJECT_SOURCE_DIR}/lib/STM32Cube${STM32FAMILY})
set(STM32CUBE_DEPENDENCY_PATH ${PROJECT_SOURCE_DIR}/lib/lib_depends/stm32cube)

# specify stm32 type to compile definition for cmsis
add_compile_definitions(STM32${STM32TYPE}xx)

################################################################################
# build
################################################################################
# not building stm32cube as it will be mocked out

################################################################################
# function
################################################################################
# function for configuring include directories from stm32 headers
function(configure_stm32_include name)
    target_include_directories(${name} PUBLIC
        ${STM32CUBE_PATH}/Drivers/CMSIS/Core/Include
        ${STM32CUBE_PATH}/Drivers/CMSIS/Device/ST/STM32${STM32FAMILY}xx/Include
        ${STM32CUBE_PATH}/Drivers/STM32${STM32FAMILY}xx_HAL_Driver/Inc
        ${STM32CUBE_PATH}/Drivers/STM32${STM32FAMILY}xx_HAL_Driver/Inc/Legacy
        ${STM32CUBE_DEPENDENCY_PATH}/include
    )
endfunction()
