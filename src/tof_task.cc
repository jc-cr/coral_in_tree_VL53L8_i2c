// tof_task.cc
#include "tof_task.hh"

namespace coralmicro {

    bool init_gpio() {
        printf("GPIO Power-on sequence starting...\r\n");
        
        // Configure LPn pin
        GpioSetMode(kLpnPin, GpioMode::kOutput);
        
        // Reset sequence
        GpioSet(kLpnPin, false);  // Assert reset
        vTaskDelay(pdMS_TO_TICKS(100));  // Increased delay
        GpioSet(kLpnPin, true);   // Release reset
        vTaskDelay(pdMS_TO_TICKS(100));  // Increased delay
        
        printf("GPIO initialization complete\r\n");
        return true;
    }

    const char* getErrorString(uint8_t status) {
        switch(status) {
            case VL53L8CX_STATUS_OK:
                return "No error";
            case VL53L8CX_STATUS_INVALID_PARAM:
                return "Invalid parameter";
            case VL53L8CX_STATUS_ERROR:
                return "Major error";
            case VL53L8CX_STATUS_TIMEOUT_ERROR:
                return "Timeout error";
            case VL53L8CX_STATUS_CORRUPTED_FRAME:
                return "Corrupted frame";
            case VL53L8CX_STATUS_LASER_SAFETY:
                return "Laser safety error";
            case VL53L8CX_STATUS_XTALK_FAILED:
                return "Cross-talk calibration failed";
            case VL53L8CX_STATUS_FW_CHECKSUM_FAIL:
                return "Firmware checksum error";
            case VL53L8CX_MCU_ERROR:
                return "MCU error";
            default:
                return "Unknown error";
        }
    }

    void printSensorError(const char* operation, uint8_t status) {
        printf("Error during %s: [%d] %s\r\n", 
            operation, 
            status, 
            getErrorString(status));
        fflush(stdout);
    }

    // In tof_task.cc, modify the initializeSensor function:

    bool initializeSensor(VL53L8CX_Configuration* dev) {
        uint8_t status;
        uint8_t isAlive = 0;
        
        // Add delay after power-on
        vTaskDelay(pdMS_TO_TICKS(10));
        
        // Check if sensor is alive
        status = vl53l8cx_is_alive(dev, &isAlive);
        if (status != VL53L8CX_STATUS_OK || !isAlive) {
            printSensorError("checking sensor alive", status);
            return false;
        }
        printf("Sensor is alive\r\n");
        
        // Add delay before initialization
        vTaskDelay(pdMS_TO_TICKS(10));
        
        // Initialize sensor
        status = vl53l8cx_init(dev);
        if (status != VL53L8CX_STATUS_OK) {
            printSensorError("sensor initialization", status);
            return false;
        }
        printf("Sensor initialized\r\n");
        
        // Add delay after initialization
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // Set resolution
        status = vl53l8cx_set_resolution(dev, kResolution);
        if (status != VL53L8CX_STATUS_OK) {
            printSensorError("setting resolution", status);
            return false;
        }
        printf("Resolution set to 8x8\r\n");
        
        // Add delay between configuration steps
        vTaskDelay(pdMS_TO_TICKS(10));
        
        // Set ranging mode to continuous
        status = vl53l8cx_set_ranging_mode(dev, VL53L8CX_RANGING_MODE_CONTINUOUS);
        if (status != VL53L8CX_STATUS_OK) {
            printSensorError("setting ranging mode", status);
            return false;
        }
        printf("Ranging mode set to continuous\r\n");
        
        vTaskDelay(pdMS_TO_TICKS(10));
        
        // Set ranging frequency
        status = vl53l8cx_set_ranging_frequency_hz(dev, kRangingFrequency);
        if (status != VL53L8CX_STATUS_OK) {
            printSensorError("setting ranging frequency", status);
            return false;
        }
        printf("Ranging frequency set to %d Hz\r\n", kRangingFrequency);
        
        vTaskDelay(pdMS_TO_TICKS(10));
        
        // Set integration time
        status = vl53l8cx_set_integration_time_ms(dev, kIntegrationTime);
        if (status != VL53L8CX_STATUS_OK) {
            printSensorError("setting integration time", status);
            return false;
        }
        printf("Integration time set to %d ms\r\n", kIntegrationTime);
        
        // Final delay before returning
        vTaskDelay(pdMS_TO_TICKS(50));
        
        return true;
    }

    void printResults(VL53L8CX_ResultsData* results) {
        printf("\nSensor Temperature: %d°C\n", results->silicon_temp_degc);
        
        for (uint8_t i = 0; i < kResolution; i++) {
            if (results->nb_target_detected[i] > 0) {
                printf("Zone %2d: ", i);
                printf("Targets=%d, ", results->nb_target_detected[i]);
                printf("Distance=%4dmm, ", results->distance_mm[i]);
                printf("Status=%3d, ", results->target_status[i]);
                printf("Signal=%5d\n", results->signal_per_spad[i]);
            }
        }
    }

    void tof_task(void* parameters) {
        (void)parameters;
        uint8_t status;
        
        // Add stack checking
        #if ( configCHECK_FOR_STACK_OVERFLOW > 0 )
        volatile StackType_t *highWaterMark;
        highWaterMark = uxTaskGetStackHighWaterMark(nullptr);
        printf("Initial stack high water mark: %u words\r\n", 
            static_cast<unsigned>(highWaterMark));
        #endif
        
        printf("TOF task starting...\r\n");
        fflush(stdout);
        
        if (!init_gpio()) {
            printf("GPIO initialization failed\r\n");
            return;
        }
        
        // Platform initialization with proper cleanup
        VL53L8CX_Platform platform = {};
        if (!vl53l8cx::PlatformInit(&platform, kI2c, kAddress)) {
            printf("Platform initialization failed\r\n");
            return;
        }
        
        // Create and initialize device instance
        auto dev = std::make_unique<VL53L8CX_Configuration>();
        if (!dev) {
            printf("Failed to allocate device configuration\r\n");
            return;
        }
        
        dev->platform = platform;
        
        
        if (!initializeSensor(dev.get())) {
            printf("Sensor initialization failed - exiting task\r\n");
            return;
        }
        
        // Start ranging
        status = vl53l8cx_start_ranging(dev.get());
        if (status != VL53L8CX_STATUS_OK) {
            printSensorError("starting ranging", status);
            return;
        }
        
        printf("Ranging started successfully\r\n");
        fflush(stdout);
        
        // Allocate results structure on heap
        auto results = std::make_unique<VL53L8CX_ResultsData>();
        if (!results) {
            printf("Failed to allocate results structure\r\n");
            return;
        }
        
        // Main task loop with watchdog
        TickType_t last_wake_time = xTaskGetTickCount();
        const TickType_t frequency = pdMS_TO_TICKS(33);  // Match your YAML config
        
        while (true) {
            uint8_t isReady = 0;
            
            // Check if new data is ready
            status = vl53l8cx_check_data_ready(dev.get(), &isReady);
            
            if (status == VL53L8CX_STATUS_OK && isReady) {
                status = vl53l8cx_get_ranging_data(dev.get(), results.get());
                if (status == VL53L8CX_STATUS_OK) {
                    printResults(results.get());
                } else {
                    printSensorError("getting ranging data", status);
                }
            } else if (status != VL53L8CX_STATUS_OK) {
                printSensorError("checking data ready", status);
            }
            
            // Check stack usage periodically
            #if ( configCHECK_FOR_STACK_OVERFLOW > 0 )
            if ((xTaskGetTickCount() % pdMS_TO_TICKS(5000)) == 0) {
                highWaterMark = uxTaskGetStackHighWaterMark(nullptr);
                printf("Current stack high water mark: %u words\r\n", 
                    static_cast<unsigned>(highWaterMark));
            }
            #endif
            
            // Use vTaskDelayUntil for consistent timing
            vTaskDelayUntil(&last_wake_time, frequency);
        }
    }


} // namespace coralmicro
