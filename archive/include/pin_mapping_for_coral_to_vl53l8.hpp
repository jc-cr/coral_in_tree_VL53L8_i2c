// pin_mapping_for_coral_to_vl53l8.hpp
#pragma once
#include "libs/base/i2c.h"
#include "libs/base/gpio.h"

class PinMappingForCoralToVL53L8
{
    public:
        PinMappingForCoralToVL53L8() = default;
        ~PinMappingForCoralToVL53L8() = default;

        // I2C Configuration
        static constexpr coralmicro::I2c kVL53L8CXI2cBus = coralmicro::I2c::kI2c1;
        
        // Pin Configuration
        static constexpr coralmicro::Gpio kVL53L8CXLpnPin = coralmicro::Gpio::kPwm0;
        static constexpr coralmicro::Gpio kVL53L8CXPwrenPin = coralmicro::Gpio::kPwm1;
        
        // VL53L8CX Configuration
        static constexpr uint8_t kVL53L8CXAddress = 0x52;
};
