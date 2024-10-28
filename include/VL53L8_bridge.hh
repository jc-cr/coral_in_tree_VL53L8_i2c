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

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// Platform implementation functions
uint8_t VL53L8CX_RdByte(VL53L8CX_Platform* p_platform, uint16_t RegisterAddress, uint8_t* p_value);
uint8_t VL53L8CX_WrByte(VL53L8CX_Platform* p_platform, uint16_t RegisterAddress, uint8_t value);
uint8_t VL53L8CX_RdMulti(VL53L8CX_Platform* p_platform, uint16_t RegisterAddress, uint8_t* p_values, uint32_t size);
uint8_t VL53L8CX_WrMulti(VL53L8CX_Platform* p_platform, uint16_t RegisterAddress, uint8_t* p_values, uint32_t size);
uint8_t VL53L8CX_WaitMs(VL53L8CX_Platform* p_platform, uint32_t TimeMs);
void VL53L8CX_SwapBuffer(uint8_t* buffer, uint16_t size);

#ifdef __cplusplus
}
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
    
    static constexpr Gpio ki2cEnPin = Gpio::kScl6;
    static constexpr Gpio kLpnPin = Gpio::kUartCts;
    static constexpr I2c kI2c = I2c::kI2c1;
    
    static bool init_I2C();
    static bool init_gpio();
    static bool init_platform();
    static bool sensor_i2c_validation();
    static bool configure_sensor();

    static bool init_buffers();
};

} // namespace coralmicro