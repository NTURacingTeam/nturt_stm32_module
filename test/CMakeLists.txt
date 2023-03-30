################################################################################
# setting
################################################################################
include(GoogleTest)

# function for adding a google test target
function(add_gtest name)
    add_executable(${name}
        ${ARGN}
    )
    configure_googletest_include(${name})
    configure_mock_include(${name})
    configure_stm32_module_include(${name})

    link_googletest_library(${name})
    link_mock_library(${name})
    link_stm32_module_library(${name})

    gtest_discover_tests(${name})
endfunction()

################################################################################
# build
################################################################################
enable_testing()

add_gtest(can_transceiver_test
        can_transceiver_test.cpp
)

add_gtest(led_controller_test
        led_controller_test.cpp
)

add_gtest(error_handler_test
        error_handler_test.cpp
)
