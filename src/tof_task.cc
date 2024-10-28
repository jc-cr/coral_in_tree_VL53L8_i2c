#include "tof_task.hh"
#include "libs/base/check.h"

namespace coralmicro {

    void tof_task(void* parameters) {
        (void)parameters;

        // Simple initial delay
        vTaskDelay(pdMS_TO_TICKS(500));  // Longer initial delay
        
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
} // namespace coralmicro