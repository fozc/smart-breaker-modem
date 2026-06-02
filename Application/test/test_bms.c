#include "test_bms.h"
#include "test_runner.h"
#include "main.h"
#include "bsp.h"
#include "uart.h"
#include "gpio.h"
#include "gpio_defs.h"
#include "xprintf.h"
#include "stm32u3xx_ll_usart.h"

TestResult_t test_bms_rs485(void)
{
  xprintf("  BMS RS-485 (UART5) line test\r\n");

  /* Enable driver: BMS_OE (HIGH = TX enable), BMS_RE (HIGH = RX disable during TX) */
  xprintf("  Enabling BMS_OE for TX...\r\n");
  gpio_set_pin(BMS_OE_BSP_GPIO, BMS_OE_BSP_PIN, GPIO_HIGH);
  gpio_set_pin(BMS_RE_BSP_GPIO, BMS_RE_BSP_PIN, GPIO_HIGH);
  bsp_delay_ms(1);

  /* Send a test query */
  uint8_t test_data[] = { 0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77 };
  xprintf("  Sending BMS test frame (%d bytes)...\r\n", (int)sizeof(test_data));

  for (uint8_t i = 0; i < sizeof(test_data); i++)
    uart_send_byte(UART_5, test_data[i]);
  while (!uart_is_transmit_complete(UART_5)) {}

  xprintf("  TX OK\r\n");

  /* Switch to receive mode */
  gpio_set_pin(BMS_OE_BSP_GPIO, BMS_OE_BSP_PIN, GPIO_LOW);
  gpio_set_pin(BMS_RE_BSP_GPIO, BMS_RE_BSP_PIN, GPIO_LOW);

  /* Try to receive a response */
  uint8_t rx[64] = {0};
  uint16_t idx = 0;
  uint32_t start = bsp_get_tick();
  while ((bsp_get_tick() - start) < 1000 && idx < sizeof(rx)) {
    if (LL_USART_IsActiveFlag_RXNE_RXFNE(UART5)) {
      rx[idx++] = LL_USART_ReceiveData8(UART5);
      start = bsp_get_tick();
    }
  }

  if (idx > 0) {
    xprintf("  Response received (%d bytes): ", idx);
    for (uint8_t i = 0; i < idx && i < 32; i++)
      xprintf("%02X ", rx[i]);
    xprintf("\r\n");
  } else {
    xprintf("  No response (no BMS connected - TX-only test passed)\r\n");
  }

  return TEST_PASS;
}
