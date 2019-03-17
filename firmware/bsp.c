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

static void initI2c(void);
static void initI2cNvic(void);

_Bool BSP_Init(void) {
	initialize_RCC();
	initWdt();
	BSP_InitGpio();
	System_init(NULL);
	Trace_Init();

	initI2c();
	initI2cNvic();

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

static void initI2c(void) {
	static const I2C_InitTypeDef config = {
		100 * 1000,
		I2C_Mode_I2C,
		I2C_DutyCycle_2,
		0x0,
		I2C_Ack_Disable,
		I2C_AcknowledgedAddress_7bit
	};
	I2C_DeInit(I2C1);
	I2C_Init(I2C1, (I2C_InitTypeDef*)&config);

//	I2C_ITConfig(I2C1, I2C_IT_BUF, ENABLE);
//	I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
//	I2C_ITConfig(I2C1, I2C_IT_ERR, ENABLE);
	I2C_Cmd(I2C1, ENABLE);
}

#define I2C_TOUT 0xFF

_Bool BSP_i2c_test(int devAdr) {
	_Bool res = false;
	int tout = I2C_TOUT;
	do {
		I2C_GenerateSTART(I2C1, ENABLE);
		while (--tout && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
		if (!tout)
			break;
		tout = I2C_TOUT;
		I2C_Send7bitAddress(I2C1, devAdr << 1, I2C_Direction_Transmitter);
		while (--tout && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
		if (!tout)
			break;
		res = true;
	} while (0);

	tout = I2C_TOUT;
	I2C_GenerateSTOP(I2C1, ENABLE);
	while (--tout && I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	return res;
}

void I2C1_EV_IRQHandler(void) {
	char flags[128];
	int occup = 0;
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_DUALF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "DUALF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBHOST))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBHOST ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBDEFAULT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBDEFAULT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_GENCALL))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "GENCALL ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TRA))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TRA ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BUSY ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_MSL)) {
		occup += snprintf(flags + occup, sizeof(flags) - occup, "MSL ");
		I2C_ClearFlag(I2C1, I2C_FLAG_MSL);
	}
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBALERT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBALERT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TIMEOUT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TIMEOUT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_PECERR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "PECERR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_OVR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "OVR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "AF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ARLO))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ARLO ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BERR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BERR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TXE ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "RXNE ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "STOPF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADD10))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ADR10 ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BTF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ADDR ");
	DBGMSG_WARN(flags);
}
void I2C1_ER_IRQHandler(void) {
	char flags[128];
	int occup = 0;
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_DUALF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "DUALF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBHOST))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBHOST ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBDEFAULT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBDEFAULT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_GENCALL))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "GENCALL ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TRA))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TRA ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BUSY ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_MSL))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "MSL ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBALERT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBALERT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TIMEOUT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TIMEOUT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_PECERR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "PECERR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_OVR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "OVR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "AF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ARLO))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ARLO ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BERR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BERR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TXE ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "RXNE ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "STOPF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADD10))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ADR10 ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BTF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ADDR ");
	DBGMSG_WARN(flags);
}


static void initI2cNvic(void) {
	static const NVIC_InitTypeDef event = {
		I2C1_EV_IRQn,
		8, 0,
		ENABLE
	};
	static const NVIC_InitTypeDef error = {
		I2C1_ER_IRQn,
		8, 0,
		ENABLE
	};
	NVIC_Init((NVIC_InitTypeDef*)&event);
	NVIC_Init((NVIC_InitTypeDef*)&error);
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

