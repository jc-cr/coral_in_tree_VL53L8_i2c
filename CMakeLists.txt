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
    -fdata-sections         # Place each data item in its own section
    -fno-exceptions         # Disable exception handling
    -fno-rtti              # Disable RTTI
    -g0                    # Disable debug info
    -ffast-math            # Optimize floating-point calculations
    -fshort-enums          # Use smallest possible enum type
    -fno-unwind-tables     # Disable generation of unwind tables
    -fno-asynchronous-unwind-tables
    -Wall                  # Enable all warnings
    -Wextra                # Enable extra warnings
)

# Linker flags optimization
set(LINKER_OPTIMIZATION_FLAGS
    -Wl,-Map=output.map
    -Wl,--print-memory-usage
    -Wl,--gc-sections      # Garbage collect unused sections
    -Wl,--sort-section=alignment # Sort sections by alignment
    -Wl,--cref            # Output cross reference table
)

# Add VL53L8CX library
add_subdirectory(libs/VL53L8CX_ULD_driver_2.0.0)

# Define source files for M7
set(M7_SOURCES
    src/main_cm7.cc
    src/VL53L8_bridge.cc
)


# Add M7 executable
add_executable_m7(${PROJECT_NAME}
    ${M7_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/VL53L8CX_ULD_driver_2.0.0/Platform
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/VL53L8CX_ULD_driver_2.0.0/VL53L8CX_ULD_API/inc
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

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        vl53l8cx_driver
        libs_base-m7_freertos
)