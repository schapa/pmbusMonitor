/*
 * bsp.h
 *
 *  Created on: Jan 18, 2017
 *      Author: pavelgamov
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define BSP_TICKS_PER_SECOND 1000
#define MINUTE_TICKS (BSP_TICKS_PER_SECOND*60)

typedef enum {
	BSP_Pin_DIG_1,
	BSP_Pin_DIG_2,
	BSP_Pin_DIG_3,
	BSP_Pin_DIG_4,
	BSP_Pin_DIG_5,
	BSP_Pin_DIG_6,
	BSP_Pin_DIG_7,
	BSP_Pin_DIG_8,
	BSP_Pin_DIG_9,

	BSP_Pin_SEG_A,
	BSP_Pin_SEG_B,
	BSP_Pin_SEG_C,
	BSP_Pin_SEG_D,
	BSP_Pin_SEG_E,
	BSP_Pin_SEG_F,
	BSP_Pin_SEG_G,
	BSP_Pin_SEG_H,

	BPS_Pin_Uart_Tx,
	BPS_Pin_Uart_Rx,

	BSP_Pin_SCL,
	BPS_Pin_SDA,

	BSP_Pin_Last,
} BSP_Pin_t;


_Bool BSP_Init(void);
void BSP_InitGpio(void);

void BSP_FeedWatchdog(void);

void BSP_SetPinVal(const BSP_Pin_t pin, const _Bool state);
_Bool BSP_GetPinVal(const BSP_Pin_t pin);

void BSP_SetLedVal(int pos, uint8_t val, _Bool dot);

void BSP_SetDisplayString(const char *str);

_Bool BSP_i2c_test(int devAdr);

#ifdef __cplusplus
}
#endif
