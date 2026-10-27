// SPDX-License-Identifier: GPL-2.0
/*
 * Host-side stand-in for the linux/watchdog.h ioctl subset elf2-wdctl uses.
 *
 * --wrap catches the ioctl calls, so a plain host executable can run
 * elf2-wdctl against a regular file instead of a real watchdog device.
 * Requests are appended to $ELF2_WDCTL_TEST_LOG when that is set.
 */
#include <errno.h>
#include <pthread.h>
#include <linux/watchdog.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

int __wrap_ioctl(int fd, unsigned long request, ...);
int __wrap_ioctl(int fd, unsigned long request, ...)
{
	const char *actual = getenv("ELF2_WDCTL_TEST_TIMEOUT");
	const char *failure = getenv("ELF2_WDCTL_TEST_FAIL");
	const char *path = getenv("ELF2_WDCTL_TEST_LOG");
	FILE *log = path ? fopen(path, "a") : NULL;
	va_list ap;
	int rc = 0;
	static unsigned kicks;

	(void)fd;
	va_start(ap, request);
	switch (request) {
	case WDIOC_SETTIMEOUT:
		if (failure && !strcmp(failure, "set")) { errno=EINVAL; rc=-1; break; }
		fprintf(log ? log : stdout, "settimeout %d\n",
			*va_arg(ap, int *));
		break;
	case WDIOC_GETTIMEOUT:
		if (failure && !strcmp(failure, "get")) { errno=EIO; rc=-1; break; }
		*va_arg(ap, int *) = actual ? atoi(actual) : 30;
		break;
	case WDIOC_KEEPALIVE:
		if (failure && !strcmp(failure, "later") && kicks++ >= 2) { errno=EIO; rc=-1; break; }
		if (failure && !strcmp(failure, "kick")) { errno=EIO; rc=-1; break; }
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

int __real_pthread_create(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
int __wrap_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                         void *(*start)(void *), void *arg)
{
    const char *failure = getenv("ELF2_WDCTL_TEST_FAIL");
    if (failure && !strcmp(failure, "thread"))
        return EAGAIN;
    return __real_pthread_create(thread, attr, start, arg);
}
