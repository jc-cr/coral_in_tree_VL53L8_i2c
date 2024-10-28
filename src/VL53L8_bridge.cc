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
        if (!coralmicro::I2cControllerWrite(config, p_platform->address >> 1, addr_bytes, sizeof(addr_bytes))) {
            return 1;
        }
        
        // Then read the data
        if (!coralmicro::I2cControllerRead(config, p_platform->address >> 1, p_value, 1)) {
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
        
        return coralmicro::I2cControllerWrite(config, p_platform->address >> 1, data, sizeof(data)) ? 0 : 1;
    }

    uint8_t VL53L8CX_RdMulti(VL53L8CX_Platform* p_platform, uint16_t RegisterAddress, uint8_t* p_values, uint32_t size) {
        auto& config = *static_cast<coralmicro::I2cConfig*>(p_platform->platform_handle);
        
        // Write register address first
        uint8_t addr_bytes[2] = {
            static_cast<uint8_t>((RegisterAddress >> 8) & 0xFF),
            static_cast<uint8_t>(RegisterAddress & 0xFF)
        };
        if (!coralmicro::I2cControllerWrite(config, p_platform->address >> 1, addr_bytes, sizeof(addr_bytes))) {
            return 1;
        }
        
        // Then read the data
        if (!coralmicro::I2cControllerRead(config, p_platform->address >> 1, p_values, size)) {
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
        
        return coralmicro::I2cControllerWrite(config, p_platform->address >> 1, buffer.data(), buffer.size()) ? 0 : 1;
    }

    void VL53L8CX_SwapBuffer(uint8_t* buffer, uint16_t size) {
        uint32_t i, tmp;
        for(i = 0; i < size; i = i + 4) {
            tmp = (buffer[i]<<24) | (buffer[i+1]<<16) | (buffer[i+2]<<8) | (buffer[i+3]);
            memcpy(&buffer[i], &tmp, 4);
        }
    }

    uint8_t VL53L8CX_WaitMs(VL53L8CX_Platform* p_platform, uint32_t TimeMs) {
        (void)p_platform;
        vTaskDelay(pdMS_TO_TICKS(TimeMs));
        return 0;
    }

} // extern "C"



namespace coralmicro {

// Static member initialization
I2cConfig VL53L8Bridge::i2c_config_;
VL53L8CX_Configuration VL53L8Bridge::sensor_config_;
bool VL53L8Bridge::initialized_ = false;

bool VL53L8Bridge::initialize() {
    if (initialized_) return true;

    // Ensure buffers are properly aligned
    assert(((uintptr_t)VL53L8CX_FIRMWARE & 0x1F) == 0);
    assert(((uintptr_t)VL53L8CX_DEFAULT_CONFIGURATION & 0x1F) == 0);
    
    // Initialize GPIO first
    GpioSetMode(kLpnPin, GpioMode::kOutput);
    GpioSet(kLpnPin, false);
    vTaskDelay(pdMS_TO_TICKS(10));
    GpioSet(kLpnPin, true);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Then I2C
    if (!init_I2C()) {
        return false;
    }

    // Setup platform after I2C is ready
    sensor_config_.platform.address = 0x52;
    sensor_config_.platform.platform_handle = &i2c_config_;

    return finalize_initialization();
}

bool VL53L8Bridge::finalize_initialization() {
    // Check sensor responsiveness
    uint8_t is_alive = 0;
    for (int i = 0; i < 3; i++) {
        if (vl53l8cx_is_alive(&sensor_config_, &is_alive) == 0 && is_alive) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        if (i == 2) return false;
    }

    if (vl53l8cx_init(&sensor_config_) || 
        vl53l8cx_set_resolution(&sensor_config_, VL53L8CX_RESOLUTION_4X4) ||
        vl53l8cx_set_ranging_frequency_hz(&sensor_config_, 1)) {
        return false;
    }

    initialized_ = true;
    return true;
}

bool VL53L8Bridge::init_I2C() {
    printf("I2C configuration starting\r\n");
    fflush(stdout);
    
    i2c_config_ = I2cGetDefaultConfig(kI2c);
    bool success = I2cInitController(i2c_config_);
    
    printf("I2C initialization %s\r\n", success ? "successful" : "failed");
    fflush(stdout);
    return success;
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

} // namespace coralmicro
