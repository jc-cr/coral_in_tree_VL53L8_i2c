#include "task_config.hh"
#include "logo.hh"

#include "libs/base/led.h"
#include "libs/base/gpio.h"

namespace coralmicro {
namespace {

    const char* PROJECT_NAME = "VL53L8 i2c";

    void setup_usr_button() {
        // Setup the usr button as an interrupt to allow for transition to SDP mode
        GpioConfigureInterrupt(
            Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
            [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
            /*debounce_interval_us=*/50 * 1e3);
    }

    void setup_tasks() {
        // Create all tasks from configuration
        // These files should be automatically generated when running CMakeLists.txt
        TaskErr_t ret = CreateAllTasks();
        if (ret != TaskErr_t::OK) {
            printf("Failed to generated all tasks\r\n");
            
            // Suspend main task
            vTaskSuspend(nullptr);
        }
        else {
            printf("All tasks generated successfully\r\n");
        }
    }
    
    [[noreturn]] void Main() {
        printf("\n%s\n", PROJECT_NAME);
        printf("Developed by JC \n");
        printf("%s\n", PROJECT_LOGO);

        // Turn on Status LED to show the board is on
        LedSet(Led::kStatus, true);

        setup_usr_button();

        setup_tasks();

        printf("Starting %s Tasks\n", PROJECT_NAME);

        // Main loop
        while (true) {
            taskYIELD();
        }
    }

} // namespace
} // namespace coralmicro

extern "C" void app_main(void* param) {
    (void)param;
    coralmicro::Main();

    // Should never reach here
    vTaskSuspend(nullptr);
}