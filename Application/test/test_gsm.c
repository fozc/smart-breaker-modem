#include "test_gsm.h"
#include "test_runner.h"
#include "main.h"
#include "bsp.h"
#include "uart.h"
#include "gpio.h"
#include "gpio_defs.h"
#include "xprintf.h"
#include "stm32u3xx_ll_usart.h"
#include <string.h>

static uint16_t gsm_rx_poll(uint8_t *buf, uint16_t maxlen, uint32_t timeout_ms)
{
  uint16_t idx = 0;
  uint32_t start = bsp_get_tick();
  while ((bsp_get_tick() - start) < timeout_ms && idx < maxlen) {
    if (LL_USART_IsActiveFlag_RXNE_RXFNE(USART1)) {
      buf[idx++] = LL_USART_ReceiveData8(USART1);
      start = bsp_get_tick();
    }
  }
  return idx;
}

static bool gsm_send_at(const char *cmd, char *resp, uint16_t resp_size, uint32_t timeout)
{
  uart_send_buffer(UART_1, cmd, strlen(cmd));
  while (!uart_is_transmit_complete(UART_1)) {}

  memset(resp, 0, resp_size);
  uint16_t len = gsm_rx_poll((uint8_t *)resp, resp_size - 1, timeout);
  return (len > 0);
}

TestResult_t test_gsm_module(void)
{
  char resp[128];

  xprintf("  Step 1: Power ON GSM module\r\n");
  gpio_set_pin(GSM_POWER_GPIO, GSM_POWER_PIN, GPIO_HIGH);
  bsp_delay_ms(500);

  xprintf("  Step 2: Toggle GSM_ONOFF pulse (4.5s)\r\n");
  gpio_set_pin(GSM_ONOFF_BSP_GPIO, GSM_ONOFF_BSP_PIN, GPIO_HIGH);
  bsp_delay_ms(4500);
  gpio_set_pin(GSM_ONOFF_BSP_GPIO, GSM_ONOFF_BSP_PIN, GPIO_LOW);

  xprintf("  Step 3: Waiting for GSM_SW_RDY (max 15s)...\r\n");
  uint32_t start = bsp_get_tick();
  bool ready = false;
  while ((bsp_get_tick() - start) < 15000) {
    if (!gpio_read_pin(GSM_SW_RDY_BSP_GPIO, GSM_SW_RDY_BSP_PIN)) {
      ready = true;
      break;
    }
    bsp_delay_ms(100);
  }

  bsp_delay_ms(100);

  if (!ready)
    xprintf("  GSM_SW_RDY not asserted within 15s\r\n");
  else
    xprintf("  GSM_SW_RDY = HIGH (%lu ms)\r\n", bsp_get_tick() - start);

  xprintf("  Step 4: Sending AT command\r\n");
  if (gsm_send_at("AT\r\n", resp, sizeof(resp), 2000)) {
    xprintf("  AT Response: %s\r\n", resp);
    if (!strstr(resp, "OK")) {
      xprintf("  No 'OK' in response\r\n");
      return TEST_FAIL;
    }
  } else {
    xprintf("  No response to AT command\r\n");
    return TEST_FAIL;
  }

  xprintf("  Step 5: Enable status LED (AT#GPIO=1,0,2)\r\n");
  if (gsm_send_at("AT#GPIO=1,0,2\r\n", resp, sizeof(resp), 2000)) {
    xprintf("  Response: %s\r\n", resp);
  } else {
    xprintf("  No response to GPIO command\r\n");
  }

  xprintf("  Step 6: Set SLED mode (AT#SLED=4)\r\n");
  if (gsm_send_at("AT#SLED=4\r\n", resp, sizeof(resp), 2000)) {
    xprintf("  Response: %s\r\n", resp);
  } else {
    xprintf("  No response to SLED command\r\n");
  }

  xprintf("  Step 7: Reading IMSI (AT+CIMI)\r\n");
  if (gsm_send_at("AT+CIMI\r\n", resp, sizeof(resp), 3000))
    xprintf("  IMSI: %s\r\n", resp);
  else
    xprintf("  IMSI read failed\r\n");

  xprintf("  Step 8: Signal quality (AT+CSQ)\r\n");
  if (gsm_send_at("AT+CSQ\r\n", resp, sizeof(resp), 3000))
    xprintf("  CSQ: %s\r\n", resp);
  else
    xprintf("  CSQ read failed\r\n");

  return TEST_PASS;
}
