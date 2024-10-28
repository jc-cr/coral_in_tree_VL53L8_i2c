#include "tof_task.hh"
#include "libs/base/check.h"

namespace coralmicro {

void tof_task(void* parameters) {
    (void)parameters;

    if (!VL53L8Bridge::initialize()) { 
        printf("Failed to initialize TOF sensor. Suspending task.\n");
        vTaskSuspend(nullptr);
        return;
    }

    if (!VL53L8Bridge::start_ranging()) { 
        printf("Failed to start ranging. Suspending task.\n");
        vTaskSuspend(nullptr);
        return;
    }

    VL53L8CX_ResultsData results;
    
    while (true) {
        if (VL53L8Bridge::get_latest_distance(&results)) {
            // Print central zone distance (zone 10 in 4x4 configuration)
            printf("Distance (center): %d mm\n", 
                results.distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * 10]);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

}  // namespace coralmicro