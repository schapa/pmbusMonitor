/*
 * system.h
 *
 *  Created on: Dec 7, 2016
 *      Author: shapa
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if 0
void System_Lock(void);
void System_Unlock(void);
#else
#define System_Lock(...)
#define System_Unlock(...)
#endif
void System_Poll(void);
void System_Wakeup(void);

#ifdef __cplusplus
}
#endif
