#include <memory>
#include <vector>
#include <cstdio>

#include "libs/base/i2c.h"
#include "libs/VL53L8CX/src/vl53l8cx.h"
#include "libs/base/led.h"

#include "libs/base/utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {
namespace {

void PrintResults(VL53L8CX_ResultsData* results) {
  printf("VL53L8CX Results:\n");
  for (int i = 0; i < 64; i++) {  // 8x8 = 64 zones
    if (i % 8 == 0) printf("\n");
    printf("%4d ", results->distance_mm[i]);
  }
  printf("\n\n");
}


void Main() 
{
  printf("VL53L8CX 8x8 Mode Example!\r\n");
  LedSet(Led::kStatus, true);

  VL53L8CX sensor(coralmicro::I2c::kI2c1, -1);  // Assuming no LPN or I2C reset pins are used

  sensor.begin();

  uint8_t status = sensor.init();

  status = sensor.start_ranging();

  for(;;)
  {

    uint8_t new_data_ready = 0;

    do {
      status = sensor.check_data_ready(&new_data_ready);
    } while (!new_data_ready);


    auto results = std::make_unique<VL53L8CX_ResultsData>();
    if ((!status) && (new_data_ready != 0)) {
      status = sensor.get_ranging_data(results.get());
    }
  }

}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}