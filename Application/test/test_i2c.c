#include "test_i2c.h"
#include "test_runner.h"
#include "main.h"
#include "bsp.h"
#include "xprintf.h"

extern I2C_HandleTypeDef hi2c3;

TestResult_t test_i2c_scan(void)
{
  xprintf("  I2C3 bus scan (0x08 - 0x77):\r\n");
  xprintf("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n");

  uint8_t found = 0;

  for (uint8_t row = 0; row < 8; row++) {
    xprintf("  %02X:", row << 4);
    for (uint8_t col = 0; col < 16; col++) {
      uint8_t addr = (row << 4) | col;

      if (addr < 0x08 || addr > 0x77) {
        xprintf("   ");
        continue;
      }

      HAL_StatusTypeDef st = HAL_I2C_IsDeviceReady(&hi2c3, (uint16_t)(addr << 1), 1, 10);
      if (st == HAL_OK) {
        xprintf(" %02X", addr);
        found++;
      } else {
        xprintf(" --");
      }
    }
    xprintf("\r\n");
  }

  xprintf("  Found %d device(s) on I2C3\r\n", found);

  if (found > 0) {
    /* List found devices explicitly */
    xprintf("  Detected addresses: ");
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
      if (HAL_I2C_IsDeviceReady(&hi2c3, (uint16_t)(addr << 1), 1, 10) == HAL_OK) {
        xprintf("0x%02X ", addr);
      }
    }
    xprintf("\r\n");
  }

  return TEST_INFO;
}
