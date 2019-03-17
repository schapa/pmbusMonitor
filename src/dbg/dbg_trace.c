/*
 * dbg_trace.c
 *
 *  Created on: May 6, 2016
 *      Author: shapa
 */

#include "dbg_trace.h"
#include "system.h"
#include "systemTimer.h"

#include <stdlib.h> // malloc
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define MAX_TRACE_LEN 1024
#define CHECK(x,y) x > y ? x - y : 0

void Trace_dataAsync(char *buff, size_t size);

void dbgmsg(const char *color, const char *siverity, const char *file, const char *func, int line, const char *fmt, ...) {
	size_t bufferSize = MAX_TRACE_LEN;
	char *buff = NULL;
	while (!buff && bufferSize) {
		buff = malloc(bufferSize);
		if (!buff)
			bufferSize /= 2;
	}
	if (!buff)
		return;
	int occupied = 0;
	if (line) {
		occupied = snprintf(buff, bufferSize, "[%4lu.%03lu] %s::%s (%d)%s %s: ",
				System_getUptime(), System_getUptimeMs(), file, func, line, color, siverity);
	} else {
		occupied = snprintf(buff, bufferSize, "[%4lu.%03lu] %s ",
				System_getUptime(), System_getUptimeMs(), color);
	}
	va_list ap;
	va_start (ap, fmt);
	occupied += vsnprintf(buff ? &buff[occupied] : NULL, CHECK(bufferSize, occupied), fmt, ap);
	va_end (ap);
	occupied += snprintf(buff ? &buff[occupied] : NULL, CHECK(bufferSize, occupied), ANSI_ESC_DEFAULT"\r\n");

	if (occupied > bufferSize) {
		occupied = bufferSize;
		char *trim = "...\r\n";
		size_t size = strlen(trim) + 1;
		snprintf(&buff[bufferSize-size], size, trim);
	} else
		buff = realloc(buff, occupied);

	Trace_dataAsync(buff, occupied);
}
