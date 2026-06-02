#include "test_rtc.h"
#include "test_runner.h"
#include "main.h"
#include "bsp.h"
#include "xprintf.h"

extern RTC_HandleTypeDef hrtc;

TestResult_t test_rtc(void)
{
  RTC_TimeTypeDef time = {0};
  RTC_DateTypeDef date = {0};

  /* Read current time & date */
  if (HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK) {
    xprintf("  RTC GetTime FAILED\r\n");
    return TEST_FAIL;
  }
  /* GetDate must be called after GetTime to unlock shadow registers */
  if (HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK) {
    xprintf("  RTC GetDate FAILED\r\n");
    return TEST_FAIL;
  }

  xprintf("  Date: 20%02d-%02d-%02d\r\n", date.Year, date.Month, date.Date);
  xprintf("  Time: %02d:%02d:%02d\r\n", time.Hours, time.Minutes, time.Seconds);

  /* Tick test: wait 2 seconds and check if seconds advance */
  uint8_t sec_before = time.Seconds;
  xprintf("  Waiting 2 seconds to verify RTC ticking...\r\n");
  bsp_delay_ms(2000);

  HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

  uint8_t sec_after = time.Seconds;
  int diff = (int)sec_after - (int)sec_before;
  if (diff < 0) diff += 60;  /* handle minute rollover */

  xprintf("  After 2s: %02d:%02d:%02d (delta=%ds)\r\n",
          time.Hours, time.Minutes, time.Seconds, diff);

  if (diff >= 1 && diff <= 3) {
    xprintf("  RTC is ticking OK\r\n");
    return TEST_PASS;
  } else {
    xprintf("  RTC NOT ticking (delta=%ds)\r\n", diff);
    return TEST_FAIL;
  }
}
