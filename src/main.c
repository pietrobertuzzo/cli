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

#define MAX_STRLEN 10

volatile char rxBuffer[MAX_STRLEN + 2];
volatile uint8_t CMD_FLAG = 0;
char cli_prompt[];
static void cli_print(cli_t *cli, const char *msg);
static void echo_func(int argc, char **argv);
static void help_func(int argc, char **argv);
static void nop_func(int argc, char **argv);
void simple_delay(uint32_t us);
uint8_t Serial_GetByte(USART_TypeDef *USARTx);
void Serial_PutByte(USART_TypeDef *USARTx, uint8_t byte);
uint8_t Serial_PutPacket(USART_TypeDef *USARTx, uint8_t *data);
uint8_t Serial_PutString(USART_TypeDef *USARTx, char *p_string);
void USART_puts(USART_TypeDef *USARTx, volatile char *str);
void Peripheral_Init(void);
void user_uart_println(char *string);


cmd_t cmd_tbl[] = {
    {
        .cmd = "help",
        .func = help_func
    },
    {
        .cmd = "echo",
        .func = echo_func
    },
	{
		.cmd = "nop",
		.func = nop_func
	}
};


cli_t cli;

int main(void)
{
	
	Peripheral_Init();
	cli.println = user_uart_println;
	cli.cmd_tbl = cmd_tbl;
	cli.cmd_cnt = sizeof(cmd_tbl)/sizeof(cmd_t);
	
	USART_puts(USART1, "System started.\r\n");

	cli_init(&cli);
	

	/* main program loop */
	for (;;) {
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
	uint8_t cnt;
	for(cnt = 0;cnt < sizeof(cmd_tbl); cnt++){
		cli_print(&cli, cmd_tbl[cnt].cmd);
		cli_print(&cli, ", ");
	}
	cli_print(&cli, "\r\n");
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

static void nop_func(int argc, char **argv){
	asm volatile ("nop");
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
