#include "test_led.h"
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
} LedDef_t;

static const LedDef_t mono_leds[] = {
  { LED1_GPIO, LED1_PIN, "LED_1" },
  { LED2_GPIO, LED2_PIN, "LED_2" },
  { LED3_GPIO, LED3_PIN, "LED_3" },
};

static const LedDef_t rgb1_leds[] = {
  { LED_RGB1_R_GPIO, LED_RGB1_R_PIN, "RGB1_R" },
  { LED_RGB1_G_GPIO, LED_RGB1_G_PIN, "RGB1_G" },
  { LED_RGB1_B_GPIO, LED_RGB1_B_PIN, "RGB1_B" },
};

static const LedDef_t rgb2_leds[] = {
  { LED_RGB2_R_GPIO, LED_RGB2_R_PIN, "RGB2_R" },
  { LED_RGB2_G_GPIO, LED_RGB2_G_PIN, "RGB2_G" },
  { LED_RGB2_B_GPIO, LED_RGB2_B_PIN, "RGB2_B" },
};

static void leds_off(const LedDef_t *leds, uint8_t n)
{
  for (uint8_t i = 0; i < n; i++)
    gpio_set_pin(leds[i].port, leds[i].pin, GPIO_LOW);
}

static void leds_on(const LedDef_t *leds, uint8_t n)
{
  for (uint8_t i = 0; i < n; i++)
    gpio_set_pin(leds[i].port, leds[i].pin, GPIO_HIGH);
}

TestResult_t test_led_run(void)
{
  xprintf("  Phase 1: Mono LED chase (LED_1 -> LED_2 -> LED_3)\r\n");
  for (uint8_t rep = 0; rep < 3; rep++) {
    for (uint8_t i = 0; i < 3; i++) {
      leds_off(mono_leds, 3);
      gpio_set_pin(mono_leds[i].port, mono_leds[i].pin, GPIO_HIGH);
      bsp_delay_ms(120);
    }
    for (int8_t i = 1; i >= 0; i--) {
      leds_off(mono_leds, 3);
      gpio_set_pin(mono_leds[i].port, mono_leds[i].pin, GPIO_HIGH);
      bsp_delay_ms(120);
    }
  }
  leds_off(mono_leds, 3);

  xprintf("  Phase 2: RGB1 color cycle (R->G->B->White)\r\n");
  const char *colors[] = {"Red", "Green", "Blue"};
  for (uint8_t i = 0; i < 3; i++) {
    leds_off(rgb1_leds, 3);
    xprintf("    RGB1 %s\r\n", colors[i]);
    gpio_set_pin(rgb1_leds[i].port, rgb1_leds[i].pin, GPIO_HIGH);
    bsp_delay_ms(400);
  }
  xprintf("    RGB1 White\r\n");
  leds_on(rgb1_leds, 3);
  bsp_delay_ms(400);
  leds_off(rgb1_leds, 3);

  xprintf("  Phase 3: RGB2 color cycle (R->G->B->White)\r\n");
  for (uint8_t i = 0; i < 3; i++) {
    leds_off(rgb2_leds, 3);
    xprintf("    RGB2 %s\r\n", colors[i]);
    gpio_set_pin(rgb2_leds[i].port, rgb2_leds[i].pin, GPIO_HIGH);
    bsp_delay_ms(400);
  }
  xprintf("    RGB2 White\r\n");
  leds_on(rgb2_leds, 3);
  bsp_delay_ms(400);
  leds_off(rgb2_leds, 3);

  xprintf("  Phase 4: All LEDs flash 3x\r\n");
  for (uint8_t n = 0; n < 3; n++) {
    leds_on(mono_leds, 3);
    leds_on(rgb1_leds, 3);
    leds_on(rgb2_leds, 3);
    bsp_delay_ms(200);
    leds_off(mono_leds, 3);
    leds_off(rgb1_leds, 3);
    leds_off(rgb2_leds, 3);
    bsp_delay_ms(200);
  }

  return test_ask_confirm("All LEDs working correctly?") ? TEST_PASS : TEST_FAIL;
}
