#include "libs/base/led.h"
#include "libs/base/console_m7.h"
#include "libs/VL53L8CX/src/vl53l8cx.h"
#include "pin_mapping_for_coral_to_vl53l8.hpp"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

void BlinkLED(int times) {
  for (int i = 0; i < times; i++) {
    coralmicro::LedSet(coralmicro::Led::kStatus, true);
    vTaskDelay(pdMS_TO_TICKS(100));
    coralmicro::LedSet(coralmicro::Led::kStatus, false);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  vTaskDelay(pdMS_TO_TICKS(500));  // Pause between blink sequences
}

extern "C" [[noreturn]] void app_main(void* param) {
  (void)param;
  
  auto& console = *coralmicro::ConsoleM7::GetSingleton();
  console.Write("Starting VL53L8CX test...\r\n", 28);
  
  VL53L8CX sensor(PinMappingForCoralToVL53L8::kVL53L8CXI2cBus, 
                  PinMappingForCoralToVL53L8::kVL53L8CXLpnPin,
                  PinMappingForCoralToVL53L8::kVL53L8CXPwrenPin);
  
  int status = sensor.begin();
  if (status != 0) {
    console.Write("Sensor begin failed\r\n", 21);
    while(true) {
      BlinkLED(5);
    }
  }
  
  console.Write("Sensor initialized. Starting ranging...\r\n", 43);
  
  status = sensor.start_ranging();
  if (status != 0) {
    char buffer[100];
    int len = snprintf(buffer, sizeof(buffer), "Failed to start ranging. Status: %d\r\n", status);
    console.Write(buffer, len);
    while(true) {
      BlinkLED(3);
    }
  }
  
  console.Write("Ranging started. Entering main loop...\r\n", 43);
  
  while (true) {
    uint8_t isReady = 0;
    status = sensor.check_data_ready(&isReady);
    
    if (status == 0 && isReady) {
      VL53L8CX_ResultsData results;
      status = sensor.get_ranging_data(&results);
      
      if (status == 0) {
        char buffer[100];
        int len = snprintf(buffer, sizeof(buffer), "Distances: %d, %d, %d, %d\r\n", 
                           results.distance_mm[0], results.distance_mm[1],
                           results.distance_mm[2], results.distance_mm[3]);
        console.Write(buffer, len);
      } else {
        char buffer[100];
        int len = snprintf(buffer, sizeof(buffer), "Failed to get ranging data. Status: %d\r\n", status);
        console.Write(buffer, len);
      }
    } else if (status != 0) {
      char buffer[100];
      int len = snprintf(buffer, sizeof(buffer), "Failed to check data ready. Status: %d\r\n", status);
      console.Write(buffer, len);
    }
    
    BlinkLED(1);
    vTaskDelay(pdMS_TO_TICKS(1000));  // 1 second delay
  }
}