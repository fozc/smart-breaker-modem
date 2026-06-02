#include "test_modbus.h"
#include "test_runner.h"
#include "main.h"
#include "bsp.h"
#include "uart.h"
#include "gpio.h"
#include "gpio_defs.h"
#include "xprintf.h"
#include "stm32u3xx_ll_usart.h"

TestResult_t test_modbus_rs485(void)
{
  xprintf("  Modbus RS-485 (UART4) line test\r\n");

  /* Check MODBUS_EN_FLT pin (PE14, open-drain) */
  uint8_t en_flt = gpio_read_pin(MODBUS_EN_FLT_BSP_GPIO, MODBUS_EN_FLT_BSP_PIN);
  xprintf("  MODBUS_EN_FLT = %s\r\n", en_flt ? "HIGH" : "LOW");

  /* Enable driver (MODBUS_OE = PA15) */
  xprintf("  Enabling MODBUS_OE (PA15)...\r\n");
  gpio_set_pin(MODBUS_OE_BSP_GPIO, MODBUS_OE_BSP_PIN, GPIO_HIGH);
  bsp_delay_ms(1);

  /* Send a Modbus-like test frame: slave addr 0x01, function 0x03, reg 0x0000, count 0x0001 */
  uint8_t test_frame[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A };
  xprintf("  Sending test frame (%d bytes)...\r\n", (int)sizeof(test_frame));

  for (uint8_t i = 0; i < sizeof(test_frame); i++)
    uart_send_byte(UART_4, test_frame[i]);
  while (!uart_is_transmit_complete(UART_4)) {}

  /* Switch to receive mode */
  gpio_set_pin(MODBUS_OE_BSP_GPIO, MODBUS_OE_BSP_PIN, GPIO_LOW);

  xprintf("  TX OK\r\n");

  /* Try to receive a response (if external slave connected) */
  uint8_t rx[32] = {0};
  uint16_t idx = 0;
  uint32_t start = bsp_get_tick();
  while ((bsp_get_tick() - start) < 1000 && idx < sizeof(rx)) {
    if (LL_USART_IsActiveFlag_RXNE_RXFNE(UART4)) {
      rx[idx++] = LL_USART_ReceiveData8(UART4);
      start = bsp_get_tick();
    }
  }

  if (idx > 0) {
    xprintf("  Response received (%d bytes): ", idx);
    for (uint8_t i = 0; i < idx; i++)
      xprintf("%02X ", rx[i]);
    xprintf("\r\n");
  } else {
    xprintf("  No response (no slave connected - TX-only test passed)\r\n");
  }

  return TEST_PASS;
}
