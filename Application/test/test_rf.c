#include "test_rf.h"
#include "test_runner.h"
#include "main.h"
#include "bsp.h"
#include "uart.h"
#include "gpio.h"
#include "gpio_defs.h"
#include "xprintf.h"
#include "stm32u3xx_ll_usart.h"
#include <string.h>

TestResult_t test_rf_module(void)
{
  xprintf("  Step 1: Reset RF module (RF_RESET = PB10, open-drain)\r\n");
  gpio_set_pin(RF_RESET_BSP_GPIO, RF_RESET_BSP_PIN, GPIO_LOW);
  bsp_delay_ms(100);
  gpio_set_pin(RF_RESET_BSP_GPIO, RF_RESET_BSP_PIN, GPIO_HIGH);
  bsp_delay_ms(500);

  xprintf("  Step 2: Check RF_IO2 (PA6)\r\n");
  uint8_t io2 = gpio_read_pin(RF_IO2_BSP_GPIO, RF_IO2_BSP_PIN);
  xprintf("  RF_IO2 = %s\r\n", io2 ? "HIGH" : "LOW");

  xprintf("  Step 3: Send 'ping\\r' via USART3\r\n");
  const char *ping = "ping\r";
  uart_send_buffer(UART_3, ping, strlen(ping));
  while (!uart_is_transmit_complete(UART_3)) {}

  xprintf("  Step 4: Waiting for 'pong' (max 2s)...\r\n");
  uint8_t rx[64] = {0};
  uint16_t idx = 0;
  uint32_t start = bsp_get_tick();
  while ((bsp_get_tick() - start) < 2000 && idx < sizeof(rx) - 1) {
    if (LL_USART_IsActiveFlag_RXNE_RXFNE(USART3)) {
      rx[idx++] = LL_USART_ReceiveData8(USART3);
      start = bsp_get_tick();
    }
  }

  if (idx > 0) {
    rx[idx] = 0;
    xprintf("  RF Response: ");
    for (uint16_t i = 0; i < idx; i++) {
      if (rx[i] >= 0x20 && rx[i] < 0x7F)
        xprintf("%c", rx[i]);
      else
        xprintf("[%02X]", rx[i]);
    }
    xprintf("\r\n");

    if (strstr((char *)rx, "pong")) {
      xprintf("  RF module responded with 'pong'\r\n");
      return TEST_PASS;
    }
    xprintf("  Response received but no 'pong'\r\n");
    return TEST_FAIL;
  }

  xprintf("  No response from RF module\r\n");
  return TEST_FAIL;
}
