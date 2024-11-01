#pragma once
#include "libs/base/gpio.h"
#include "libs/base/i2c.h"
#include "vl53l8cx_api.h"
#include "platform.h"

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {
    void tof_task(void* parameters);


    bool init_gpio();

    const char* getErrorString(uint8_t status);
    void printSensorError(const char* operation, uint8_t status);

    static constexpr Gpio ki2cEnPin = Gpio::kScl6;
    static constexpr Gpio kLpnPin = Gpio::kUartCts;
    static constexpr I2c kI2c = I2c::kI2c1;

}
