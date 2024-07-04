#include <memory>
#include <vector>

#include "libs/base/check.h"
#include "libs/base/i2c.h"
#include "libs/base/led.h"
#include "libs/base/utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Runs the board as an I2C controller device.

namespace coralmicro {
namespace {
// [start-sphinx-snippet:i2c-controller]
void Main() {
  printf("i2c Controller Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  auto config = I2cGetDefaultConfig(coralmicro::I2c::kI2c1);
  I2cInitController(config);

  std::string serial = GetSerialNumber();
  constexpr int kTargetAddress = 0x42;
  int kTransferSize = serial.length();

  printf("Writing our serial number to the remote device...\r\n");
  CHECK(I2cControllerWrite(config, kTargetAddress,
                           reinterpret_cast<uint8_t*>(serial.data()),
                           kTransferSize));
  auto buffer = std::vector<uint8_t>(kTransferSize, 0);

  printf("Reading back our serial number from the remote device...\r\n");
  CHECK(
      I2cControllerRead(config, kTargetAddress, buffer.data(), kTransferSize));
  CHECK(memcmp(buffer.data(), serial.data(), kTransferSize) == 0);
  printf("Readback of data from target device matches written data!\r\n");
}
// [end-sphinx-snippet:i2c-controller]
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
