/*
 * bsp.c — Board Support Package for Smart Breaker Modem
 *
 * Console output via LPUART1 (USB_TX/USB_RX) using LL driver.
 */
#include "bsp.h"
#include "main.h"
#include "gpio.h"
#include "gpio_defs.h"
#include "stm32u3xx_ll_lpuart.h"

void bsp_putchr(int chr)
{
  while (!LL_LPUART_IsActiveFlag_TXE_TXFNF(LPUART1));
  LL_LPUART_TransmitData8(LPUART1, (uint8_t)chr);
}

void bsp_init(void)
{
  xdev_out(bsp_io_putchr);
}

void bsp_kick_wdt(void)
{
  gpio_toggle_pin(EXT_WDT_KICK_GPIO, EXT_WDT_KICK_PIN);
}

uint32_t bsp_get_tick(void)
{
  return HAL_GetTick();
}

void bsp_delay_ms(uint32_t ms)
{
  HAL_Delay(ms);
}
