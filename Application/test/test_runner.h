#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  TEST_PASS,
  TEST_FAIL,
  TEST_SKIP,
  TEST_INFO,
} TestResult_t;

typedef struct {
  const char *name;
  TestResult_t (*func)(void);
} TestEntry_t;

typedef struct {
  uint8_t total;
  uint8_t passed;
  uint8_t failed;
  uint8_t skipped;
} TestReport_t;

/* Console helpers */
void test_console_init(void);
uint8_t test_getc_timeout(uint8_t *ch, uint32_t timeout_ms);
bool test_ask_confirm(const char *question);

/* Runner */
void test_run_all(void);
void test_run_menu(void);

#endif /* TEST_RUNNER_H */
