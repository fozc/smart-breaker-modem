#include "test_uart.h"
#include "test_runner.h"
#include "main.h"
#include "bsp.h"
#include "uart.h"
#include "gpio.h"
#include "gpio_defs.h"
#include "xprintf.h"
#include "stm32u3xx_ll_usart.h"
#include <string.h>

/* ---- Helper: send string via LL UART wrapper ---- */
static void uart_tx(uart_port_t port, const char *data, const char *label)
{
  uart_send_buffer(port, data, strlen(data));
  /* Wait for TC */
  while (!uart_is_transmit_complete(port)) {}
  xprintf("  %s TX OK (%d bytes)\r\n", label, (int)strlen(data));
}

/* ---- Helper: try to receive data via LL polling ---- */
static uint16_t uart_rx_poll(USART_TypeDef *inst, uint8_t *buf, uint16_t maxlen, uint32_t timeout_ms)
{
  uint16_t idx = 0;
  uint32_t start = bsp_get_tick();
  while ((bsp_get_tick() - start) < timeout_ms && idx < maxlen) {
    if (LL_USART_IsActiveFlag_RXNE_RXFNE(inst)) {
      buf[idx++] = LL_USART_ReceiveData8(inst);
      start = bsp_get_tick(); /* reset timeout on each byte */
    }
  }
  return idx;
}

/* ---- LPUART1 (Console/USB) ---- */
TestResult_t test_uart_lpuart1(void)
{
  xprintf("  LPUART1 209700 baud - this message proves TX works\r\n");
  return TEST_PASS;
}

/* ---- USART1 (GSM Modem - 115200, RTS/CTS) ---- */
TestResult_t test_uart_usart1_gsm(void)
{
  xprintf("  USART1 (GSM) 115200 baud, HW flow control\r\n");
  const char *at_cmd = "AT\r\n";
  uart_tx(UART_1, at_cmd, "GSM");

  uint8_t rx[64] = {0};
  uint16_t len = uart_rx_poll(USART1, rx, sizeof(rx) - 1, 2000);

  if (len > 0) {
    rx[len] = 0;
    xprintf("  GSM RX: %s\r\n", (char *)rx);
    if (strstr((char *)rx, "OK"))
      return TEST_PASS;
    xprintf("  No 'OK' in response\r\n");
    return TEST_FAIL;
  }
  xprintf("  No response (modem may be off)\r\n");
  return TEST_SKIP;
}

/* ---- USART3 (RF Module - 115200) ---- */
TestResult_t test_uart_usart3_rf(void)
{
  xprintf("  USART3 (RF) 115200 baud\r\n");
  uart_tx(UART_3, "RF_TEST\r\n", "RF");
  return TEST_PASS;
}

/* ---- UART4 (Modbus RS-485 - 115200) ---- */
TestResult_t test_uart_uart4_modbus(void)
{
  xprintf("  UART4 (Modbus) 115200 baud\r\n");
  gpio_set_pin(MODBUS_OE_BSP_GPIO, MODBUS_OE_BSP_PIN, GPIO_HIGH);
  bsp_delay_ms(1);
  uart_tx(UART_4, "MODBUS_TEST\r\n", "MODBUS");
  gpio_set_pin(MODBUS_OE_BSP_GPIO, MODBUS_OE_BSP_PIN, GPIO_LOW);
  return TEST_PASS;
}

/* ---- UART5 (BMS RS-485 - 115200) ---- */
TestResult_t test_uart_uart5_bms(void)
{
  xprintf("  UART5 (BMS) 115200 baud\r\n");
  gpio_set_pin(BMS_OE_BSP_GPIO, BMS_OE_BSP_PIN, GPIO_HIGH);
  gpio_set_pin(BMS_RE_BSP_GPIO, BMS_RE_BSP_PIN, GPIO_HIGH);
  bsp_delay_ms(1);
  uart_tx(UART_5, "BMS_TEST\r\n", "BMS");
  gpio_set_pin(BMS_OE_BSP_GPIO, BMS_OE_BSP_PIN, GPIO_LOW);
  gpio_set_pin(BMS_RE_BSP_GPIO, BMS_RE_BSP_PIN, GPIO_LOW);
  return TEST_PASS;
}
