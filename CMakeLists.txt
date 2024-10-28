cmake_minimum_required(VERSION 3.16)

# Project name
project(coral_in_tree_VL53L8_i2c)

# Enable IPO/LTO if available
include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)
if(supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO/LTO not supported: ${error}")
endif()

# Define paths for task configuration
set(TASK_CONFIG_YAML "${CMAKE_CURRENT_SOURCE_DIR}/config/tasks_config.yaml")
set(TASK_CONFIG_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/task_config.hh")
set(TASK_CONFIG_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/task_config.cc")
set(TASK_GENERATOR_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_tasks.py")

# Define task source files
set(TASK_SOURCES
    src/tof_task.cc
)

# Common ARM architecture and FPU flags
set(ARM_ARCHITECTURE_FLAGS
    -mcpu=cortex-m7
    -mthumb
    -mfpu=fpv5-d16
    -mfloat-abi=hard
)

# Compiler flags optimization
set(COMPILER_OPTIMIZATION_FLAGS
    ${ARM_ARCHITECTURE_FLAGS}
    -Os                     # Optimize for size
    -ffunction-sections     # Place each function in its own section
    -fdata-sections         # Place each data item in its own section
    -fno-exceptions        # Disable exception handling
    -fno-rtti             # Disable RTTI
    -g0                   # Disable debug info
    -ffast-math           # Optimize floating-point calculations
    -fshort-enums        # Use smallest possible enum type
    -fno-unwind-tables   # Disable generation of unwind tables
    -fno-asynchronous-unwind-tables
    -Wall                # Enable all warnings
    -Wextra              # Enable extra warnings
)

# Linker flags optimization
set(LINKER_OPTIMIZATION_FLAGS
    ${ARM_ARCHITECTURE_FLAGS}
    -Wl,-Map=output.map
    -Wl,--print-memory-usage
    -Wl,--gc-sections    # Garbage collect unused sections
    -Wl,--sort-section=alignment # Sort sections by alignment
    -Wl,--cref          # Output cross reference table
)

# Create static library for VL53L8CX core driver
add_library(vl53l8cx_driver STATIC
    libs/VL53L8CX_ULD_driver_2.0.0/VL53L8CX_ULD_API/src/vl53l8cx_api.c
    libs/VL53L8CX_ULD_driver_2.0.0/VL53L8CX_ULD_API/src/vl53l8cx_buffers.c
)

# Include directories for the core driver
target_include_directories(vl53l8cx_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/VL53L8CX_ULD_driver_2.0.0/Platform
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/VL53L8CX_ULD_driver_2.0.0/VL53L8CX_ULD_API/inc
)

# Apply architecture flags to driver library
target_compile_options(vl53l8cx_driver
    PRIVATE
        ${ARM_ARCHITECTURE_FLAGS}
        -Os
        -ffunction-sections
        -fdata-sections
        -Wall
        -Wextra
)

# Add custom command to generate task configuration
add_custom_command(
    OUTPUT ${TASK_CONFIG_HEADER} ${TASK_CONFIG_SOURCE}
    COMMAND python3 ${TASK_GENERATOR_SCRIPT} 
            ${TASK_CONFIG_YAML} 
            ${TASK_CONFIG_HEADER} 
            ${TASK_CONFIG_SOURCE}
    DEPENDS ${TASK_CONFIG_YAML} ${TASK_GENERATOR_SCRIPT}
    COMMENT "Generating task configuration files"
    VERBATIM
)

# Create a custom target for task configuration generation
add_custom_target(${PROJECT_NAME}_generate_task_config
    DEPENDS ${TASK_CONFIG_HEADER} ${TASK_CONFIG_SOURCE}
)

# Add the executable and make it depend on task configuration
add_executable_m7(${PROJECT_NAME}
    src/main_cm7.cc
    src/VL53L8_bridge.cc
    ${TASK_CONFIG_SOURCE}
    ${TASK_SOURCES}
)

# Set include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/VL53L8CX_ULD_driver_2.0.0/Platform
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/VL53L8CX_ULD_driver_2.0.0/VL53L8CX_ULD_API/inc
)

# Add dependency on task configuration generation
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_generate_task_config)

# Apply compiler flags
target_compile_options(${PROJECT_NAME} 
    PRIVATE
        ${COMPILER_OPTIMIZATION_FLAGS}
)

# Apply linker flags
target_link_options(${PROJECT_NAME}
    PRIVATE
        ${LINKER_OPTIMIZATION_FLAGS}
)

# Disable LTO for this target due to ABI issues
set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        vl53l8cx_driver
        libs_base-m7_freertos
)