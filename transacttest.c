#define _GNU_SOURCE
#include <unistd.h>
#include <time.h>
#include <stdio.h>

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
		int transact_fd = open("transact", O_RDONLY);
		if (transact_fd == -1) {
			perror("child: open transact");
			return 1;
		}

		unsigned long long dead;
		int i = 0, j, sum;

		while (1) {
			*x = childsum(x + 1, COUNT);
			read(transact_fd, &dead, sizeof(dead));
			if (__builtin_expect(dead, 0)) {
				break;
			}
		}
	} else {
		unsigned long long dead;
		const int expected = COUNT * (COUNT - 1) / 2 + 1;
		int i;
		parentfill(x + 1, COUNT);

		int transact_fd = open("transact", O_RDWR);
		if (transact_fd == -1) {
			perror("parent: open transact");
			return 1;
		}

		struct timespec t0, t1;
		clock_gettime(CLOCK_REALTIME, &t0);
		for (i = 0; i < ITERATIONS; i++) {
			*x = 0;
			read(transact_fd, &dead, sizeof(dead));
			if (__builtin_expect(dead, 0)) {
				fprintf(stderr, "child died\n");
				break;
			}
			if (__builtin_expect(*x != expected, 0)) {
				fprintf(stderr, "%s: %d/%d: %d != %d\n", argv[0], i, ITERATIONS, *x, expected);
				ret = 1;
				break;
			}
		}
		clock_gettime(CLOCK_REALTIME, &t1);

		printf("%.4f\n", ((t1.tv_sec * 1000000000LL + t1.tv_nsec) - (t0.tv_sec * 1000000000LL + t0.tv_nsec)) / (1000.0 * ITERATIONS));

		close(transact_fd);
		int status;
		wait(&status);
	}
	return ret;
}
