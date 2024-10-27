#include "tof_task.hh"

namespace coralmicro {

// Static member initialization
VL53L8CX* TofTaskInterface::sensor = nullptr;
bool TofTaskInterface::sensor_initialized = false;
I2cConfig TofTaskInterface::config;

void tof_task(void* parameters) {
    (void)parameters;

    printf("Initializing TOF sensor task...\r\n");
    
    // First run I2C scan to verify bus
    TofTaskInterface::DEBUG_scan_i2c();
    
    printf("Starting sensor initialization...\r\n");
    // Initialize the interface
    TofTaskInterface::initialize();
    
    printf("Checking if sensor is alive...\r\n");
    // Check if sensor is alive
    if (!TofTaskInterface::is_sensor_alive()) {
        printf("TOF sensor not detected. Suspending task.\r\n");
        printf("Last known I2C address: 0x%02X\r\n", VL53L8CX_DEFAULT_I2C_ADDRESS);
        vTaskSuspend(nullptr);
        return;
    }

    printf("TOF sensor detected. Starting measurements...\r\n");
    
    // Main task loop
    VL53L8CX_ResultsData results;
    uint8_t isReady;
    
    while(true) {
        // Check if new data is ready
        if (TofTaskInterface::sensor->check_data_ready(&isReady) == 0) {
            if (isReady) {
                if (TofTaskInterface::get_latest_distance(&results)) {
                    // Print central zone distance (zone 4,4 in 8x8 mode)
                    printf("Distance (center): %d mm\r\n", results.distance_mm[4*8 + 4]);
                }
            }
        }
        
        // Small delay to prevent flooding
        vTaskDelay(pdMS_TO_TICKS(50));  // Increased delay for debugging
    }
}

void TofTaskInterface::initialize() {
    printf("1. Configuring GPIO pins...\r\n");
    // Configure pins
    GpioSetMode(LpnPin, GpioMode::kOutput);
    GpioSetMode(GPIO1Pin, GpioMode::kInput);
    
    printf("2. Power cycling sensor...\r\n");
    // Start with sensor off
    sensor_off();
    vTaskDelay(pdMS_TO_TICKS(100));  // Increased delay
    
    // Turn sensor on
    sensor_on();
    vTaskDelay(pdMS_TO_TICKS(100));  // Increased delay
    
    printf("3. Initializing I2C...\r\n");
    // Initialize I2C
    config = I2cGetDefaultConfig(kI2c);
    I2cInitController(config);
    
    printf("4. Creating sensor instance...\r\n");
    // Create sensor instance
    if (!sensor) {
        sensor = new VL53L8CX(kI2c, static_cast<int>(LpnPin));
        if (!sensor) {
            printf("Failed to create sensor instance!\r\n");
            return;
        }
    }
    
    printf("5. Initializing sensor...\r\n");
    // Initialize sensor
    if (sensor) {
        int status = sensor->begin();
        if (status != 0) {
            printf("Sensor begin() failed with status: %d\r\n", status);
            return;
        }
        
        printf("6. Configuring sensor settings...\r\n");
        // Configure sensor for 8x8 mode
        status = sensor->set_resolution(VL53L8CX_RESOLUTION_8X8);
        if (status != 0) {
            printf("Failed to set resolution: %d\r\n", status);
            return;
        }
        
        status = sensor->set_ranging_frequency_hz(15);  // 15Hz refresh rate
        if (status != 0) {
            printf("Failed to set frequency: %d\r\n", status);
            return;
        }
        
        printf("7. Starting ranging...\r\n");
        // Start ranging
        status = sensor->start_ranging();
        if (status != 0) {
            printf("Failed to start ranging: %d\r\n", status);
            return;
        }
        
        sensor_initialized = true;
        printf("Sensor initialization complete!\r\n");
    }
}

bool TofTaskInterface::get_latest_distance(VL53L8CX_ResultsData* results) {
    if (!sensor || !sensor_initialized) {
        printf("Sensor not ready for ranging\r\n");
        return false;
    }
    
    uint8_t status = sensor->get_ranging_data(results);
    if (status != 0) {
        printf("Error getting ranging data: %d\r\n", status);
        return false;
    }
    return true;
}

void TofTaskInterface::DEBUG_scan_i2c() {
    printf("Starting I2C scan...\r\n");
    bool found_any = false;

    for (uint8_t address = 1; address < 128; address++) {
        if (I2cControllerWrite(config, address, nullptr, 0)) {
            printf("Device found at address 0x%02X\r\n", address);
            found_any = true;
        }
    }

    if (!found_any) {
        printf("No I2C devices found!\r\n");
    }
    printf("I2C scan complete.\r\n");
}

bool TofTaskInterface::is_sensor_alive() {
    if (!sensor) {
        printf("Sensor instance is null!\r\n");
        return false;
    }
    
    uint8_t is_alive = 0;
    uint8_t status = sensor->is_alive(&is_alive);
    
    if (status != 0) {
        printf("Error checking if sensor is alive. Status: %d\r\n", status);
        return false;
    }
    
    printf("Sensor alive check returned: %d\r\n", is_alive);
    return (is_alive == 1);
}

void TofTaskInterface::sensor_off() {
    printf("Setting LPn pin LOW (sensor off)\r\n");
    GpioSet(LpnPin, false);  // Active low
}

void TofTaskInterface::sensor_on() {
    printf("Setting LPn pin HIGH (sensor on)\r\n");
    GpioSet(LpnPin, true);
}

} // namespace coralmicro