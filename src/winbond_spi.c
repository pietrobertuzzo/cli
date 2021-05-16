/*
*   Created by Pietro Bertuzzo
*   2021-05-15
*
*
*/

/*! @file winbond_spi.c
*   @brief Interface for Winbond SPI flash memory.
*/

#include "winbond_spi.h"
#include <stm32f4xx_spi.h>

/**
  * @brief  Sets the NCS pin of the Winbond flash ic.
  * @retval None
  */
void Winbond_CS_Set(void)
{
    GPIO_SetBits(GPIOA, WINBOND_CS);
}

/**
  * @brief  Resets the NCS pin of the Winbond flash ic.
  * @retval None
  */
void Winbond_CS_Reset(void)
{
    GPIO_ResetBits(GPIOA, WINBOND_CS);
}

/**
  * @brief  Sends a byte of information through SPI1 to the Winbond flash ic.
  * @param  data: Byte to be sent.
  * @retval None
  */
void Winbond_SendByte(uint8_t data)
{
    SPI_I2S_SendData(SPI1, data);
}

/**
  * @brief  Recieves a byte of information through SPI1 from the winbond flash ic.
  * @retval Recieved byte from SPI1
  */
uint8_t Winbond_GetByte(void)
{
    //while(!SPI_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
    return SPI_I2S_ReceiveData(SPI1);

}

/**
  * @brief  Reads the manufacturer ID from the winbond flash. The operation
  *         consists of lowering NCS, sending the instruction (0x90), sending
  *         the 24 bit address (0x00000), reading the manufacturer ID (0xef) and 
  *         lastly reading the device ID.
  * @retval Manufacturer ID
  */
uint8_t Winbond_Read_Mnfr_ID(void){
    Winbond_CS_Reset();
    Winbond_SendByte(144);
    Winbond_SendByte(0x00);
    Winbond_SendByte(0x00);
    Winbond_SendByte(0x00);
    uint8_t mnfr_id = Winbond_GetByte();
    uint8_t dev_id = Winbond_GetByte();
    Winbond_CS_Set();
    return mnfr_id;
}

/**
  * @brief  Reads the device ID from the winbond flash. The operation
  *         consists of lowering NCS, sending the instruction (0x90), sending
  *         the 24 bit address (0x00001), reading the device ID and 
  *         lastly reading the manufacturer ID (0xef).
  * @retval Device ID
  */
uint8_t Winbond_Read_Dev_ID(void){
    Winbond_CS_Reset();
    Winbond_SendByte(144);
    Winbond_SendByte(0x01);
    Winbond_SendByte(0x00);
    Winbond_SendByte(0x00);

    uint8_t dev_id = Winbond_GetByte();
    uint8_t mnfr_id = Winbond_GetByte();
    Winbond_CS_Set();
    return dev_id;
}

