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

	int out[2];
	int in[2];

	pipe(out);
	pipe(in);

	int child = fork();
	if (child == -1) {
		perror("fork");
		return 1;
	} else if (child == 0) {
		close(out[1]);
		close(in[0]);
		int msg = 0;

		// Synchronize.
		write(in[1], &msg, sizeof(msg));

		while (read(out[0], &msg, sizeof(msg)) == sizeof(msg)) {
			*x = childsum(x + 1, COUNT);
			write(in[1], &msg, sizeof(msg));
		}
	} else {
		int i, msg = 0;

		// Synchronize.
		read(in[0], &msg, sizeof(msg));

		struct timespec t0, t1;
		clock_gettime(CLOCK_REALTIME, &t0);
		for (i = 0; i < ITERATIONS; i++) {
			*x = 0;
			parentfill(x + 1, COUNT, i);
			const int expected = COUNT * (COUNT - 1) / 2 + 1 + i * COUNT;
			write(out[1], &msg, sizeof(msg));
			read(in[0], &msg, sizeof(msg));
			if (__builtin_expect(*x != expected, 0)) {
				fprintf(stderr, "%s: %d/%d: %d != %d\n", argv[0], i, ITERATIONS, *x, expected);
				ret = 1;
				break;
			}
		}
		clock_gettime(CLOCK_REALTIME, &t1);
		close(out[1]);
		close(in[0]);

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
