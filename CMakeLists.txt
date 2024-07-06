set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -flto -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -g0 -Wall -Wextra")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=output.map -Wl,--gc-sections -Wl,--print-memory-usage")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)


add_executable_m7(coral_in_tree_VL53L8_i2c
    src/main.cpp
)

target_compile_options(coral_in_tree_VL53L8_i2c PRIVATE
    -fno-exceptions
    -fno-rtti
)

target_link_libraries(coral_in_tree_VL53L8_i2c
    libs_base-m7_freertos
    libs_vl53l8cx
)