/*
 * system.c
 *
 *  Created on: Dec 7, 2016
 *      Author: shapa
 */

#include "system.h"
#include "stm32f10x.h"

#include <stdatomic.h>

static atomic_int s_lock;

void System_Lock(void) {
    int lock = atomic_fetch_add(&s_lock, 1);
    if (!lock) {
        __disable_irq();
    }
}

void System_Unlock(void) {
    int lock = atomic_fetch_sub(&s_lock, 1);
    if (lock <= 1) {
        __enable_irq();
    }
}

void System_Poll(void) {
    __WFE();
}
void System_Wakeup(void) {
    __SEV();
}

