// main_cm7.cc
#include "VL53L8_bridge.hh"
#include "libs/base/led.h"

namespace coralmicro {
namespace {

[[noreturn]] void Main() {
    printf("Starting VL53L8 example...\r\n");
    fflush(stdout);
    
    // Turn on Status LED
    LedSet(Led::kStatus, true);
    
        
        printf("TOF task created and starting...\r\n");
        fflush(stdout);
        
        vTaskDelay(pdMS_TO_TICKS(100));  // Add delay between prints
        
        printf("TOF task starting sensor init...\r\n");
        fflush(stdout);
        
        bool init_success = VL53L8Bridge::initialize();
        
        if (!init_success) {
            printf("Failed to initialize TOF sensor. Suspending task.\r\n");
            fflush(stdout);
            vTaskDelay(pdMS_TO_TICKS(100));  // Add delay before suspend
            vTaskSuspend(nullptr);
            return;
        }
            
        printf("TOF sensor initialized successfully\r\n");
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(100));

        if (!VL53L8Bridge::start_ranging()) { 
            printf("Failed to start ranging. Suspending task.\r\n");
            fflush(stdout);
            vTaskDelay(pdMS_TO_TICKS(100));
            vTaskSuspend(nullptr);
            return;
        }

        printf("TOF ranging started\r\n");
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(100));

        VL53L8CX_ResultsData results;
        const TickType_t xDelay = pdMS_TO_TICKS(33);
        
        printf("Entering TOF task loop\r\n");
        fflush(stdout);

        while (true) {
            if (VL53L8Bridge::get_latest_distance(&results)) {
                printf("Distance (center): %d mm\r\n", 
                    results.distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * 10]);
                fflush(stdout);
            }
            vTaskDelay(xDelay);
        }
    }

} // namespace
} // namespace coralmicro

extern "C" void app_main(void* param) {
    (void)param;
    coralmicro::Main();
}