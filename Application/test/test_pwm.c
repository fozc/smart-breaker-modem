#include "test_pwm.h"
#include "test_runner.h"
#include "main.h"
#include "bsp.h"
#include "xprintf.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;

TestResult_t test_pwm_relays(void)
{
  xprintf("  TIM3 CH3 (RELAY1_PWM, PC8) - 50%% duty\r\n");
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, htim3.Init.Period / 2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_Delay(2000);
  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);

  xprintf("  TIM3 CH4 (RELAY2_PWM, PC9) - 50%% duty\r\n");
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, htim3.Init.Period / 2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  HAL_Delay(2000);
  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);

  return test_ask_confirm("Relay clicks heard?") ? TEST_PASS : TEST_FAIL;
}

TestResult_t test_pwm_heater(void)
{
  xprintf("  TIM1 CH1 (HEATER_PWM, PA8) - 25%% duty for 3s\r\n");
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, htim1.Init.Period / 4);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_Delay(3000);
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);

  return test_ask_confirm("Heater PWM output measured?") ? TEST_PASS : TEST_FAIL;
}
