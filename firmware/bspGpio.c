/*
 * bspGpio.c
 *
 *  Created on: Jan 19, 2017
 *      Author: shapa
 */

#include <stdlib.h>

#include "bsp.h"
#include "stm32f10x_gpio.h"

typedef struct {
	GPIO_TypeDef *const port;
	const GPIO_InitTypeDef setting;
} BspGpioConfig_t;

//BSP_Pin_DIG_1,
//BSP_Pin_DIG_2,
//BSP_Pin_DIG_3,
//BSP_Pin_DIG_4,
//BSP_Pin_DIG_5,
//BSP_Pin_DIG_6,
//BSP_Pin_DIG_7,
//BSP_Pin_DIG_8,
//BSP_Pin_DIG_9,
//
//BSP_Pin_SEG_A,
//BSP_Pin_SEG_B,
//BSP_Pin_SEG_C,
//BSP_Pin_SEG_D,
//BSP_Pin_SEG_E,
//BSP_Pin_SEG_F,
//BSP_Pin_SEG_G,
//BSP_Pin_SEG_H,

static const BspGpioConfig_t s_gpioConfig[] = {

	[BSP_Pin_DIG_1] = { GPIOA, { GPIO_Pin_0, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_DIG_2] = { GPIOA, { GPIO_Pin_2, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_DIG_3] = { GPIOA, { GPIO_Pin_4, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_DIG_4] = { GPIOA, { GPIO_Pin_6, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_DIG_5] = { GPIOB, { GPIO_Pin_0, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_DIG_6] = { GPIOB, { GPIO_Pin_2, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_DIG_7] = { GPIOB, { GPIO_Pin_11, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_DIG_8] = { GPIOB, { GPIO_Pin_13, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_DIG_9] = { GPIOB, { GPIO_Pin_15, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },

	[BSP_Pin_SEG_A] = { GPIOB, { GPIO_Pin_10, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_SEG_B] = { GPIOA, { GPIO_Pin_3, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_SEG_C] = { GPIOB, { GPIO_Pin_14, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_SEG_D] = { GPIOA, { GPIO_Pin_7, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_SEG_E] = { GPIOB, { GPIO_Pin_1, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_SEG_F] = { GPIOA, { GPIO_Pin_1, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_SEG_G] = { GPIOA, { GPIO_Pin_5, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
	[BSP_Pin_SEG_H] = { GPIOB, { GPIO_Pin_12, GPIO_Speed_2MHz, GPIO_Mode_Out_PP} },
//0
};

void BSP_InitGpio(void) {
	static const size_t size = sizeof(s_gpioConfig)/sizeof(*s_gpioConfig);
	for (size_t i = 0; i < size; i++)
		GPIO_Init((GPIO_TypeDef*)s_gpioConfig[i].port, (GPIO_InitTypeDef*)&s_gpioConfig[i].setting);

	GPIOA->BRR = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
	GPIOB->BRR = GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_12 | GPIO_Pin_14;
}

void BSP_SetPinVal(const BSP_Pin_t pin, const _Bool state) {
	if (pin > BSP_Pin_Last)
		return;
	if (state)
		s_gpioConfig[pin].port->BSRR = s_gpioConfig[pin].setting.GPIO_Pin;
	else
		s_gpioConfig[pin].port->BRR = s_gpioConfig[pin].setting.GPIO_Pin;
}

_Bool BSP_GetPinVal(const BSP_Pin_t pin) {
	if (pin > BSP_Pin_Last)
		return false;
	return !(s_gpioConfig[pin].port->IDR & s_gpioConfig[pin].setting.GPIO_Pin);
}

void BSP_SetLedVal(uint8_t val) {
	GPIOA->BRR = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
	GPIOB->BRR = GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_12 | GPIO_Pin_14;

	switch (val) {
	case '0':
		GPIOA->BSRR = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_7;
		GPIOB->BSRR = GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case '1':
		GPIOA->BSRR = GPIO_Pin_3;
		GPIOB->BSRR = GPIO_Pin_14;
		break;
	case '2':
		GPIOA->BSRR = GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
		GPIOB->BSRR = GPIO_Pin_1 | GPIO_Pin_10;
		break;
	case '3':
		GPIOA->BSRR = GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
		GPIOB->BSRR = GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case '4':
		GPIOA->BSRR = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5;
		GPIOB->BSRR = GPIO_Pin_14;
		break;
	case '5':
		GPIOA->BSRR = GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_7;
		GPIOB->BSRR = GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case '6':
		GPIOA->BSRR = GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_7;
		GPIOB->BSRR = GPIO_Pin_1 | GPIO_Pin_14;
		break;
	case '7':
		GPIOA->BSRR = GPIO_Pin_3;
		GPIOB->BSRR = GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case '8':
		GPIOA->BSRR = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
		GPIOB->BSRR = GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case '9':
		GPIOA->BSRR = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
		GPIOB->BSRR = GPIO_Pin_10 | GPIO_Pin_14;
		break;
	}

}

