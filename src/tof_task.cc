#include "tof_task.hh"

namespace coralmicro {
    void tof_task(void* parameters) {
        (void)parameters;
        
        vTaskDelay(pdMS_TO_TICKS(500));  // Initial delay
        
        printf("TOF task starting sensor init...\r\n");
        fflush(stdout);
        
        if (!VL53L8Bridge::initialize()) {
            printf("Failed to initialize TOF sensor. Suspending task.\r\n");
            fflush(stdout);
            vTaskSuspend(nullptr);
            return;
        }
            
        printf("TOF sensor initialized successfully\r\n");
        fflush(stdout);

        if (!VL53L8Bridge::start_ranging()) { 
            printf("Failed to start ranging. Suspending task.\r\n");
            fflush(stdout);
            vTaskSuspend(nullptr);
            return;
        }

        VL53L8CX_ResultsData results;
        while (true) {
            if (VL53L8Bridge::get_latest_distance(&results)) {
                printf("Distance (center): %d mm\r\n", 
                    results.distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * 10]);
                fflush(stdout);
            }
            vTaskDelay(pdMS_TO_TICKS(33));  // ~30Hz update rate
        }
    }
}