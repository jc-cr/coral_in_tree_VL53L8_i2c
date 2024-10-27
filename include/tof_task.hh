#pragma once

#include "libs/base/i2c.h"
#include "libs/base/utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "VL53L8CX/include/vl53l8cx.h"

namespace coralmicro {

// Forward declare task function as friend
class TofTaskInterface;
void tof_task(void* parameters);

// TOF task interface
class TofTaskInterface {
    // Declare task function as friend to allow access to private members
    friend void tof_task(void* parameters);
    
public:
    static void initialize();
    static bool get_latest_distance(VL53L8CX_ResultsData* results);
    static void DEBUG_scan_i2c();
    static bool is_sensor_alive();
    
    // Shutdown/wakeup methods
    static void sensor_off();
    static void sensor_on();

protected:
    // i2c config
    static I2cConfig config;
    static constexpr I2c kI2c = I2c::kI2c1;
    static constexpr int kI2cAddress = VL53L8CX_DEFAULT_I2C_ADDRESS;

    // Sensor instance
    static VL53L8CX* sensor;
    static bool sensor_initialized;

private:
    // Pins
    static constexpr Gpio LpnPin = Gpio::kUartCts;    // Pin 2 - LPn
    static constexpr Gpio GPIO1Pin = Gpio::kUartRts;  // GPIO1 pin
};

// Task function declaration 
void tof_task(void* parameters);

} // namespace coralmicro