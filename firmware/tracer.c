/*
 * tracer.c
 *
 *  Created on: May 6, 2016
 *      Author: shapa
 */

#include "tracer.h"
#include "system.h"
#include "stm32f10x_usart.h"

#include <string.h>
#include <stdlib.h>

typedef struct traceNode {
	struct traceNode *next;
	char *string;
	size_t size;
} traceNode_t, *traceNode_p;

static traceNode_p s_traceHead = NULL;
static traceNode_p s_traceTail = NULL;

void USART1_IRQHandler(void);
void DMA1_Channel4_IRQHandler(void);

static void initNVIC(void);
static void inline sendNextItem(const char *ptr, size_t size);
static void inline onTxComplete(void);

void Trace_Init(void) {
	static const USART_InitTypeDef iface = {
			921600,
			USART_WordLength_8b,
			USART_StopBits_1,
			USART_Parity_No,
			USART_Mode_Tx,
			USART_HardwareFlowControl_None
	};
	USART_DeInit(USART1);
	USART_Init(USART1, (USART_InitTypeDef*)&iface);
	USART_Cmd(USART1, ENABLE);

	static const DMA_InitTypeDef dmaCfg = {
			(uint32_t)&(USART1->DR),
			(uint32_t)"",
			DMA_DIR_PeripheralDST,
			1,
			DMA_PeripheralInc_Disable,
			DMA_MemoryInc_Enable,
			DMA_PeripheralDataSize_Byte,
			DMA_MemoryDataSize_Byte,
			DMA_Mode_Normal,
			DMA_Priority_Low,
			DMA_M2M_Disable
	};
	DMA_DeInit(DMA1_Channel4);
	DMA_Init(DMA1_Channel4, (DMA_InitTypeDef*)&dmaCfg);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TE, ENABLE);

	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
//	DMA_Cmd(DMA1_Channel4, ENABLE);
	initNVIC();
}

void Trace_dataAsync(const char *buff, size_t size) {
	_Bool send = false;

	traceNode_p elt = (traceNode_p)malloc(sizeof(traceNode_t));
	if (elt) {
		elt->next = NULL;
		elt->string = (char*)buff;
		elt->size = size;
		System_Lock();
		if (!s_traceHead) {
			s_traceHead = s_traceTail = elt;
			send = true;
		} else {
			s_traceTail->next = elt;
			s_traceTail = elt;
		}
		System_Unlock();
	}

	if (send) {
		sendNextItem(buff, size);
	}
}

static inline void flushLocked(void) {
	uint16_t cnt = s_traceHead->size - DMA_GetCurrDataCounter(DMA1_Channel4);
	while (s_traceHead && s_traceHead->size) {
		sendNextItem(s_traceHead->string + cnt, s_traceHead->size - cnt);
		while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);
		DMA_ClearFlag(DMA1_FLAG_TC4);
		cnt = 0;
		traceNode_p cur = s_traceHead;
		s_traceHead = cur->next;
		free(cur->string);
		free(cur);
	}
	s_traceTail = s_traceHead;
}

_Bool Trace_dataAsyncFlush(void) {
	if (!s_traceHead)
		return false;

	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, DISABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TE, DISABLE);
	flushLocked();
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TE, ENABLE);
    return true;
}

void Trace_dataSync(const char *buff, size_t size) {
	if (!buff || !size)
		return;

	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, DISABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TE, DISABLE);
	DMA_ClearFlag(DMA1_FLAG_TC4);
	flushLocked();
	sendNextItem(buff, size);
	while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);
	DMA_ClearFlag(DMA1_FLAG_TC4);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TE, ENABLE);
}

void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1, USART_IT_TC)) {
		USART_ClearITPendingBit(USART1, USART_IT_TC);
	}
	if (USART_GetITStatus(USART1, USART_IT_RXNE)) {
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
	if (USART_GetITStatus(USART1, USART_IT_LBD)) {
		USART_ClearITPendingBit(USART1, USART_IT_LBD);
	}
	if (USART_GetITStatus(USART1, USART_IT_CTS)) {
		USART_ClearITPendingBit(USART1, USART_IT_CTS);
	}
}

void DMA1_Channel4_IRQHandler(void) {
	if (DMA_GetITStatus(DMA1_IT_TC4)) { // transfer complete
		DMA_ClearITPendingBit(DMA1_IT_TC4);
		onTxComplete();
		return;
	}
	if (DMA_GetITStatus(DMA1_IT_HT4)) { // Half transfer complete
		DMA_ClearITPendingBit(DMA1_IT_HT4);
	}
	if (DMA_GetITStatus(DMA1_IT_TE4)) { // Error occurred
		DMA_ClearITPendingBit(DMA1_IT_TE4);
	}
	if (DMA_GetITStatus(DMA1_IT_GL4)) { // Global interrupt
		DMA_ClearITPendingBit(DMA1_IT_GL4);
	}
}

static void initNVIC(void) {
	static const NVIC_InitTypeDef uartPrio = {
		USART1_IRQn,
		10,
		0,
		ENABLE
	};
	static const NVIC_InitTypeDef dmaUartPrio = {
		DMA1_Channel4_IRQn,
		5,
		0,
		ENABLE
	};
	NVIC_Init((NVIC_InitTypeDef*)&uartPrio);
	NVIC_Init((NVIC_InitTypeDef*)&dmaUartPrio);
}

static void inline sendNextItem(const char *ptr, size_t size) {
	DMA1_Channel4->CCR &= (uint16_t)(~DMA_CCR1_EN);
	DMA1_Channel4->CMAR = (uint32_t)ptr;
	DMA1_Channel4->CNDTR = size;
	DMA1_Channel4->CCR |= DMA_CCR1_EN;
}

static void inline onTxComplete(void) {
	if (!s_traceHead)
		return;
	System_Lock();
	traceNode_p cur = s_traceHead;
	s_traceHead = cur->next;
	free(cur->string);
	free(cur);
	if (s_traceHead)
		sendNextItem(s_traceHead->string, s_traceHead->size);
	 else
		s_traceTail = NULL;
	System_Unlock();
}
