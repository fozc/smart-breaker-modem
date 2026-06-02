#include "test_spi.h"
#include "test_runner.h"
#include "main.h"
#include "bsp.h"
#include "spi.h"
#include "w25qxx.h"
#include "xprintf.h"

TestResult_t test_spi_flash_id(void)
{
  xprintf("  SPI2 Flash test via W25QXX driver\r\n");

  uint32_t jedec = w25qxx_read_jedecid();
  uint8_t mfr      = (jedec >> 16) & 0xFF;
  uint8_t mem_type = (jedec >> 8) & 0xFF;
  uint8_t capacity = jedec & 0xFF;

  xprintf("  JEDEC ID:     0x%06lX\r\n", jedec);
  xprintf("  Manufacturer: 0x%02X\r\n", mfr);
  xprintf("  Memory Type:  0x%02X\r\n", mem_type);
  xprintf("  Capacity:     0x%02X\r\n", capacity);

  if (mfr == 0xEF)
    xprintf("  -> Winbond W25Qxx detected\r\n");
  else if (mfr == 0xC8)
    xprintf("  -> GigaDevice GD25Qxx detected\r\n");
  else if (mfr == 0x20)
    xprintf("  -> Micron/ST detected\r\n");
  else if (mfr == 0x00 || mfr == 0xFF) {
    xprintf("  -> No flash device detected\r\n");
    return TEST_FAIL;
  } else
    xprintf("  -> Unknown manufacturer 0x%02X\r\n", mfr);

  uint16_t mfr_dev = w25qxx_read_manu_deviceid();
  xprintf("  Manufacturer/Device ID: 0x%04X\r\n", mfr_dev);

  /* Read-back test: read first 16 bytes */
  xprintf("  First 16 bytes at addr 0x000000: ");
  for (uint8_t i = 0; i < 16; i++) {
    uint8_t b = w25qxx_read_byte(i);
    xprintf("%02X ", b);
  }
  xprintf("\r\n");

  return TEST_PASS;
}
