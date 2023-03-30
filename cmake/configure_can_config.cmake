################################################################################
# setting
################################################################################
set(CAN_CONFIG_PATH ${PROJECT_SOURCE_DIR}/lib/nturt_can_config)

# buiild for normal use case
set(FOR_ROS2 OFF)

################################################################################
# build
################################################################################
add_subdirectory(${CAN_CONFIG_PATH})

################################################################################
# function
################################################################################
# function to link library from can config
function(link_can_config_library name)
    target_link_libraries(${name}
        nturt_can_config
    )
    add_dependencies(${name}
        nturt_can_config
    )
endfunction()

# function for configureing include directories from can config headers
function(configure_can_config_include name)
    target_include_directories(${name} PUBLIC
        ${CAN_CONFIG_PATH}/generated_code/nturt_can_config/include
    )
endfunction()
