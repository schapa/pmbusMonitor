
#include <stdio.h>
#include <stdlib.h>

#include "bsp.h"
#include "Queue.h"
#include "timers.h"
#include "systemTimer.h"

#include "dbg_base.h"
#if 01
#include "dbg_trace.h"
#endif

static inline void onTimerPush(uint32_t id) {
	EventQueue_Push(EVENT_TIMCALL, (void*)id, NULL);
}
extern "C" void Trace_dataSync(const char *buff, size_t size);

int main(int argc, char* argv[]) {

	Timer_init(onTimerPush);
	BSP_Init();

	DBGMSG_INFO("System started");

	static const char *strs[] = {
		"-abcdef",
		"12 66 456",
		"1",
		"22.16..55",
		"33 44",
		"   44",
	};
	size_t pos = 0;
	while (true) {
		Event_t event;
		if (EventQueue_Pend(&event)) {
			for (int i = 0; i < 128; ++i) {
				if (BSP_i2c_test(i))
					DBGMSG_INFO("0x%02X found", i);
			}
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
//				BSP_SetSinBase(intVal);
				break;
			}
			default:
				break;
		}
		EventQueue_Dispose(&event);
	}
	return 0;
}
