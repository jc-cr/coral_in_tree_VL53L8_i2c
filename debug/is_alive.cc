
#include <memory>
#include <vector>
#include <cstdio>
#include "libs/base/check.h"
#include "libs/base/i2c.h"
#include "libs/base/led.h"
#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {
namespace {

static constexpr Gpio kLpnPin = Gpio::kPwm0;
static constexpr I2c kI2c = I2c::kI2c1;
static constexpr uint8_t kSensorAddr = 0x29;  // Using 7-bit address

static I2cConfig g_i2c_config;


// Helper function to scan I2C bus
void ScanI2C() {
    printf("\nScanning I2C bus...\r\n");
    printf("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n");
    
    uint8_t dummy_data = 0;
    
    for (int baseAddr = 0; baseAddr < 128; baseAddr += 16) {
        printf("%02X: ", baseAddr);
        
        for (int addr = 0; addr < 16; addr++) {
            int currentAddr = baseAddr + addr;
            
            // Try to read a byte from the device
            bool success = I2cControllerRead(g_i2c_config, currentAddr, &dummy_data, 1);
            
            if (success) {
                printf("%02X ", currentAddr);
            } else {
                printf("-- ");
            }
        }
        printf("\r\n");
    }
    printf("\nScan complete!\r\n");
}


// Helper function to write a register with verification
bool WriteRegByte(uint16_t reg_addr, uint8_t value) {
    uint8_t write_buffer[3];
    write_buffer[0] = static_cast<uint8_t>(reg_addr >> 8);
    write_buffer[1] = static_cast<uint8_t>(reg_addr & 0xFF);
    write_buffer[2] = value;
    
    // First verify device responds to address
    uint8_t verify_buffer[1] = {0};
    if (!I2cControllerRead(g_i2c_config, kSensorAddr, verify_buffer, 1)) {
        printf("Device not responding at address 0x%02X\r\n", kSensorAddr);
        return false;
    }
    
    bool success = I2cControllerWrite(g_i2c_config, kSensorAddr, write_buffer, sizeof(write_buffer));
    if (!success) {
        printf("Failed to write register 0x%04X\r\n", reg_addr);
        return false;
    }
    
    return true;
}

// Helper function to read a register
bool ReadRegByte(uint16_t reg_addr, uint8_t* value) {
    uint8_t write_buffer[2];
    write_buffer[0] = static_cast<uint8_t>(reg_addr >> 8);
    write_buffer[1] = static_cast<uint8_t>(reg_addr & 0xFF);
    
    // First verify device responds to address
    uint8_t verify_buffer[1] = {0};
    if (!I2cControllerRead(g_i2c_config, kSensorAddr, verify_buffer, 1)) {
        printf("Device not responding at address 0x%02X\r\n", kSensorAddr);
        return false;
    }
    
    // Write register address
    if (!I2cControllerWrite(g_i2c_config, kSensorAddr, write_buffer, sizeof(write_buffer))) {
        printf("Failed to write register address 0x%04X\r\n", reg_addr);
        return false;
    }
    
    // Read the value with restart
    vTaskDelay(pdMS_TO_TICKS(1));  // Small delay before read
    if (!I2cControllerRead(g_i2c_config, kSensorAddr, value, 1)) {
        printf("Failed to read register value\r\n");
        return false;
    }
    
    return true;
}

bool TestSensorAlive() {
    printf("\nTesting VL53L8CX at address 0x%02X...\r\n", kSensorAddr);
    
    uint8_t device_id = 0;
    uint8_t revision_id = 0;
    
    // Step 1: Write 0x00 to register 0x7FFF
    printf("Writing 0x00 to reg 0x7FFF...\r\n");
    if (!WriteRegByte(0x7FFF, 0x00)) {
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(1));
    
    // Step 2: Read device ID from register 0x00
    printf("Reading device ID from reg 0x00...\r\n");
    if (!ReadRegByte(0x00, &device_id)) {
        return false;
    }
    printf("Device ID: 0x%02X\r\n", device_id);
    
    // Step 3: Read revision ID from register 0x01
    printf("Reading revision ID from reg 0x01...\r\n");
    if (!ReadRegByte(0x01, &revision_id)) {
        return false;
    }
    printf("Revision ID: 0x%02X\r\n", revision_id);
    
    // Step 4: Write 0x02 to register 0x7FFF
    printf("Writing 0x02 to reg 0x7FFF...\r\n");
    if (!WriteRegByte(0x7FFF, 0x02)) {
        return false;
    }
    
    bool is_alive = (device_id == 0xF0) && (revision_id == 0x0C);
    printf("Sensor %s (device_id=0x%02X, revision_id=0x%02X)\r\n",
           is_alive ? "identified correctly" : "not identified correctly",
           device_id, revision_id);
    
    return is_alive;
}

bool InitHardware() {
    printf("\nInitializing hardware...\r\n");
    
    // Configure GPIO
    GpioSetMode(kLpnPin, GpioMode::kOutput);
    
    // Reset sequence
    printf("Performing reset sequence...\r\n");
    GpioSet(kLpnPin, false);  // Assert reset
    vTaskDelay(pdMS_TO_TICKS(100));
    GpioSet(kLpnPin, true);   // Release reset
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Initialize I2C
    g_i2c_config = I2cGetDefaultConfig(kI2c);
    g_i2c_config.controller_config.baudRate_Hz = 400'000;  // 400 KHz
    g_i2c_config.controller_config.enableDoze = false;
    
    printf("Initializing I2C...\r\n");
    if (!I2cInitController(g_i2c_config)) {
        printf("Failed to initialize I2C\r\n");
        return false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(50));  // Startup delay
    return true;
}

void Main() {
    printf("\nVL53L8CX Debug Test Starting\r\n");
    LedSet(Led::kStatus, true);
    
    if (!InitHardware()) {
        printf("Hardware initialization failed!\r\n");
        return;
    }
    
    // Basic presence test
    uint8_t dummy;
    bool device_present = I2cControllerRead(g_i2c_config, kSensorAddr, &dummy, 1);
    printf("\nDevice %s at address 0x%02X\r\n", 
           device_present ? "responded" : "not found",
           kSensorAddr);
    
    if (device_present) {
        bool success = TestSensorAlive();
        while (true) {
            LedSet(Led::kStatus, success || (xTaskGetTickCount() % 1000 > 500));
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
    (void)param;
    coralmicro::Main();
    vTaskSuspend(nullptr);
}