#include "tof_task.hh"

namespace coralmicro {

bool init_gpio() {
    printf("GPIO Power-on sequence starting...\r\n");
    
    // Configure GPIO pins using constants from header
    GpioSetMode(kLpnPin, GpioMode::kOutput);
    GpioSetMode(ki2cEnPin, GpioMode::kOutput);

    // Initial state - everything off
    GpioSet(ki2cEnPin, 1);  // I2C disabled
    vTaskDelay(pdMS_TO_TICKS(10));

    // Power-on sequence
    printf("  1. Enabling I2C...\r\n");
    GpioSet(ki2cEnPin, 0);  // Enable I2C
    vTaskDelay(pdMS_TO_TICKS(10));

    printf("  2. Setting LPn high...\r\n");
    GpioSet(kLpnPin, 1);    // Enable sensor
    vTaskDelay(pdMS_TO_TICKS(100)); // Longer delay for power stabilization
    
    printf("GPIO initialization complete\r\n");
    return true;
}

const char* getErrorString(uint8_t status) {
    switch(status) {
        case VL53L8CX_STATUS_OK:
            return "No error";
        case VL53L8CX_STATUS_INVALID_PARAM:
            return "Invalid parameter (wrong resolution, frequency, etc)";
        case VL53L8CX_STATUS_ERROR:
            return "Major error (I2C/SPI timeout)";
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

void tof_task(void* parameters) {
    (void)parameters;
    
    vTaskDelay(pdMS_TO_TICKS(500));  // Initial delay
    
    printf("TOF task starting sensor init...\r\n");

    // Initialize GPIO first
    printf("Configuring GPIO...\r\n");
    if (!init_gpio()) {
        printf("GPIO initialization failed\r\n");
        return;
    }

    // Initialize VL53L8CX sensor
    VL53L8CX_Configuration dev;
    VL53L8CX_Platform platform;
    uint8_t status = 0;

    // Initialize platform - using the function as declared in platform.h
    if (!VL53L8CX_PlatformInit(&platform, kI2c)) {
        printf("Error: Failed to initialize platform\r\n");
        return;
    }

    // Store platform reference
    dev.platform = platform;

    // Initialize sensor
    status = vl53l8cx_init(&dev);
    if (status) {
        printSensorError("sensor initialization", status);
        return;
    }

    // Configure sensor settings
    status = vl53l8cx_set_resolution(&dev, VL53L8CX_RESOLUTION_8X8);
    if (status) {
        printSensorError("setting resolution", status);
        return;
    }

    // Set ranging frequency to 10Hz
    status = vl53l8cx_set_ranging_frequency_hz(&dev, 10);
    if (status) {
        printSensorError("setting ranging frequency", status);
        return;
    }

    // Set ranging mode (continuous or autonomous)
    status = vl53l8cx_set_ranging_mode(&dev, VL53L8CX_RANGING_MODE_CONTINUOUS);
    if (status) {
        printSensorError("setting ranging mode", status);
        return;
    }

    // Start ranging
    printf("Starting ranging...\r\n");
    status = vl53l8cx_start_ranging(&dev);
    if (status) {
        printSensorError("starting ranging", status);
        return;
    }

    // Results data structure
    VL53L8CX_ResultsData results;
    
    // Main task loop
    while (true) {
        uint8_t isReady;
        
        // Check if new data is ready
        status = vl53l8cx_check_data_ready(&dev, &isReady);
        
        if (!status && isReady) {
            // Get ranging data
            status = vl53l8cx_get_ranging_data(&dev, &results);

            if (!status) {
                // Print some results (customize based on your needs)
                printf("\r\nFrame: %u\r\n", dev.streamcount);
                
                // Example: Print distance for each zone
                for (uint8_t i = 0; i < VL53L8CX_RESOLUTION_8X8; i++) {
                    // Only print if target detected
                    if (results.nb_target_detected[i] > 0) {
                        printf("Zone %d: ", i);
                        printf("Distance %d mm, ", results.distance_mm[i]);
                        printf("Status %d\r\n", results.target_status[i]);
                    }
                }
            } else {
                printSensorError("getting ranging data", status);
            }
        }
        
        // Add small delay to prevent task starvation
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

} // namespace coralmicro