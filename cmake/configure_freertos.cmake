################################################################################
# setting
################################################################################
# use freertos kernel from stm32cube
set(FREERTOS_KERNEL_PATH ${STM32CUBE_PATH}/Middlewares/Third_Party/FreeRTOS/Source)
set(FREERTOS_KERNEL_MEMMANG_PATH ${FREERTOS_KERNEL_PATH}/portable/MemMang)
# use posix port for testing
set(FREERTOS_KERNEL_PORT_PATH ${PROJECT_SOURCE_DIR}/lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix)
set(FREERTOS_DEPENDENCY_PATH ${PROJECT_SOURCE_DIR}/lib/lib_depends/freertos)

################################################################################
# build
################################################################################
# build as a dynamic library for c-mock to mock out at link time
add_library(freertos_kernel SHARED
    ${FREERTOS_KERNEL_PATH}/croutine.c
    ${FREERTOS_KERNEL_PATH}/event_groups.c
    ${FREERTOS_KERNEL_PATH}/list.c
    ${FREERTOS_KERNEL_PATH}/queue.c
    ${FREERTOS_KERNEL_PATH}/stream_buffer.c
    ${FREERTOS_KERNEL_PATH}/tasks.c
    ${FREERTOS_KERNEL_PATH}/timers.c
    # posix port only supports heap_3
    ${FREERTOS_KERNEL_MEMMANG_PATH}/heap_3.c
    ${FREERTOS_KERNEL_PORT_PATH}/port.c
    ${FREERTOS_KERNEL_PORT_PATH}/utils/wait_for_event.c
    ${FREERTOS_DEPENDENCY_PATH}/src/freertos_fix.c
)
target_include_directories(freertos_kernel PUBLIC
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PORT_PATH}
    ${FREERTOS_KERNEL_PORT_PATH}/utils
    ${FREERTOS_DEPENDENCY_PATH}/include
)

################################################################################
# function
################################################################################
# function for configureing include directories from freertos headers
function(configure_freertos_kernel_include name)
    target_include_directories(${name} PUBLIC
        ${FREERTOS_KERNEL_PATH}/CMSIS_RTOS_V2
        ${FREERTOS_KERNEL_PATH}/include
        ${FREERTOS_KERNEL_PORT_PATH}
        ${PROJECT_SOURCE_DIR}/include/dependency_include
    )
endfunction()

# function to link library from freertos kernel
function(link_freertos_kernel_library name)
    target_link_libraries(${name}
        freertos_kernel
    )
    add_dependencies(${name}
        freertos_kernel
    )
endfunction()
