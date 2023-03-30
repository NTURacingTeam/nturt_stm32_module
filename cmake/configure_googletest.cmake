################################################################################
# setting
################################################################################
# library path
set(GOOGLETEST_PATH ${PROJECT_SOURCE_DIR}/lib/googletest)
set(CMOCK_PATH ${PROJECT_SOURCE_DIR}/lib/C-Mock)

# disable install
set(INSTALL_GTEST OFF)

################################################################################
# build
################################################################################
add_subdirectory(${GOOGLETEST_PATH})
# don't need to build CMock as it's header only

################################################################################
# function
################################################################################
# function for configureing include directories from google test
function(configure_googletest_include name)
    target_include_directories(${name} PUBLIC
        ${CMOCK_PATH}/include
        ${GOOGLETEST_PATH}/googlemock/include
        ${GOOGLETEST_PATH}/googletest/include
    )
endfunction()

# function to link library from google test
function(link_googletest_library name)
    target_link_libraries(${name}
        gmock
        gtest
        GTest::gtest_main
    )
    add_dependencies(${name}
        gmock
        gtest
        GTest::gtest_main
    )
endfunction()
