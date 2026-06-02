#include "test_adc.h"
#include "test_runner.h"
#include "main.h"
#include "bsp.h"
#include "xprintf.h"

/*
 * ADC1 readings via CubeMX-generated hadc1 handle.
 *
 * ADC1 channels:
 *   PA0  SENS_3V3  -> ADC1_IN3
 *   PA1  SENS_3V8  -> ADC1_IN4
 *   PA2  SENS_5V   -> ADC1_IN5
 *
 * VREF = 3300 mV, 12-bit resolution.
 */

extern ADC_HandleTypeDef hadc1;

#define ADC_VREF_MV  2500  /* VREFBUF at SCALE3 = 2.5V */
#define ADC_MAX_VAL  4095

/* Voltage divider: 16.2K + 10K, ADC reads across 10K
 * V_rail = V_adc * (R1 + R2) / R2 = V_adc * 262 / 100 */
#define DIVIDER_NUM   262
#define DIVIDER_DEN   100

typedef struct {
  const char *name;
  uint32_t channel;
} AdcCh_t;

static const AdcCh_t channels[] = {
  { "SENS_3V3 (PA0)", ADC_CHANNEL_3 },
  { "SENS_3V8 (PA1)", ADC_CHANNEL_4 },
  { "SENS_5V  (PA2)", ADC_CHANNEL_5 },
};

#define CH_COUNT (sizeof(channels) / sizeof(channels[0]))

static uint32_t adc_read_channel(uint32_t channel)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel      = channel;
  sConfig.Rank         = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_246CYCLES_5;  /* longer sampling for high-Z dividers */
  sConfig.SingleDiff   = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset       = 0;

  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    return 0xFFFFFFFF;

  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
    HAL_ADC_Stop(&hadc1);
    return 0xFFFFFFFF;
  }

  uint32_t val = HAL_ADC_GetValue(&hadc1);
  HAL_ADC_Stop(&hadc1);
  return val;
}

TestResult_t test_adc_voltages(void)
{
  xprintf("  ADC1 Voltage Readings (VREF=%dmV, Divider=16.2K/10K)\r\n", ADC_VREF_MV);

  xprintf("  %-20s  %6s  %8s  %8s\r\n", "Channel", "Raw", "Pin mV", "Rail mV");
  xprintf("  %-20s  %6s  %8s  %8s\r\n", "-------", "---", "------", "-------");

  for (uint8_t i = 0; i < CH_COUNT; i++) {
    /* Discard first reading (may have stale data), use second */
    adc_read_channel(channels[i].channel);

    uint32_t raw = adc_read_channel(channels[i].channel);
    if (raw == 0xFFFFFFFF) {
      xprintf("  %-20s  %6d  %8d  %8d  [WARNING: ADC read error]\r\n", channels[i].name, 0, 0, 0);
      continue;
    }
    uint32_t pin_mv  = (raw * ADC_VREF_MV) / ADC_MAX_VAL;
    uint32_t rail_mv = (pin_mv * DIVIDER_NUM) / DIVIDER_DEN;
    xprintf("  %-20s  %6lu  %8lu  %8lu\r\n", channels[i].name, raw, pin_mv, rail_mv);
  }

  xprintf("\r\n  Divider: 16.2K / 10K  ->  Rail = Pin x 2.62\r\n");

  return TEST_INFO;
}
