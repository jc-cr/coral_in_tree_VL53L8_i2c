#include <memory>
#include <vector>
#include <cstdio>
#include "libs/base/i2c.h"
#include "libs/VL53L8CX/src/vl53l8cx.h"
#include "libs/base/led.h"
#include "libs/base/utils.h"
#include "libs/base/console_m7.h"
#include "libs/base/tasks.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {
namespace {

void PrintResults(const VL53L8CX_ResultsData& results) {
  char buffer[256];
  int offset = 0;

  offset += snprintf(buffer + offset, sizeof(buffer) - offset, "VL53L8CX Results:\r\n");
  for (int i = 0; i < 16; i++) {  // 4x4 = 16 zones
    if (i % 4 == 0 && i != 0) {
      offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n");
    }
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%4d ", results.distance_mm[i]);
  }
  offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\r\n\r\n");

  ConsoleM7::GetSingleton()->Write(buffer, offset);
}

void Main() 
{
  ConsoleM7::GetSingleton()->Write("VL53L8CX 4x4 Mode Example!\r\n", 30);
  LedSet(Led::kStatus, true);
  
  VL53L8CX sensor(coralmicro::I2c::kI2c1, -1);  // Assuming no LPN or I2C reset pins are used
  sensor.begin();
  uint8_t status = sensor.init();
  if (status != 0) {
    ConsoleM7::GetSingleton()->Write("Sensor initialization failed!\r\n", 32);
    return;
  }

  // Set resolution to 4x4
  status = sensor.set_resolution(VL53L8CX_RESOLUTION_4X4);
  if (status != 0) {
    ConsoleM7::GetSingleton()->Write("Failed to set resolution!\r\n", 28);
    return;
  }

  status = sensor.start_ranging();
  if (status != 0) {
    ConsoleM7::GetSingleton()->Write("Failed to start ranging!\r\n", 27);
    return;
  }

  ConsoleM7::GetSingleton()->Write("Sensor initialized and ranging started. Reading data...\r\n", 59);

  while (true)
  {
    uint8_t new_data_ready = 0;
    do {
      status = sensor.check_data_ready(&new_data_ready);
    } while (!new_data_ready);

    auto results = std::make_unique<VL53L8CX_ResultsData>();
    if ((!status) && (new_data_ready != 0)) {
      status = sensor.get_ranging_data(results.get());
      if (status == 0) {
        PrintResults(*results);
      } else {
        ConsoleM7::GetSingleton()->Write("Failed to get ranging data!\r\n", 30);
      }
    }

    // Add a delay to avoid flooding the console
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}