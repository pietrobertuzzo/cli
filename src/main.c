#ifdef STM32L1
	#include <stm32l1xx_gpio.h>
	#include <stm32l1xx_rcc.h>
	#define LEDPORT (GPIOB)
	#define LEDPIN (GPIO_Pin_7)
	#define ENABLE_GPIO_CLOCK (RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE))
#elif STM32F3
	#include <stm32f30x_gpio.h>
	#include <stm32f30x_rcc.h>
	#define LEDPORT (GPIOE)
	#define LEDPIN (GPIO_Pin_8)
	#define ENABLE_GPIO_CLOCK (RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE))
#elif STM32F4
	#include <stm32f4xx_gpio.h>
	#include <stm32f4xx_rcc.h>
	#include <misc.h>
	#include <stm32f4xx_exti.h>
	#include <stm32f4xx_usart.h>
	#include <stm32f4xx_spi.h>
	#include "winbond_spi.h"
	#define LEDPORT (GPIOA)
	#define LEDPIN (GPIO_Pin_1)
	#define USART1_RXPIN (GPIO_Pin_10)
	#define USART1_TXPIN (GPIO_Pin_9)
	#define USART1_PORT (GPIOA)
	#define BUTTONPORT (GPIOA)
	#define BUTTONPIN (GPIO_Pin_0)
	#define ENABLE_GPIO_CLOCK (RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE))
	#define ENABLE_USART1_CLOCK (RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE))
#endif

#include "cli.h"
#include <string.h>
#define MAX_STRLEN 10

const char *asciihex[256] = {
	"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0a", "0b", "0c", "0d", "0e", "0f",
	"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1a", "1b", "1c", "1d", "1e", "1f",
	"20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2a", "2b", "2c", "2d", "2e", "2f",
	"30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3a", "3b", "3c", "3d", "3e", "3f",
	"40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4a", "4b", "4c", "4d", "4e", "4f",
	"50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5a", "5b", "5c", "5d", "5e", "5f",
	"60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6a", "6b", "6c", "6d", "6e", "6f",
	"70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7a", "7b", "7c", "7d", "7e", "7f",
	"80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8a", "8b", "8c", "8d", "8e", "8f",
	"90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9a", "9b", "9c", "9d", "9e", "9f",
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af",
	"b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "b9", "ba", "bb", "bc", "bd", "be", "bf",
	"c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf",
	"d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "da", "db", "dc", "dd", "de", "df",
	"e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7", "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef",
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "fa", "fb", "fc", "fd", "fe", "ff"

};

volatile char rxBuffer[MAX_STRLEN + 2];
volatile uint8_t CMD_FLAG = 0;
char cli_prompt[];

void devid_func(int argc, char **argv);
void mnfrid_func(int argc, char **argv);
static void cli_print(cli_t *cli, const char *msg);
static void echo_func(int argc, char **argv);
static void help_func(int argc, char **argv);
//static void spi_func(int argc, char **argv);
//static uint8_t *spi_func_for(int argc, char **argv);
void simple_delay(uint32_t us);
uint8_t Serial_GetByte(USART_TypeDef *USARTx);
void Serial_PutByte(USART_TypeDef *USARTx, uint8_t byte);
uint8_t Serial_PutPacket(USART_TypeDef *USARTx, uint8_t *data);
uint8_t Serial_PutString(USART_TypeDef *USARTx, char *p_string);
void USART_puts(USART_TypeDef *USARTx, volatile char *str);
void Peripheral_Init(void);
void user_uart_println(char *string);
void SPI_Peripheral_Init(void);

cmd_t cmd_tbl[] = {
    {
        .cmd = "help",
        .func = help_func
    },
    {
        .cmd = "echo",
        .func = echo_func
    },
	/*
	{
		.cmd = "spi",
		.func = spi_func
	},
	*/
	{
		.cmd = "mnfrid",
		.func = mnfrid_func
	},
	{
		.cmd = "devid",
		.func = devid_func
	}
};


cli_t cli;

int main(void)
{
	
	Peripheral_Init();
	SPI_Peripheral_Init();
	cli.println = user_uart_println;
	cli.cmd_tbl = cmd_tbl;
	cli.cmd_cnt = sizeof(cmd_tbl)/sizeof(cmd_t);
	
	USART_puts(USART1, "System started.\r\n");

	cli_init(&cli);
	

	/* main program loop */
	for (;;) {

		Winbond_Read_Dev_ID();
		//Winbond_Read_Mnfr_ID();
		if(CMD_FLAG){
			cli_process(&cli);
			CMD_FLAG = 0;
			cli_print(&cli, cli_prompt);
		}
	}

	/* never reached */
	return 0;
}

void simple_delay(uint32_t us)
{
	/* simple delay loop */
	while (us--) {
		asm volatile ("nop");
	}
}
uint8_t Serial_GetByte(USART_TypeDef *USARTx)
{
	while(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET);
	return USART_ReceiveData(USARTx);
}
void Serial_PutByte(USART_TypeDef *USARTx, uint8_t byte)
{
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
	USART_SendData(USARTx, byte);
}
void USART_puts(USART_TypeDef *USARTx, volatile char *str)
{
	while(*str){
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
		USART_SendData(USARTx, *str);
		*str++; 
	}
}

void Peripheral_Init(void)
{
	USART_InitTypeDef usart1;
	GPIO_InitTypeDef gpio_usart1;
	NVIC_InitTypeDef NVIC_InitStruct;
	RCC_DeInit();

	ENABLE_USART1_CLOCK;
	ENABLE_GPIO_CLOCK;

	gpio_usart1.GPIO_Pin = USART1_RXPIN | USART1_TXPIN;
	gpio_usart1.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_usart1.GPIO_Mode = GPIO_Mode_AF;
	gpio_usart1.GPIO_OType = GPIO_OType_PP;
	gpio_usart1.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(USART1_PORT, &gpio_usart1);

	GPIO_PinAFConfig(USART1_PORT, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(USART1_PORT, GPIO_PinSource10, GPIO_AF_USART1);

	usart1.USART_BaudRate = 9600;
	usart1.USART_Parity = USART_Parity_No;
	usart1.USART_WordLength = USART_WordLength_8b;
	usart1.USART_StopBits = USART_StopBits_1;
	usart1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart1.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &usart1);


	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	USART_Cmd(USART1, ENABLE);
}

void user_uart_println(char *string)
{
	USART_puts(USART1, string);
}


void help_func(int argc, char **argv)
{
    cli_print(&cli, "Functions available:\r\n");
	cli_print(&cli, "help, echo.\r\n");
}

void echo_func(int argc, char **argv)
{
	uint8_t cnt;
	for(cnt = 0;cnt < argc;cnt++){
		**argv++;
    	cli_print(&cli,*argv);
		cli_print(&cli," ");
	}
	cli_print(&cli,"\r\n");
}

/*
void spi_func(int argc, char **argv)
{
	uint8_t cnt, *data;
	if(argc == 0){
		cli_print(&cli, "Insufficient parameters.\r\n");
	}
	if(strcmp(*argv[1], "hex")){
		for(cnt = 0;cnt < (argc - 1);cnt++){
			*data = spi_func_for(argc, **argv);
			SPI_I2S_SendData(SPI1, asciihex[data[cnt]]);
		}
	}

}

uint8_t *spi_func_for(int argc, char **argv)
{
	uint8_t data[argc-1];
	uint8_t cnt;
	for(cnt = 2;cnt < argc; cnt++){
		data[cnt] = *argv[cnt];
	}
	return *data;
}
*/

void mnfrid_func(int argc, char **argv)
{
	cli_print(&cli, asciihex[Winbond_Read_Mnfr_ID()]);
	cli_print(&cli,"\r\n");
}

void devid_func(int argc, char **argv)
{
	cli_print(&cli, asciihex[Winbond_Read_Dev_ID()]);
	cli_print(&cli,"\r\n");
}

void USART1_IRQHandler(void)
{
	/*
	if(USART_GetITStatus(USART2, USART_IT_RXNE)){
		static uint8_t cnt = 0;
		char ch = USART1->DR;
		if((ch != '\r') && (cnt < MAX_STRLEN)){
			rxBuffer[cnt++] = ch;static void cli_print(cli_t *cli, const char *msg)
		}
		else{
			rxBuffer[cnt] = '\r';
			rxBuffer[cnt+1] = '\n';
			USART_puts(USART1, rxBuffer);
			for(cnt = 0;cnt < MAX_STRLEN +2;cnt++){
				rxBuffer[cnt] = '\0';
			}
			cnt = 0;
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
	*/

	char c = USART1->DR;
	cli_put(&cli, c);
	if(c == 0x0d){
		CMD_FLAG = 1;
	}
}

static void cli_print(cli_t *cli, const char *msg)
{
    char buf[50];

    strcpy(buf, msg);
    cli->println(buf);
}

void SPI_Peripheral_Init(void)
{
	// Enable peripheral clock for SPI1 and GPIOB 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_PinAFConfig(GPIOB, WINBOND_CLK_SOURCE, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, WINBOND_DI_SOURCE, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, WINBOND_DO_SOURCE, GPIO_AF_SPI1);

	GPIO_InitTypeDef gpio_spi;
	gpio_spi.GPIO_Mode = GPIO_Mode_AF;
	gpio_spi.GPIO_OType = GPIO_OType_PP;
	gpio_spi.GPIO_Pin = WINBOND_CLK | WINBOND_DI | WINBOND_DO;
	gpio_spi.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio_spi.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio_spi);

	SPI_InitTypeDef spi;
	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	spi.SPI_Mode = SPI_Mode_Master;
	spi.SPI_DataSize = SPI_DataSize_8b;
	spi.SPI_CPOL = SPI_CPOL_Low;
	spi.SPI_CPHA = SPI_CPHA_1Edge;
	spi.SPI_NSS = SPI_NSS_Soft;
	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	spi.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPI1, &spi);

	SPI_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);

	SPI_Cmd(SPI1, ENABLE);

	GPIO_InitTypeDef gpio_cs_winbond;
	gpio_cs_winbond.GPIO_Pin = WINBOND_CS;
	gpio_cs_winbond.GPIO_Mode = GPIO_Mode_OUT;
	gpio_cs_winbond.GPIO_OType = GPIO_OType_PP;
	gpio_cs_winbond.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio_cs_winbond.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_cs_winbond);
}

