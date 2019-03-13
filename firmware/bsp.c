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
#include "Trace.h"

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_tim.h"

#include <stddef.h>
#include <stdlib.h>
#include <math.h>

static uint8_t s_led[9];
static int s_ledPos = 0;

static inline void initialize_RCC(void);
static inline void initWdt(void);
static void initTim4(void);
static inline void initTim4Nvic(void);

_Bool BSP_Init(void) {
	initialize_RCC();
	initWdt();
	BSP_InitGpio();
	System_init(NULL);
	System_setStatus(INFORM_IDLE);

	initTim4Nvic();

	initTim4();

	for (int i = 0; i < sizeof(s_led); ++i)
		s_led[i] = 0x31 + i;
	return true;
}

void BSP_FeedWatchdog(void) {
	IWDG_ReloadCounter();
}

static void initialize_RCC(void) {
	RCC_HSEConfig(RCC_HSE_OFF);
	RCC_WaitForHSEStartUp(); // really we wait for shutdown

	RCC->APB2ENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO;
	RCC->APB1ENR |= RCC_APB1Periph_TIM4;
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
		0x1F,
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
		0, 0,
		ENABLE
	};
	NVIC_Init((NVIC_InitTypeDef*)&nvic);
}

static void inline setPos(int pos, uint8_t val) {
	if (!pos)
		BSP_SetPinVal(BSP_Pin_DIG_9, 1);
	else
		BSP_SetPinVal(BSP_Pin_DIG_1 + pos - 1, 1);
	BSP_SetLedVal(val);

	BSP_SetPinVal(BSP_Pin_DIG_1 + pos, 0);
}

void TIM4_IRQHandler(void) {
	TIM_ClearFlag(TIM4, TIM_IT_Update);
	setPos(s_ledPos, s_led[sizeof(s_led) - 1 - s_ledPos]);
	if (++s_ledPos >= sizeof(s_led))
		s_ledPos = 0;

}

