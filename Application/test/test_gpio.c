#include "test_gpio.h"
#include "test_runner.h"
#include "main.h"
#include "bsp.h"
#include "gpio.h"
#include "gpio_defs.h"
#include "xprintf.h"

typedef struct {
  gpio_port_t port;
  gpio_pin_t pin;
  const char *name;
} GpioDef_t;

static const GpioDef_t inputs[] = {
  { USER_RESET_SW_GPIO, USER_RESET_SW_PIN, "USER_HARD_RESET_SW" },
  { DIP_SW1_GPIO,       DIP_SW1_PIN,       "DIP_SW1" },
  { DIP_SW2_GPIO,       DIP_SW2_PIN,       "DIP_SW2" },
  { DIN1_BSP_GPIO,      DIN1_BSP_PIN,      "DIN1" },
  { DIN2_BSP_GPIO,      DIN2_BSP_PIN,      "DIN2" },
  { DIN3_BSP_GPIO,      DIN3_BSP_PIN,      "DIN3" },
  { DIN4_BSP_GPIO,      DIN4_BSP_PIN,      "DIN4" },
  { VREG_SENS_BSP_GPIO, VREG_SENS_BSP_PIN, "VREG_SENS" },
  { RF_IO2_BSP_GPIO,    RF_IO2_BSP_PIN,    "RF_IO2" },
  { OVP_PV_BSP_GPIO,    OVP_PV_BSP_PIN,    "OVP_PV" },
  { GSM_SW_RDY_BSP_GPIO,GSM_SW_RDY_BSP_PIN,"GSM_SW_RDY" },
  { CHARGER_STAT_BSP_GPIO,CHARGER_STAT_BSP_PIN,"CHARGER_STAT" },
  { CHARGER_INT_BSP_GPIO, CHARGER_INT_BSP_PIN, "CHARGER_INT" },
};

#define INPUT_COUNT (sizeof(inputs) / sizeof(inputs[0]))

TestResult_t test_gpio_inputs(void)
{
  xprintf("  GPIO Input States:\r\n");
  xprintf("  %-22s  %s\r\n", "Pin", "State");
  xprintf("  %-22s  %s\r\n", "---", "-----");

  for (uint8_t i = 0; i < INPUT_COUNT; i++) {
    uint8_t val = gpio_read_pin(inputs[i].port, inputs[i].pin);
    xprintf("  %-22s  %s\r\n", inputs[i].name, val ? "HIGH" : "LOW");
  }

  return TEST_INFO;
}

TestResult_t test_watchdog_feed(void)
{
  xprintf("  External watchdog feed test (EWDT_FEED = PE6)\r\n");
  xprintf("  Toggling EWDT_FEED 10 times...\r\n");

  for (uint8_t i = 0; i < 10; i++) {
    gpio_toggle_pin(EXT_WDT_KICK_GPIO, EXT_WDT_KICK_PIN);
    bsp_delay_ms(50);
  }

  gpio_set_pin(EXT_WDT_KICK_GPIO, EXT_WDT_KICK_PIN, GPIO_LOW);
  xprintf("  EWDT_FEED toggled successfully - system still running\r\n");
  return TEST_PASS;
}
