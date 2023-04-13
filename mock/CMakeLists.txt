################################################################################
# build
################################################################################
# library: mock
add_library(mock
    src/can_transceiver_mock.cpp
    src/cmsis_os2_mock.cpp
    src/freertos_mock.cpp
    src/hal_can_mock.cpp
    src/hal_gpio_mock.cpp
    src/hal_timer_mock.cpp
    src/mock_common.cpp
)
target_include_directories(mock PUBLIC
    include
    ${PROJECT_SOURCE_DIR}/include
)

configure_stm32_include(mock)
configure_freertos_kernel_include(mock)
configure_googletest_include(mock)

link_freertos_kernel_library(mock)
link_googletest_library(mock)

################################################################################
# function
################################################################################
# function for configureing include directories from mock drivers
function(configure_mock_include name)
    target_include_directories(${name} PUBLIC
        ${PROJECT_SOURCE_DIR}/test/mock/include
    )
endfunction()

# function to link library from mocked dependency
function(link_mock_library name)
    target_link_libraries(${name}
        mock
    )
    add_dependencies(${name}
        mock
    )
endfunction()
