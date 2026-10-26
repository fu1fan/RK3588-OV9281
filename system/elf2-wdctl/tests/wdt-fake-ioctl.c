// SPDX-License-Identifier: GPL-2.0
/*
 * Host-side stand-in for the linux/watchdog.h ioctl subset elf2-wdctl uses.
 *
 * --wrap catches the ioctl calls, so a plain host executable can run
 * elf2-wdctl against a regular file instead of a real watchdog device.
 * Requests are appended to $ELF2_WDCTL_TEST_LOG when that is set.
 */
#include <errno.h>
#include <linux/watchdog.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

int __wrap_ioctl(int fd, unsigned long request, ...);
int __wrap_ioctl(int fd, unsigned long request, ...)
{
	const char *path = getenv("ELF2_WDCTL_TEST_LOG");
	FILE *log = path ? fopen(path, "a") : NULL;
	va_list ap;
	int rc = 0;

	(void)fd;
	va_start(ap, request);
	switch (request) {
	case WDIOC_SETTIMEOUT:
		fprintf(log ? log : stdout, "settimeout %d\n",
			*va_arg(ap, int *));
		break;
	case WDIOC_GETTIMEOUT:
		*va_arg(ap, int *) = 30;
		break;
	case WDIOC_KEEPALIVE:
		fprintf(log ? log : stdout, "kick\n");
		break;
	case WDIOC_SETOPTIONS:
		fprintf(log ? log : stdout, "setoptions %d\n",
			*va_arg(ap, int *));
		break;
	case WDIOC_GETSTATUS:
		*va_arg(ap, int *) = 0;
		break;
	default:
		errno = ENOTTY;
		rc = -1;
		break;
	}
	va_end(ap);
	if (log)
		fclose(log);
	return rc;
}
