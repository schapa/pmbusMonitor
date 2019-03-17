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

	[BPS_Pin_Uart_Tx] = { GPIOA, { GPIO_Pin_9, GPIO_Speed_2MHz, GPIO_Mode_AF_PP} },
	[BPS_Pin_Uart_Rx] = { GPIOA, { GPIO_Pin_10, GPIO_Speed_2MHz, GPIO_Mode_IPU} },

	[BSP_Pin_SCL] = { GPIOB, { GPIO_Pin_8, GPIO_Speed_2MHz, GPIO_Mode_AF_OD} },
	[BPS_Pin_SDA] = { GPIOB, { GPIO_Pin_9, GPIO_Speed_2MHz, GPIO_Mode_AF_OD} },
//0
};

void BSP_InitGpio(void) {
	static const size_t size = sizeof(s_gpioConfig)/sizeof(*s_gpioConfig);
	for (size_t i = 0; i < size; i++)
		GPIO_Init((GPIO_TypeDef*)s_gpioConfig[i].port, (GPIO_InitTypeDef*)&s_gpioConfig[i].setting);

	GPIOA->BRR = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
	GPIOB->BRR = GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_12 | GPIO_Pin_14;

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE); //pb8, pb9
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

void BSP_SetLedVal(int pos, uint8_t val, _Bool dot) {

	static const uint32_t maskA0 = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
	static const uint32_t maskA1 = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6;
	static const uint32_t maskB0 = GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_12 | GPIO_Pin_14;
	static const uint32_t maskB1 = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_15;

	uint32_t pa = (GPIOA->ODR & ~maskA0) | maskA1;
	uint32_t pb = (GPIOB->ODR & ~maskB0) | maskB1;

	const BspGpioConfig_t *cfg = s_gpioConfig + BSP_Pin_DIG_1 + pos;
	const uint32_t pinMask = ~(cfg->setting.GPIO_Pin);
	if (cfg->port == GPIOA)
		pa &= pinMask;
	else
		pb &= pinMask;
	if (dot) {
		static const BspGpioConfig_t *dotCfg = s_gpioConfig + BSP_Pin_SEG_H;
		pb |= dotCfg->setting.GPIO_Pin;
	}

	switch (val) {
	case 'a':
		pa |= GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5;
		pb |= GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case 'b':
		pa |= GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_7;
		pb |= GPIO_Pin_1 | GPIO_Pin_14;
		break;
	case 'c':
		pa |= GPIO_Pin_1 | GPIO_Pin_7;
		pb |= GPIO_Pin_1 | GPIO_Pin_10;
		break;
	case 'd':
		pa |= GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
		pb |= GPIO_Pin_1 | GPIO_Pin_14;
		break;
	case 'e':
		pa |= GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_7;
		pb |= GPIO_Pin_1 | GPIO_Pin_10;
		break;
	case 'f':
		pa |= GPIO_Pin_1 | GPIO_Pin_5;
		pb |= GPIO_Pin_1 | GPIO_Pin_10;
		break;
	case '-':
		pa |= GPIO_Pin_5;
		break;
	case ' ':
		break;
	case '0':
		pa |= GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_7;
		pb |= GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case '1':
		pa |= GPIO_Pin_3;
		pb |= GPIO_Pin_14;
		break;
	case '2':
		pa |= GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
		pb |= GPIO_Pin_1 | GPIO_Pin_10;
		break;
	case '3':
		pa |= GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
		pb |= GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case '4':
		pa |= GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5;
		pb |= GPIO_Pin_14;
		break;
	case '5':
		pa |= GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_7;
		pb |= GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case '6':
		pa |= GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_7;
		pb |= GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case '7':
		pa |= GPIO_Pin_3;
		pb |= GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case '8':
		pa |= GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
		pb |= GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_14;
		break;
	case '9':
		pa |= GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7;
		pb |= GPIO_Pin_10 | GPIO_Pin_14;
		break;
	}

	GPIOA->ODR = pa;
	GPIOB->ODR = pb;
}

