/*
 * memman.c
 *
 *  Created on: Apr 21, 2016
 *      Author: shapa
 */

#include <stdlib.h>
#include <stdio.h>
#include "system.h"
#include "tracer.h"

void * __real_malloc(size_t size);
void __real_free(void *__ptr);

void HardFault_Handler(void);

void *__wrap_malloc(size_t size) {
	System_Lock();
    char _stack_Limit;
	void *ptr = __real_malloc(size);
#ifndef EMULATOR
    extern char _Heap_Limit;
    int end = (int)ptr + size;
	if ((end >= (int)&_stack_Limit) || (end >= (int)&_Heap_Limit)) {
		static const char head[] = "\r\nFATAL: stack smashed. Stack Limit 0x";
		static const char heap[] = "\r\n                       Heap Limit 0x";
		static const char fmt[] =  "\r\n                              ptr 0x";
		__real_free(ptr);
		char val[12];
		Trace_dataSync(head, sizeof(head) -1);
		size = snprintf(val, sizeof(val), "%08X", (int)&_stack_Limit);
		Trace_dataSync(val, size);

		Trace_dataSync(heap, sizeof(heap) -1);
		size = snprintf(val, sizeof(val), "%08X", (int)&_Heap_Limit);
		Trace_dataSync(val, size);

		Trace_dataSync(fmt, sizeof(fmt) -1);
		size = snprintf(val, sizeof(val), "%08X\r\n", (int)end);
		Trace_dataSync(val, size);
//		HardFault_Handler();
		ptr = NULL;
	}
#endif
//	char val[16];
//	size = snprintf(val, sizeof(val), "\r\n;0x%08X;\r\n", (int)ptr);
//	Trace_dataSync(val, size);
	System_Unlock();
	return ptr;
}

void __wrap_free(void *ptr) {
	System_Lock();
	if ((int)ptr < 0x20000000 || (int)ptr > (0x20000000 + 8 * 1024)) {
		static const char fmt[] =  "\r\n Freeing improper ptr 0x";
		Trace_dataSync(fmt, sizeof(fmt) -1);
		char val[12];
		size_t size = snprintf(val, sizeof(val), "%08X\r\n", (int)ptr);
		Trace_dataSync(val, size);
	}
	__real_free(ptr);
	System_Unlock();
}
