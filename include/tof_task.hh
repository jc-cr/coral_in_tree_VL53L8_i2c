// include/tof_task.hh
#pragma once

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "VL53L8_bridge.hh"

namespace coralmicro {

    void tof_task(void* pvParameters);

} // namespace coralmicro