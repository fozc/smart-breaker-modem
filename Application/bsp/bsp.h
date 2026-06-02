/*
 * bsp.h — Board Support Package for Smart Breaker Modem
 */
#ifndef BSP_BSP_H_
#define BSP_BSP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "xprintf.h"

#define bsp_io_putchr  bsp_putchr

void bsp_init(void);
void bsp_putchr(int chr);
void bsp_kick_wdt(void);
uint32_t bsp_get_tick(void);
void bsp_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BSP_H_ */
