// include/tof_task.hh
#pragma once

#include "VL53L8_bridge.hh"
#include "libs/base/check.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"

namespace coralmicro {

    void tof_task(void* pvParameters);

} // namespace coralmicro