/*
 * system.c
 *
 *  Created on: Dec 7, 2016
 *      Author: shapa
 */

#include "system.h"
#include "stm32f10x.h"

#if 0
static volatile int s_lock;

void System_Lock(void) {
    if (!s_lock++) {
        __disable_irq();
    }
}

void System_Unlock(void) {
    if (s_lock-- <= 1) {
        __enable_irq();
    }
}
#endif

void System_Poll(void) {
    __WFE();
}
void System_Wakeup(void) {
    __SEV();
}

