#define _GNU_SOURCE
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <sys/resource.h>

#include "ipcperf.h"

int main(int argc, char* argv[]) {
	int ret = 0;
	parse_args(argc, argv);

	int* x = (int*)create_shm(sizeof(int) * (COUNT + 1));
	if (!x) {
		return 1;
	}

	int child = fork();
	if (child == -1) {
		perror("fork");
		return 1;
	} else if (child == 0) {
		int transact_fd = open("transact", O_RDWR);
		if (transact_fd == -1) {
			perror("child: open transact");
			return 1;
		}

		unsigned long long message;
		int i = 0, j, res;

		// Synchronize.
		message = 0;
		if (write(transact_fd, &message, sizeof(message)) <= 0) {
			perror("child transactfd init");
			return 1;
		}

		while (1) {
			*x = childsum(x + 1, COUNT);
			res = read(transact_fd, &message, sizeof(message));
			if (__builtin_expect(res != sizeof(message), 0)) {
				break;
			}
		}
	} else {
		unsigned long long message;
		const int expected = COUNT * (COUNT - 1) / 2 + 1;
		int i, res;

		int transact_fd = open("transact", O_RDWR);
		if (transact_fd == -1) {
			perror("parent: open transact");
			return 1;
		}

		// Synchronize.
		message = 1;
		if (write(transact_fd, &message, sizeof(message)) <= 0) {
			perror("parent transactfd init");
			return 1;
		}

		struct timespec t0, t1;
		clock_gettime(CLOCK_REALTIME, &t0);
		for (i = 0; i < ITERATIONS; i++) {
			*x = 0;
			parentfill(x + 1, COUNT, i);
			const int expected = COUNT * (COUNT - 1) / 2 + 1 + i * COUNT;
			res = read(transact_fd, &message, sizeof(message));
			if (__builtin_expect(*x != expected, 0)) {
				fprintf(stderr, "%s: %d/%d: %d != %d\n", argv[0], i, ITERATIONS, *x, expected);
				ret = 1;
				break;
			}
		}
		clock_gettime(CLOCK_REALTIME, &t1);

		close(transact_fd);
		int status;
		struct rusage usage;
		wait3(&status, 0, &usage);
		printf("%.4f %.4f %.4f\n",
				(usage.ru_utime.tv_sec * 1000000LL + usage.ru_utime.tv_usec) / (1.0 * ITERATIONS),
				(usage.ru_stime.tv_sec * 1000000LL + usage.ru_stime.tv_usec) / (1.0 * ITERATIONS),
				((t1.tv_sec * 1000000000LL + t1.tv_nsec) - (t0.tv_sec * 1000000000LL + t0.tv_nsec)) / (1000.0 * ITERATIONS)
		);
	}
	return ret;
}
