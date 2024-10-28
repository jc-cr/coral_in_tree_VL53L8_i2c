#pragma once

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "VL53L8_bridge.hh"

namespace coralmicro {
    // Increased stack size for firmware loading
    constexpr uint32_t kTofTaskStackSize = 8192;
    constexpr uint8_t kTofTaskPriority = 5;

    void tof_task(void* parameters);
}
