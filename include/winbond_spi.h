/*! @file winbond_spi.h
*   @brief Definitions for Winbond SPI flash memory interface.
*/
// Define to prevent recursive inclusion
#ifndef _WINBOND_SPI_H_
#define _WINBOND_SPI_H_
#endif

// Includes
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"

// Pin mapping to the flash chip on the board (https://stm32-base.org/boards/STM32F407VGT6-STM32F4XX-M#W25Q16BV)
#define WINBOND_CS (GPIO_Pin_15)    //  \CS -> PA15
#define WINBOND_CS_SOURCE (GPIO_PinSource15)
#define WINBOND_DO (GPIO_Pin_4)     //  DO -> PB4
#define WINBOND_DO_SOURCE (GPIO_PinSource4)
#define WINBOND_DI (GPIO_Pin_5)     //  DI -> PB5
#define WINBOND_DI_SOURCE (GPIO_PinSource5)
#define WINBOND_CLK (GPIO_Pin_3)    //  CLK -> PB3
#define WINBOND_CLK_SOURCE (GPIO_PinSource3)



void Winbond_CS_Set(void);
void Winbond_CS_Reset(void);

uint8_t Winbond_Read_Mnfr_ID(void);
uint8_t Winbond_Read_Dev_ID(void);