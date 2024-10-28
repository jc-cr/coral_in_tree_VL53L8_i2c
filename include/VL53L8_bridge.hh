#pragma once

#include "libs/base/i2c.h"
#include "libs/base/utils.h"
#include "libs/base/gpio.h"
#include "libs/base/check.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "vl53l8cx_api.h"

#include <stdint.h>
#include <string.h>

// Define section attribute if not already defined
#ifndef VL53L8CX_SDRAM_DATA
#define VL53L8CX_SDRAM_DATA __attribute__((section(".sdram_data")))
#endif

namespace coralmicro {

    class VL53L8Bridge {
    public:
        static bool initialize();
        static bool start_ranging();
        static bool stop_ranging();
        static bool get_latest_distance(VL53L8CX_ResultsData* results);
        static void deinitialize();

    private:
        static I2cConfig i2c_config_;
        static VL53L8CX_Configuration sensor_config_;
        static bool initialized_;
        
        // Pin definitions
        static constexpr Gpio kLpnPin = Gpio::kUartCts;
        static constexpr Gpio kGPIO1Pin = Gpio::kUartRts;
        static constexpr I2c kI2c = I2c::kI2c1;
        
        static bool init_I2C();
        static bool configure_sensor();
    };

} // namespace coralmicro