#include "test_runner.h"
#include "test_led.h"
#include "test_uart.h"
#include "test_i2c.h"
#include "test_spi.h"
#include "test_adc.h"
#include "test_gpio.h"
#include "test_pwm.h"
#include "test_gsm.h"
#include "test_rf.h"
#include "test_modbus.h"
#include "test_bms.h"
#include "test_rtc.h"
#include "main.h"
#include "bsp.h"
#include "uart.h"
#include "xprintf.h"
#include "gpio.h"
#include "gpio_defs.h"
#include "stm32u3xx_ll_lpuart.h"

/* ---- Console I/O via LPUART1 (LL-based, through BSP) ---- */

void test_console_init(void)
{
  bsp_init();
}

/* Wait for a character with LED1 blink while idle */
uint8_t test_getc_timeout(uint8_t *ch, uint32_t timeout_ms)
{
  uint32_t start = bsp_get_tick();
  uint32_t last_blink = bsp_get_tick();
  while (timeout_ms == 0 || (bsp_get_tick() - start) < timeout_ms) {
    if (LL_LPUART_IsActiveFlag_RXNE_RXFNE(LPUART1)) {
      *ch = LL_LPUART_ReceiveData8(LPUART1);
      return 1;
    }
    /* Blink LED1 every 500ms while waiting */
    if ((bsp_get_tick() - last_blink) >= 500) {
      gpio_toggle_pin(LED1_GPIO, LED1_PIN);
      last_blink = bsp_get_tick();
    }
  }
  return 0;
}

bool test_ask_confirm(const char *question)
{
  xprintf("%s (y/n): ", question);
  uint8_t ch = 0;
  while (1) {
    if (test_getc_timeout(&ch, 30000)) {
      xprintf("%c\r\n", ch);
      return (ch == 'y' || ch == 'Y');
    }
    /* 30s timeout → treat as NO */
    xprintf("\r\n[TIMEOUT] No response, assuming NO\r\n");
    return false;
  }
}

/* ---- Test Registry ---- */

static const TestEntry_t test_table[] = {
  { "LED Animation",           test_led_run },
  { "UART LPUART1 (USB/DBG)",  test_uart_lpuart1 },
  { "UART USART1 (GSM)",       test_uart_usart1_gsm },
  { "UART USART3 (RF)",        test_uart_usart3_rf },
  { "UART UART4 (Modbus)",     test_uart_uart4_modbus },
  { "UART UART5 (BMS)",        test_uart_uart5_bms },
  { "I2C Bus Scan",            test_i2c_scan },
  { "SPI Flash JEDEC ID",      test_spi_flash_id },
  { "ADC Voltages",            test_adc_voltages },
  { "GPIO Inputs",             test_gpio_inputs },
  { "Watchdog Feed",           test_watchdog_feed },
  { "PWM Relays",              test_pwm_relays },
  { "PWM Heater",              test_pwm_heater },
  { "GSM Module",              test_gsm_module },
  { "RF Module",               test_rf_module },
  { "Modbus RS-485",           test_modbus_rs485 },
  { "BMS RS-485",              test_bms_rs485 },
  { "RTC Clock",               test_rtc },
};

#define TEST_COUNT (sizeof(test_table) / sizeof(test_table[0]))

/* ---- Helpers ---- */

static const char *result_str(TestResult_t r)
{
  switch (r) {
    case TEST_PASS: return XCOLOR_GREEN "[PASS]" XCOLOR_RESET;
    case TEST_FAIL: return XCOLOR_RED   "[FAIL]" XCOLOR_RESET;
    case TEST_SKIP: return XCOLOR_YELLOW "[SKIP]" XCOLOR_RESET;
    case TEST_INFO: return XCOLOR_CYAN  "[INFO]" XCOLOR_RESET;
    default:        return "[????]";
  }
}

static void print_banner(void)
{
  xprintf("\r\n");
  xprintf("=====================================================\r\n");
  xprintf("   SMART BREAKER MODEM - PRODUCTION TEST v1.0\r\n");
  xprintf("=====================================================\r\n");
}

static void print_report(const TestReport_t *r)
{
  xprintf("\r\n=====================================================\r\n");
  xprintf("   TEST REPORT\r\n");
  xprintf("=====================================================\r\n");
  xprintf("   Total : %d\r\n", r->total);
  xcprintf(XCOLOR_GREEN,  "   Pass  : %d\r\n", r->passed);
  xcprintf(XCOLOR_RED,    "   Fail  : %d\r\n", r->failed);
  xcprintf(XCOLOR_YELLOW, "   Skip  : %d\r\n", r->skipped);
  xprintf("=====================================================\r\n");

  if (r->failed == 0)
    xcprintf(XCOLOR_GREEN, "   >> BOARD OK <<\r\n");
  else
    xcprintf(XCOLOR_RED,   "   >> BOARD FAIL <<\r\n");

  xprintf("=====================================================\r\n\r\n");
}

static void run_single(uint8_t idx, TestReport_t *report)
{
  xprintf("\r\n--- [%d/%d] %s ---\r\n", idx + 1, (int)TEST_COUNT, test_table[idx].name);
  TestResult_t res = test_table[idx].func();
  xprintf("--- Result: %s %s ---\r\n", test_table[idx].name, result_str(res));

  report->total++;
  switch (res) {
    case TEST_PASS: report->passed++;  break;
    case TEST_FAIL: report->failed++;  break;
    case TEST_SKIP: report->skipped++; break;
    case TEST_INFO: report->passed++;  break; /* INFO counts as pass */
  }
}

/* ---- Public API ---- */

void test_run_all(void)
{
  print_banner();
  xprintf("Running ALL tests...\r\n");

  TestReport_t report = {0};
  for (uint8_t i = 0; i < TEST_COUNT; i++) {
    run_single(i, &report);
  }

  print_report(&report);
}

static void print_menu(void)
{
  print_banner();
  xprintf("[0] Run ALL tests\r\n");
  for (uint8_t i = 0; i < TEST_COUNT; i++) {
    if (i < 9)
      xprintf("[%d] %s\r\n", i + 1, test_table[i].name);
    else
      xprintf("[%c] %s\r\n", 'A' + (i - 9), test_table[i].name);
  }
  xprintf("[Q] Reprint menu\r\n");
}

void test_run_menu(void)
{
  print_menu();

  while (1) {
    xprintf("\r\nSelect> ");

    uint8_t ch = 0;
    if (!test_getc_timeout(&ch, 0)) {
      continue;
    }
    xprintf("%c\r\n", ch);

    if (ch == '0') {
      test_run_all();
      print_menu();
      continue;
    }

    /* Enter, Q, or invalid key → reprint menu */
    if (ch == '\r' || ch == '\n' || ch == 'q' || ch == 'Q') {
      print_menu();
      continue;
    }

    int idx = -1;
    if (ch >= '1' && ch <= '9')
      idx = ch - '1';
    else if (ch >= 'A' && ch <= 'Z')
      idx = 9 + (ch - 'A');
    else if (ch >= 'a' && ch <= 'z')
      idx = 9 + (ch - 'a');

    if (idx >= 0 && idx < (int)TEST_COUNT) {
      TestReport_t report = {0};
      run_single((uint8_t)idx, &report);
      print_report(&report);
    } else {
      print_menu();
    }
  }
}
