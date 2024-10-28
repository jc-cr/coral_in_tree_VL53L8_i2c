// vl53l8cx_bridge.cc
#include "VL53L8_bridge.hh"


// Platform implementation for VL53L8CX - must be in global namespace
extern "C" {
    uint8_t VL53L8CX_RdByte(VL53L8CX_Platform* p_platform, uint16_t RegisterAddress, uint8_t* p_value) {
        auto& config = *static_cast<coralmicro::I2cConfig*>(p_platform->platform_handle);
        
        // Write register address first
        uint8_t addr_bytes[2] = {
            static_cast<uint8_t>((RegisterAddress >> 8) & 0xFF),
            static_cast<uint8_t>(RegisterAddress & 0xFF)
        };
        if (!coralmicro::I2cControllerWrite(config, p_platform->address, addr_bytes, sizeof(addr_bytes))) {
            return 1;
        }
        
        // Then read the data
        if (!coralmicro::I2cControllerRead(config, p_platform->address, p_value, 1)) {
            return 1;
        }
        
        return 0;
    }

    uint8_t VL53L8CX_WrByte(VL53L8CX_Platform* p_platform, uint16_t RegisterAddress, uint8_t value) {
        auto& config = *static_cast<coralmicro::I2cConfig*>(p_platform->platform_handle);
        
        uint8_t data[3] = {
            static_cast<uint8_t>((RegisterAddress >> 8) & 0xFF),
            static_cast<uint8_t>(RegisterAddress & 0xFF),
            value
        };
        
        return coralmicro::I2cControllerWrite(config, p_platform->address, data, sizeof(data)) ? 0 : 1;
    }

    uint8_t VL53L8CX_RdMulti(VL53L8CX_Platform* p_platform, uint16_t RegisterAddress, uint8_t* p_values, uint32_t size) {
        auto& config = *static_cast<coralmicro::I2cConfig*>(p_platform->platform_handle);
        
        // Write register address first
        uint8_t addr_bytes[2] = {
            static_cast<uint8_t>((RegisterAddress >> 8) & 0xFF),
            static_cast<uint8_t>(RegisterAddress & 0xFF)
        };
        if (!coralmicro::I2cControllerWrite(config, p_platform->address, addr_bytes, sizeof(addr_bytes))) {
            return 1;
        }
        
        // Then read the data
        if (!coralmicro::I2cControllerRead(config, p_platform->address, p_values, size)) {
            return 1;
        }
        
        return 0;
    }

    uint8_t VL53L8CX_WrMulti(VL53L8CX_Platform* p_platform, uint16_t RegisterAddress, uint8_t* p_values, uint32_t size) {
        auto& config = *static_cast<coralmicro::I2cConfig*>(p_platform->platform_handle);
        
        std::vector<uint8_t> buffer(size + 2);
        buffer[0] = static_cast<uint8_t>((RegisterAddress >> 8) & 0xFF);
        buffer[1] = static_cast<uint8_t>(RegisterAddress & 0xFF);
        memcpy(&buffer[2], p_values, size);
        
        return coralmicro::I2cControllerWrite(config, p_platform->address, buffer.data(), buffer.size()) ? 0 : 1;
    }

    uint8_t VL53L8CX_WaitMs(VL53L8CX_Platform* p_platform, uint32_t TimeMs) {
        (void)p_platform; // Unused parameter
        vTaskDelay(pdMS_TO_TICKS(TimeMs));
        return 0;
    }

    void VL53L8CX_SwapBuffer(uint8_t* buffer, uint16_t size) {
        uint32_t i, tmp;
        for(i = 0; i < size; i += 4) {
            // Handle potential size not being multiple of 4
            if(i + 4 > size) break;
            
            tmp = ((uint32_t)buffer[i] << 24) | 
                  ((uint32_t)buffer[i + 1] << 16) | 
                  ((uint32_t)buffer[i + 2] << 8) | 
                  (uint32_t)buffer[i + 3];
            
            buffer[i] = (tmp >> 24) & 0xFF;
            buffer[i + 1] = (tmp >> 16) & 0xFF;
            buffer[i + 2] = (tmp >> 8) & 0xFF;
            buffer[i + 3] = tmp & 0xFF;
        }
    }
}


namespace coralmicro {

    // Static member initialization
    I2cConfig VL53L8Bridge::i2c_config_;
    VL53L8CX_Configuration VL53L8Bridge::sensor_config_;
    bool VL53L8Bridge::initialized_ = false;

    // Helper function to convert status code to string
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


    bool VL53L8Bridge::initialize() {

        if (initialized_) return true;
        printf("Starting VL53L8 initialization...\r\n");

        // Memory addresses debug output
        printf("Firmware buffer: addr=0x%08X size=%u\r\n", 
            (unsigned int)VL53L8CX_FIRMWARE, VL53L8CX_FIRMWARE_SIZE);
        printf("Config buffer: addr=0x%08X size=%u\r\n", 
            (unsigned int)VL53L8CX_DEFAULT_CONFIGURATION, VL53L8CX_DEFAULT_CONFIGURATION_SIZE);
        fflush(stdout);

        // GPIO initialization
        printf("Configuring GPIO...\r\n");
        if (!init_gpio()) {
            printf("GPIO initialization failed\r\n");
            return false;
        }
        printf("GPIO configured successfully\r\n");


        printf("Configuring I2C...\r\n");
        if (!init_I2C()) {
            printf("I2C initialization failed\r\n");
            return false;
        }

        // Platform configuration
        printf("Setting up platform configuration...\r\n");
        if (!init_platform()) {
            printf("Platform configuration failed\r\n");
            return false;
        }
        
        printf("Validating sensor I2C communication...\r\n");
        if (!sensor_i2c_validation()) {
            printf("Sensor I2C validation failed\r\n");
            return false;
        }

        printf("Configuring sensor...\r\n");
        if (!configure_sensor()) {
            printf("Sensor configuration failed\r\n");
            return false;
        }
        
        initialized_ = true;
        return true;
    }

    bool VL53L8Bridge::init_gpio() {
        GpioSetMode(kLpnPin, GpioMode::kOutput);
        GpioSetMode(ki2cEnPin, GpioMode::kOutput);
        return true;
    }

    bool VL53L8Bridge::init_I2C() {

        // Reset I2C bus on hardware
        GpioSet(ki2cEnPin, 1);  // Turn off i2c on TOF
        // Short delay
        vTaskDelay(pdMS_TO_TICKS(5));
        GpioSet(ki2cEnPin, 0);  // Turn on i2c on TOF

        i2c_config_ = I2cGetDefaultConfig(kI2c);
        // More detailed I2C configuration
        i2c_config_.controller_config.baudRate_Hz = 400 * 1000;  // 400kHz
        
        bool success = I2cInitController(i2c_config_);
        if (!success) {
            printf("I2C controller initialization failed\r\n");
            fflush(stdout);
            return false;
        }
        
        printf("I2C initialized at %ukHz\r\n", 
            (unsigned int)(i2c_config_.controller_config.baudRate_Hz/1000));
        
        return true;
    }

    bool VL53L8Bridge::init_platform() {

        memset(&sensor_config_, 0, sizeof(VL53L8CX_Configuration));
        sensor_config_.platform.address = VL53L8CX_DEFAULT_I2C_ADDRESS >> 1;
        sensor_config_.platform.platform_handle = &i2c_config_;

        return true;
    }

    bool VL53L8Bridge::sensor_i2c_validation(){
        // Basic I2C communication test
        uint8_t is_alive = 0;
        uint8_t status = vl53l8cx_is_alive(&sensor_config_, &is_alive);
        
        if (status != VL53L8CX_STATUS_OK) {
            printSensorError("alive check", status);
            printf("Attempting deeper diagnostic...\r\n");
            
            // Try reading device ID registers
            uint8_t device_id = 0, revision_id = 0;
            status = VL53L8CX_WrByte(&sensor_config_.platform, 0x7fff, 0x00);
            if (status != VL53L8CX_STATUS_OK) {
                printSensorError("writing page select", status);
                return false;
            }
            
            status = VL53L8CX_RdByte(&sensor_config_.platform, 0x00, &device_id);
            if (status == VL53L8CX_STATUS_OK) {
                status = VL53L8CX_RdByte(&sensor_config_.platform, 0x01, &revision_id);
            }
            
            if (status == VL53L8CX_STATUS_OK) {
                printf("Device ID: 0x%02X, Revision ID: 0x%02X\r\n", device_id, revision_id);
                if (device_id != 0xF0 || revision_id != 0x0C) {
                    printf("Warning: Unexpected device/revision ID\r\n");
                }
            } else {
                printSensorError("reading device ID", status);
                return false;
            }
        } 
        else if (!is_alive) {
            printf("Error: Sensor reports not alive despite successful communication\r\n");
            return false;
        }
    
        return true;
    }

    bool VL53L8Bridge::configure_sensor() {

        printf("Starting sensor initialization...\r\n");

        auto status = VL53L8CX_STATUS_OK;

        // Store current task handle for priority management
        TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
        UBaseType_t originalPriority = uxTaskPriorityGet(currentTask);
        
        // Boost priority for timing-sensitive operations
        vTaskPrioritySet(currentTask, configMAX_PRIORITIES - 1);
        
        status = vl53l8cx_init(&sensor_config_);
        
        // Restore original priority
        vTaskPrioritySet(currentTask, originalPriority);

        if (status != VL53L8CX_STATUS_OK) {
            printSensorError("initialization", status);
            
            // Additional diagnostic information
            uint8_t status_regs[8];
            if (VL53L8CX_RdMulti(&sensor_config_.platform, 0x0100, status_regs, sizeof(status_regs)) == 0) {
                printf("Status registers 0x0100-0x0107:");
                for (int i = 0; i < 8; i++) {
                    printf(" %02X", status_regs[i]);
                }
                printf("\r\n");
            }
            return false;
        }

        // Configure resolution
        printf("Setting sensor resolution...\r\n");
        status = vl53l8cx_set_resolution(&sensor_config_, VL53L8CX_RESOLUTION_4X4);
        if (status != VL53L8CX_STATUS_OK) {
            printSensorError("setting resolution", status);
            return false;
        }

        // Set ranging frequency
        printf("Configuring ranging frequency...\r\n");
        status = vl53l8cx_set_ranging_frequency_hz(&sensor_config_, 1);
        if (status != VL53L8CX_STATUS_OK) {
            printSensorError("setting ranging frequency", status);
            return false;
        }

        return true;
    }

    bool VL53L8Bridge::start_ranging() {
        if (!initialized_) return false;
        return vl53l8cx_start_ranging(&sensor_config_) == 0;
    }

    bool VL53L8Bridge::stop_ranging() {
        if (!initialized_) return false;
        return vl53l8cx_stop_ranging(&sensor_config_) == 0;
    }

    bool VL53L8Bridge::get_latest_distance(VL53L8CX_ResultsData* results) {
        if (!initialized_) return false;
        
        uint8_t isReady;
        if (vl53l8cx_check_data_ready(&sensor_config_, &isReady)) {
            return false;
        }
        
        if (!isReady) {
            return false;
        }
        
        return vl53l8cx_get_ranging_data(&sensor_config_, results) == 0;
    }

    void VL53L8Bridge::deinitialize() {
        if (!initialized_) return;
        
        stop_ranging();
        initialized_ = false;
    }

    bool VL53L8Bridge::init_buffers() {
        // Verify first few bytes of firmware buffer
        const uint8_t* fw_buffer = VL53L8CX_FIRMWARE;
        
        // Known good first bytes from your output
        const uint8_t expected_header[] = {0xE0, 0x00, 0x03, 0x08};
        bool valid = true;
        
        printf("Firmware buffer contents:\r\n");
        for(int i = 0; i < 16; i++) {
            printf("%02X ", fw_buffer[i]);
        }
        printf("\r\n");
        fflush(stdout);
        
        // Check against known header
        for(int i = 0; i < sizeof(expected_header); i++) {
            if(fw_buffer[i] != expected_header[i]) {
                valid = false;
                printf("Header mismatch at %d: expected %02X, got %02X\r\n", 
                    i, expected_header[i], fw_buffer[i]);
            }
        }
        
        if (!valid) {
            printf("Error: Firmware header verification failed\r\n");
        } else {
            printf("Buffer verification passed\r\n");
        }
        fflush(stdout);
        
        return valid;
    }

} // namespace coralmicro
