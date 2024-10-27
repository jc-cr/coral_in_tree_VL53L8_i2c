# Require a minimum CMake version
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

# Compiler flags optimization
set(COMPILER_OPTIMIZATION_FLAGS
    -Os                     # Optimize for size
    -flto                   # Link Time Optimization
    -ffunction-sections     # Place each function in its own section
    -fdata-sections        # Place each data item in its own section
    -fno-exceptions        # Disable exception handling
    -fno-rtti             # Disable RTTI
    -g0                   # Disable debug info
    -ffast-math           # Optimize floating-point calculations
    -fshort-enums        # Use smallest possible enum type
    -fno-unwind-tables   # Disable generation of unwind tables
    -fno-asynchronous-unwind-tables
    -Wall                # Enable all warnings
    -Wextra              # Enable extra warnings
    #-Werror              # Treat warnings as errors
)

# Linker flags optimization
set(LINKER_OPTIMIZATION_FLAGS
    -Wl,-Map=output.map
    -Wl,--gc-sections    # Garbage collect unused sections
    -Wl,--print-memory-usage
    -Wl,--sort-section=alignment # Sort sections by alignment
    -Wl,--cref          # Output cross reference table
)

# Define paths for task configuration
set(TASK_CONFIG_YAML "${CMAKE_CURRENT_SOURCE_DIR}/config/tasks_config.yaml")
set(TASK_CONFIG_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/task_config.hh")
set(TASK_CONFIG_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/task_config.cc")
set(TASK_GENERATOR_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_tasks.py")

# Define task source files
set(TASK_SOURCES
    src/tof_task.cc
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
    src/main.cc

    ${TASK_CONFIG_SOURCE}
    ${TASK_SOURCES}
)

# Add dependency on task configuration generation
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_generate_task_config
)

# Set include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        libs_base-m7_freertos
        libs_vl53l8cx
)