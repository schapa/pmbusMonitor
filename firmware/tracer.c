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
static void sendNextItem(const char *ptr, size_t size);
static void onTxComplete(void);

void Trace_Init(void) {
	static const USART_InitTypeDef iface = {
			115200,
			USART_WordLength_8b,
			USART_StopBits_1,
			USART_Parity_No,
			USART_Mode_Tx,
			USART_HardwareFlowControl_None
	};
	USART_DeInit(USART1);
	USART_Init(USART1, (USART_InitTypeDef*)&iface);
	USART_Cmd(USART1, ENABLE);

	initNVIC();
}

void Trace_dataAsync(char *buff, size_t size) {
	_Bool send = false;

	traceNode_p elt = (traceNode_p)malloc(sizeof(traceNode_t));
	if (elt) {
		elt->next = NULL;
		elt->string = buff;
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

void Trace_dataAsyncFlush(void) {
	if (!s_traceHead)
		return;

	USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
	while (s_traceHead) {
		for (size_t i = 0; i < s_traceHead->size; ++i) {
			USART_SendData(USART1, s_traceHead->string[i]);
			while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		}
		traceNode_p cur = s_traceHead;
		s_traceHead = cur->next;
		free(cur->string);
		free(cur);
	}
	s_traceTail = s_traceHead;
}

void Trace_dataSync(const char *buff, size_t size) {
	if (!buff || !size)
		return;
	Trace_dataAsyncFlush();
	for (size_t i = 0; i < size; ++i) {
		USART_SendData(USART1, buff[i]);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
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
		onTxComplete();
		DMA_ClearITPendingBit(DMA1_IT_TC4);
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
		14,
		0,
		ENABLE
	};
	NVIC_Init((NVIC_InitTypeDef*)&uartPrio);
	NVIC_Init((NVIC_InitTypeDef*)&dmaUartPrio);
}

static void sendNextItem(const char *ptr, size_t size) {
	DMA_InitTypeDef iface = {
			(uint32_t)&(USART1->DR),
			(uint32_t)ptr,
			DMA_DIR_PeripheralDST,
			size,
			DMA_PeripheralInc_Disable,
			DMA_MemoryInc_Enable,
			DMA_PeripheralDataSize_Byte,
			DMA_MemoryDataSize_Byte,
			DMA_Mode_Normal,
			DMA_Priority_Low,
			DMA_M2M_Disable
	};
	DMA_DeInit(DMA1_Channel4);
	DMA_Init(DMA1_Channel4, &iface);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TE, ENABLE);

	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(DMA1_Channel4, ENABLE);
}

static void onTxComplete(void) {
	if (!s_traceHead)
		return;
	System_Lock();
	traceNode_p cur = s_traceHead;
	s_traceHead = cur->next;
	free(cur->string);
	free(cur);
	if (!s_traceHead) {
		s_traceTail = NULL;
	}
	System_Unlock();
	if (s_traceHead) {
		sendNextItem(s_traceHead->string, s_traceHead->size);
	}
}
