/*
 * bsp.c
 *
 *  Created on: May 18, 2016
 *      Author: shapa
 */

#include "bsp.h"
#include "system.h"
#include "systemTimer.h"
#include "Queue.h"
#include "timers.h"
#include "tracer.h"
#include "i2c.h"

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_tim.h"

#include "stm32f10x_i2c.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "dbg_base.h"
#if 01
#include "dbg_trace.h"
#endif

#define LED_CONT 9

static struct {
	uint8_t codes[LED_CONT * 2];
	int codePos;
	int digitPos;
} s_ledDisplay;

static inline void initialize_RCC(void);
static inline void initWdt(void);
static void initTim4(void);
static inline void initTim4Nvic(void);

_Bool BSP_Init(void) {
	initialize_RCC();
	initWdt();
	BSP_InitGpio();
	System_init(NULL);
	Trace_Init();

	I2C_init();

	for (int i = 0; i < LED_CONT; ++i)
		s_ledDisplay.codes[i] = 0x31 + i;
	s_ledDisplay.codes[5] = ' ';

	initTim4();
	initTim4Nvic();

	return true;
}

void BSP_FeedWatchdog(void) {
	IWDG_ReloadCounter();
}

void BSP_SetDisplayString(const char *str) {
	size_t len = strlen(str) + 1;
	const size_t size = len > sizeof(s_ledDisplay.codes) ? sizeof(s_ledDisplay.codes) : len;
	memcpy(s_ledDisplay.codes, str, size);
}

static void initialize_RCC(void) {
	RCC_LSEConfig(RCC_LSE_OFF);
	RCC_HSEConfig(RCC_HSE_OFF);
	RCC_WaitForHSEStartUp(); // really we wait for shutdown

	RCC->APB2ENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_USART1;
	RCC->APB1ENR |= RCC_APB1Periph_TIM4 | RCC_APB1Periph_I2C1;

	RCC->AHBENR |= RCC_AHBPeriph_DMA1;
}

static void initWdt(void) {
//	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
//	IWDG_SetPrescaler(IWDG_Prescaler_32);
//	IWDG_SetReload(0x0FFF);
//	IWDG_ReloadCounter();
//	IWDG_Enable();
}

static void initTim4(void) {
	static const TIM_TimeBaseInitTypeDef iface = {
		0x3F,
        TIM_CounterMode_Up,
        0xFF,
        TIM_CKD_DIV1,
        0
	};

	TIM_Cmd(TIM4, DISABLE);
	TIM_TimeBaseInit(TIM4, (TIM_TimeBaseInitTypeDef*)&iface);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
}

static void initTim4Nvic(void) {
	static const NVIC_InitTypeDef nvic = {
		TIM4_IRQn,
		15, 0,
		ENABLE
	};
	NVIC_Init((NVIC_InitTypeDef*)&nvic);
}

void TIM4_IRQHandler(void) {
	TIM_ClearFlag(TIM4, TIM_IT_Update);
	uint8_t val = s_ledDisplay.codes[s_ledDisplay.codePos++];
	if (val == '\0') {
		s_ledDisplay.digitPos = 0;
		s_ledDisplay.codePos = 0;
		val = s_ledDisplay.codes[s_ledDisplay.codePos++];
	}
	_Bool isDot = (val == '.' || val == ',');
	if (s_ledDisplay.codePos < sizeof(s_ledDisplay.codes)) {
		const uint8_t next = s_ledDisplay.codes[s_ledDisplay.codePos];
		if (next == '.' || next == ',') {
			isDot |= true;
			++s_ledDisplay.codePos;
		}
	}
	BSP_SetLedVal(LED_CONT - s_ledDisplay.digitPos - 1, val, isDot);
	if (++s_ledDisplay.digitPos >= LED_CONT) {
		s_ledDisplay.digitPos = 0;
		s_ledDisplay.codePos = 0;
	}
}

