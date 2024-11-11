// tof_task.hh
#pragma once

// Coral Micro
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "libs/base/i2c.h"
#include "libs/base/gpio.h"

// VL53L8CX implementation
#include "vl53l8cx_api.h"
#include "platform.hpp"

// C++ standard library
#include <stdio.h>
#include <memory>

namespace coralmicro {
    bool init_gpio();
    void tof_task(void* parameters);

    const char* getErrorString(uint8_t status);
    void printSensorError(const char* operation, uint8_t status);

    static constexpr Gpio kLpnPin = Gpio::kPwm0;
    static constexpr I2c kI2c = I2c::kI2c1;
    
    // Add configuration constants
    static constexpr uint8_t kResolution = VL53L8CX_RESOLUTION_8X8;
    static constexpr uint8_t kRangingFrequency = 15; // Hz
    static constexpr uint8_t kIntegrationTime = 10;  // ms
}
