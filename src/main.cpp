
#include <stdio.h>
#include <stdlib.h>

#include "i2c.h"
#include "bsp.h"
#include "Queue.h"
#include "timers.h"
#include "tracer.h"
#include "systemTimer.h"

#include "dbg_base.h"
#if 01
#include "dbg_trace.h"
#endif

static inline void onTimerPush(uint32_t id) {
	EventQueue_Push(EVENT_TIMCALL, (void*)id, NULL);
}

int main(int argc, char* argv[]) {

	Timer_init(onTimerPush);
	BSP_Init();

	static const char *strs[] = {
		"-abcdef",
		"12 66 456",
		"1",
		"22.16..55",
		"33 44",
		"   44",
	};
	size_t pos = 0;
	DBGMSG_INFO("System started");
	DBGMSG_INFO("yes");
	while (true) {
		Event_t event;
		if (EventQueue_Pend(&event) && ((uint32_t)event.data > 5)) {

			for (int i = 0; i < 256; ++i) {
				uint16_t val = 0;
				SMBus_ReadWord(0x53 << 1, &val, i);
				DBGMSG_INFO("0x%02X 0x%04X", i, val);
			}
//			for (int i = 0; i < 128; ++i) {
//				if (BSP_i2c_test(i))
//					DBGMSG_INFO("0x%02X found", i);
//			}
		}
		BSP_FeedWatchdog();
		uint32_t intVal = (uint32_t)event.data;
		switch (event.type) {
			case EVENT_SYSTICK:
				BSP_SetDisplayString(strs[pos]);
				if (++pos >= 6)
					pos = 0;
				break;
			case EVENT_TIMCALL:
				Timer_onTimerCb(intVal);
				break;
			case EVENT_ADC: {
				break;
			}
			default:
				break;
		}
		EventQueue_Dispose(&event);
	}
	return 0;
}
