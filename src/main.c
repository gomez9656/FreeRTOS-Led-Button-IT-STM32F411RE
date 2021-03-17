/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

#define TRUE			1
#define FALSE			0
#define	NOT_PRESSED		FALSE
#define PRESSED			TRUE

static void prvSetupUart(void);
static void prvSetupGPIO(void);
static void prvSetupHardware(void);
void printmsg(char *msg);
char usr_msg[250];

void led_task_handler(void *params);
void button_handler(void *params);

uint8_t button_status_flag = NOT_PRESSED;

int main(void)
{

	//1. Reset the RCC clock configuration to the default reset state.
	//HSI on, PLL off, HSE off, CPU clock = 16MhZ
	RCC_DeInit();

	//2. Update the SystemCoreClock variable
	SystemCoreClockUpdate();

	prvSetupHardware();

	//Create tasks
	xTaskCreate(led_task_handler,
				"LED task",
				configMINIMAL_STACK_SIZE,
				NULL,
				1,
				NULL);

	//Start scheduler
	vTaskStartScheduler();

	for(;;);
}

void led_task_handler(void *params){

	while(1){
		if(button_status_flag == PRESSED){

			//turn on the LED
			GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);


		} else{

			//turn off the LED
			GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
		}
	}
}

void button_handler(void *params){


}

static void prvSetupHardware(void){

	//Setup Button and LED
	prvSetupGPIO();

	//Setup UART
	prvSetupUart();

}

static void prvSetupGPIO(void){

	GPIO_InitTypeDef led_init, button_init;

	//Enable the GPIOA, GPIOC and SYSCFG peripheral clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);


	led_init.GPIO_Mode = GPIO_Mode_OUT;
	led_init.GPIO_OType = GPIO_OType_PP;
	led_init.GPIO_Pin = GPIO_Pin_5; //LED onboard
	led_init.GPIO_Speed = GPIO_Low_Speed;
	led_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOA, &led_init);

	button_init.GPIO_Mode = GPIO_Mode_IN;
	button_init.GPIO_OType = GPIO_OType_PP;
	button_init.GPIO_Pin = GPIO_Pin_13; //Button onboard
	button_init.GPIO_Speed = GPIO_Low_Speed;
	button_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOC, &button_init);

	//Interrupt configuration for button(PC13)
	//1. System configuration for EXTI line
	SYSCFG_EXTILineConfig(GPIOC, EXTI_PinSource13);

	//2. EXTI line configuration
	EXTI_InitTypeDef exti_init;
	exti_init.EXTI_Line = EXTI_Line13;
	exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init.EXTI_Trigger = EXTI_Trigger_Falling;
	exti_init.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti_init);

	//3. NVIC settings for the 13 EXTI line
	NVIC_SetPriority(EXTI15_10_IRQn, 5);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void prvSetupUart(void){

	GPIO_InitTypeDef gpio_uart_pins;
	USART_InitTypeDef uart2_init;

	//Enable the UART2 and GPIOA peripheral clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	//PA2 is Tx. PA3 is Rx
	//Alternate function configuration of MCU to behave as UART2

	memset(&gpio_uart_pins, 0, sizeof(gpio_uart_pins));

	gpio_uart_pins.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	gpio_uart_pins.GPIO_Mode = GPIO_Mode_AF;
	gpio_uart_pins.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &gpio_uart_pins);

	//AF mode setting
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); //PA2
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); //PA3

	//UART parameter initializations

	memset(&uart2_init, 0, sizeof(uart2_init));

	uart2_init.USART_BaudRate = 115200;
	uart2_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart2_init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	uart2_init.USART_Parity = USART_Parity_No;
	uart2_init.USART_StopBits = USART_StopBits_1;
	uart2_init.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &uart2_init);

	//Enable the UART2 peripheral
	USART_Cmd(USART2, ENABLE);
}

void printmsg(char *msg){

	for(uint32_t i = 0; i < strlen(msg); i++){

		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
		USART_SendData(USART2, msg[i]);
	}
}
