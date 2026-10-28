// SPDX-License-Identifier: GPL-2.0
/*
 * elf2-wdctl - keep the RK3588 DW watchdog fed from early boot.
 *
 * The bootloader arms the hardware watchdog, so userspace has to open
 * /dev/watchdog and kick it before the first TOP expires.  Once the device is
 * open it must never be left un-kicked: closing the descriptor or exiting
 * either disarms the counter or causes an immediate reset, depending on the
 * nowayout setting of the driver.  The daemon therefore opens the device
 * exactly once, keeps that descriptor for its whole life, and feeds it from a
 * dedicated thread so no other work can delay a kick past the deadline.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/watchdog.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_DEVICE "/dev/watchdog"
#define DEFAULT_TIMEOUT 30u
#define DEFAULT_INTERVAL 10u

static volatile sig_atomic_t stop_requested;
static volatile sig_atomic_t hang_requested;
static volatile sig_atomic_t feed_failed;
static int watchdog_fd = -1;
static const char *device_path = DEFAULT_DEVICE;

static void log_msg(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fputs("elf2-wdctl: ", stderr);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	va_end(ap);
}

static void on_signal(int sig)
{
	if (sig == SIGUSR1)
		hang_requested = 1;
	else
		stop_requested = 1;
}

/* Deadlines only use CLOCK_MONOTONIC: a realtime clock step must not be able
 * to stretch a kick interval past the hardware timeout.
 */
static void deadline_in(struct timespec *ts, unsigned interval_sec)
{
	clock_gettime(CLOCK_MONOTONIC, ts);
	ts->tv_sec += (time_t)interval_sec;
}

static bool reached(const struct timespec *deadline)
{
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec > deadline->tv_sec ||
	       (now.tv_sec == deadline->tv_sec && now.tv_nsec >= deadline->tv_nsec);
}

static void *feeder(void *arg)
{
	struct timespec next;
	unsigned interval = *(const unsigned *)arg;

	while (!stop_requested && !hang_requested) {
		deadline_in(&next, interval);
		if (ioctl(watchdog_fd, WDIOC_KEEPALIVE, 0) < 0) {
			feed_failed = 1;
			stop_requested = 1;
			log_msg("WDIOC_KEEPALIVE on %s: %s", device_path,
				strerror(errno));
			break;
		}
		while (!stop_requested && !hang_requested && !reached(&next)) {
			struct timespec slice;

			deadline_in(&slice, 0);
			slice.tv_nsec += 200L * 1000L * 1000L;
			if (slice.tv_nsec >= 1000000000L) {
				slice.tv_nsec -= 1000000000L;
				slice.tv_sec += 1;
			}
			/* Absolute sleep; shortened by any delivered signal. */
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &slice, NULL);
		}
	}
	return NULL;
}

static int open_watchdog(unsigned timeout, unsigned *interval)
{
	int fd, seconds = (int)timeout;

	fd = open(device_path, O_WRONLY | O_CLOEXEC);
	if (fd < 0)
		return -1;
	if (timeout && ioctl(fd, WDIOC_SETTIMEOUT, &seconds) < 0)
		log_msg("WDIOC_SETTIMEOUT failed: %s; querying current timeout", strerror(errno));
	/* GETTIMEOUT is authoritative, including after SETTIMEOUT failure. */
	if (ioctl(fd, WDIOC_GETTIMEOUT, &seconds) < 0 || seconds < 2) {
		log_msg("cannot determine a safe watchdog timeout");
		close(fd);
		errno = EINVAL;
		return -1;
	}
	if (*interval > (unsigned)seconds / 2)
		*interval = (unsigned)seconds / 2;
	log_msg("actual timeout %ds, kick interval %us", seconds, *interval);
	if (ioctl(fd, WDIOC_KEEPALIVE, 0) < 0) {
		log_msg("initial keepalive failed: %s", strerror(errno));
		close(fd);
		return -1;
	}
	return fd;
}

static bool parse_seconds(const char *value, unsigned *result)
{
	char *end;
	unsigned long parsed;

	if (!value || !*value || *value == '-')
		return false;
	errno = 0;
	parsed = strtoul(value, &end, 10);
	if (errno || *end || parsed > INT_MAX)
		return false;
	*result = (unsigned)parsed;
	return true;
}

static void usage(void)
{
	fputs("Usage: elf2-wdctl [-f] [-d DEVICE] [-t SECONDS] [-i SECONDS]\n"
	      "  -f  stay in the foreground\n"
	      "  -d  watchdog device node (default " DEFAULT_DEVICE ")\n"
	      "  -t  hardware timeout in seconds (default 30)\n"
	      "  -i  kick interval in seconds (default 10, must be < timeout)\n",
	      stderr);
}

int main(int argc, char **argv)
{
	struct sigaction sa;
	pthread_t thread;
	unsigned timeout = DEFAULT_TIMEOUT, interval = DEFAULT_INTERVAL;
	bool foreground = false;
	pid_t child;
	int ready[2] = { -1, -1 };

	while (argc > 1) {
		if (argv[1][0] != '-' || !argv[1][1] || argv[1][2])
			goto bad;
		switch (argv[1][1]) {
		case 'f':
			foreground = true;
			break;
		case 'd':
			if (!argv[2])
				goto bad;
			device_path = argv[2];
			argv++;
			argc--;
			break;
		case 't':
			if (!argv[2])
				goto bad;
			if (!parse_seconds(argv[2], &timeout))
				goto bad;
			argv++;
			argc--;
			break;
		case 'i':
			if (!argv[2])
				goto bad;
			if (!parse_seconds(argv[2], &interval))
				goto bad;
			argv++;
			argc--;
			break;
		default:
			goto bad;
		}
		argv++;
		argc--;
	}
	if (!interval || (timeout && interval >= timeout))
		goto bad;

	watchdog_fd = open_watchdog(timeout, &interval);
	if (watchdog_fd < 0) {
		log_msg("cannot open %s: %s", device_path, strerror(errno));
		return 1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	signal(SIGPIPE, SIG_IGN);

	/* Fork only after validating the device; the child inherits its fd. */
	if (!foreground) {
		if (pipe(ready) < 0) {
			log_msg("readiness pipe: %s", strerror(errno));
			return 1;
		}
		child = fork();
		if (child < 0) {
			log_msg("fork: %s", strerror(errno));
			return 1;
		}
		if (child > 0) {
			char status;
			ssize_t count;

			close(ready[1]);
			do {
				count = read(ready[0], &status, 1);
			} while (count < 0 && errno == EINTR);
			close(ready[0]);
			close(watchdog_fd);
			return count == 1 && status == 'R' ? 0 : 1;
		}
		close(ready[0]);
		if (setsid() < 0) {
			log_msg("setsid: %s", strerror(errno));
			return 1;
		}
	}

	if (pthread_create(&thread, NULL, feeder, &interval) != 0) {
		log_msg("cannot start kick thread");
		return 1;
	}

	if (!foreground) {
		if (write(ready[1], "R", 1) != 1) {
			stop_requested = 1;
			pthread_join(thread, NULL);
			return 1;
		}
		close(ready[1]);
	}

	/* No SA_RESTART: the absolute sleep below must return on a signal so
	 * that stop_requested is observed without waiting a full second.
	 */
	while (!stop_requested && !hang_requested) {
		struct timespec pause;

		clock_gettime(CLOCK_MONOTONIC, &pause);
		pause.tv_sec += 1;
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &pause, NULL);
	}
	if (hang_requested) {
		struct timespec forever = { 0x7fffffff, 0 };

		/* Diagnostics only: stop feeding the device and sit still so the
		 * armed counter can be observed resetting the board.
		 */
		log_msg("SIGUSR1 received, stopping kicks on purpose");
		for (;;)
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &forever, NULL);
	}

	pthread_join(thread, NULL);
	if (feed_failed) {
		/* Leave the card armed so a broken feeder triggers recovery. */
		close(watchdog_fd);
		return 1;
	}
	log_msg("stopping");
	/* Best effort: with nowayout the driver ignores this, which is fine -
	 * the counter then resets the board once the TOP expires.
	 */
	if (ioctl(watchdog_fd, WDIOC_SETOPTIONS, (int[]){ WDIOS_DISABLECARD }) < 0)
		log_msg("WDIOC_SETOPTIONS: %s (device stays armed)", strerror(errno));
	close(watchdog_fd);
	watchdog_fd = -1;
	return 0;

bad:
	usage();
	return 2;
}
