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
	if ((ptr >= (void*)&_stack_Limit) || (ptr >= (void*)&_Heap_Limit)) {
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
		size = snprintf(val, sizeof(val), "%08X\r\n", (int)ptr);
		Trace_dataSync(val, size);
		HardFault_Handler();
		ptr = NULL;
	}
#endif
	System_Unlock();
	return ptr;
}

void __wrap_free(void *ptr) {
	System_Lock();
	__real_free(ptr);
	System_Unlock();
}
