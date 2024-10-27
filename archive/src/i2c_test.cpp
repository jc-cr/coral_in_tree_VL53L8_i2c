#include <cstdio>
#include "libs/base/i2c.h"
#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/base/console_m7.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "pin_mapping_for_coral_to_vl53l8.hpp"

namespace coralmicro {
namespace {

void SetupPins() 
{
  // Setup LPn pin
  GpioSetMode(PinMappingForCoralToVL53L8::kVL53L8CXLpnPin, GpioMode::kOutput);
  GpioSet(PinMappingForCoralToVL53L8::kVL53L8CXLpnPin, true);  // Set high to enable the sensor

  // Setup PWREN pin
  GpioSetMode(PinMappingForCoralToVL53L8::kVL53L8CXPwrenPin, GpioMode::kOutput);
  GpioSet(PinMappingForCoralToVL53L8::kVL53L8CXPwrenPin, true);  // Set high to power on the sensor

  // Give the sensor some time to power up
  vTaskDelay(pdMS_TO_TICKS(10));
}

void ScanI2C() 
{
  auto& console = *ConsoleM7::GetSingleton();
  console.Write("Starting I2C scan...\r\n", 23);

  auto config = I2cGetDefaultConfig(PinMappingForCoralToVL53L8::kVL53L8CXI2cBus);
  I2cInitController(config);

  for (uint8_t address = 1; address < 128; address++) 
  {
    if (I2cControllerWrite(config, address, nullptr, 0)) {
      char buffer[50];
      int len = snprintf(buffer, sizeof(buffer), "Device found at address 0x%02X\r\n", address);
      console.Write(buffer, len);
    }
  }

  console.Write("I2C scan complete.\r\n", 21);
}

void Main() 
{
  LedSet(Led::kStatus, true);
  SetupPins();
  while (true) 
  {
    ScanI2C();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) 
{
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}