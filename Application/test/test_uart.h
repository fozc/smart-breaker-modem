#ifndef TEST_UART_H
#define TEST_UART_H

#include "test_runner.h"

TestResult_t test_uart_lpuart1(void);
TestResult_t test_uart_usart1_gsm(void);
TestResult_t test_uart_usart3_rf(void);
TestResult_t test_uart_uart4_modbus(void);
TestResult_t test_uart_uart5_bms(void);

#endif
