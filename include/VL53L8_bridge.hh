// VL53L8_bridge.hh
#pragma once

#include "libs/base/i2c.h"
#include "libs/base/utils.h"
#include "libs/base/gpio.h"
#include "libs/base/check.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include "vl53l8cx_api.h"
#include "vl53l8cx_buffers.h"


#include <stdint.h>
#include <string.h>
namespace coralmicro {

class VL53L8Bridge {
public:
    static bool initialize();
    static bool finalize_initialization();
    static bool start_ranging();
    static bool stop_ranging();
    static bool get_latest_distance(VL53L8CX_ResultsData* results);
    static void deinitialize();

private:
    static I2cConfig i2c_config_;
    static VL53L8CX_Configuration sensor_config_;
    static bool initialized_;
    
    static constexpr Gpio kLpnPin = Gpio::kUartCts;
    static constexpr I2c kI2c = I2c::kI2c1;
    
    static bool init_I2C();
    static bool configure_sensor();
    static bool init_buffers();
};

} // namespace coralmicro