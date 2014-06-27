/*
 * Copyright (C) 2002-2003 Ardis Technolgies <roman@ardistech.com>
 *
 * Released under the terms of the GNU GPL v2.0.
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/time.h>

int log_daemon = 1;
int log_level = 0;

void log_init(void)
{
	if (log_daemon)
		openlog("dualcontrl", 0, LOG_LOCAL0);
}

static void dolog(int prio, const char *fmt, va_list ap)
{
	if (log_daemon)
		vsyslog(prio, fmt, ap);
	else {
		struct timeval time;

		gettimeofday(&time, NULL);
		fprintf(stderr, "%ld.%06ld: ", time.tv_sec, time.tv_usec);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
		fflush(stderr);
	}
}

void log_warning(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	dolog(LOG_WARNING, fmt, ap);
	va_end(ap);
}

void log_error(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	dolog(LOG_ERR, fmt, ap);
	va_end(ap);
}

void log_debug(int level, const char *fmt, ...)
{
	if (log_level >= level) {
		va_list ap;
		va_start(ap, fmt);
		dolog(LOG_DEBUG, fmt, ap);
		va_end(ap);
	}
}

