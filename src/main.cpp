
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
	while (true) {
		Event_t event;
		EventQueue_Pend(&event);
		if ((uint32_t)event.data % 5 == 0) {
			DBGMSG_INFO("Detect");
			for (int i = 0; i < 8; ++i) {
				char buff[128];
				int ocp = 0;
				for (int j = 0; j < 16; ++j) {
					uint8_t val = 0;
					int rv = I2C_burst_read((16 * i + j) << 1, 1, &val, 1);
					if (rv)
						ocp += snprintf(buff + ocp, sizeof(buff) - ocp, " - ");
					else
						ocp += snprintf(buff + ocp, sizeof(buff) - ocp, "%02X ", val);
				}
				DBGMSG_INFO(buff, NULL);
			}
		} else if ((uint32_t)event.data % 6 == 0) {
			DBGMSG_INFO("Dump");
			for (int i = 0; i < 16; ++i) {
				char buff[128];
				int ocp = 0;
				for (int j = 0; j < 16; ++j) {
					uint8_t val = 0;
	//				SMBus_ReadWord(0x53 << 1, &val, i);
					int rv = I2C_burst_read(0x53 << 1, 16 * i + j, &val, 1);
					if (rv) {
						DBGMSG_ERR("fail %d", 16 * i + j);
						break;
					}
					ocp += snprintf(buff + ocp, sizeof(buff) - ocp, "%02X ", val);
				}
				DBGMSG_INFO(buff, NULL);
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
				break;
			}
			default:
				break;
		}
		EventQueue_Dispose(&event);
	}
	return 0;
}
