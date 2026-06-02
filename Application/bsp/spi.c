/*
 * spi.c — SPI2 abstraction for W25QXX flash (CS = PD5)
 *
 * Uses HAL SPI driver (hspi2 from CubeMX) for reliable transfers.
 */
#include "spi.h"
#include "main.h"
#include "stm32u3xx_ll_gpio.h"

#define SPI_DUMMY_DATA 0xFF
#define SPI_TIMEOUT    100

#define SPI_FLASH_CS_HIGH() LL_GPIO_SetOutputPin(SPI2_CS_GPIO_Port, SPI2_CS_Pin)
#define SPI_FLASH_CS_LOW()  LL_GPIO_ResetOutputPin(SPI2_CS_GPIO_Port, SPI2_CS_Pin)

extern SPI_HandleTypeDef hspi2;

void spi_init(void)
{
  /* CS idle high */
  SPI_FLASH_CS_HIGH();
}

void spi_cs_high(void)
{
  SPI_FLASH_CS_HIGH();
}

void spi_cs_low(void)
{
  SPI_FLASH_CS_LOW();
}

uint8_t spi_send_byte(uint8_t data)
{
  uint8_t rx = 0;
  HAL_SPI_TransmitReceive(&hspi2, &data, &rx, 1, SPI_TIMEOUT);
  return rx;
}

uint8_t spi_read_byte(void)
{
  return spi_send_byte(SPI_DUMMY_DATA);
}
