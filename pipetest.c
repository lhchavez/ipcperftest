#define _GNU_SOURCE
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

#define NO_SHM
#include "ipcperf.h"

ssize_t writefull(int fd, const void* buf, size_t size) {
	ssize_t bytes, ret = 0;
	while (size) {
		bytes = write(fd, buf, size);
		if (bytes <= 0) {
			break;
		}
		ret += bytes;
		size -= bytes;
		buf = (char*)buf + bytes;
	}
	return ret;
}

ssize_t readfull(int fd, void* buf, size_t size) {
	ssize_t bytes, ret = 0;
	while (size) {
		bytes = read(fd, buf, size);
		if (bytes <= 0) {
			break;
		}
		ret += bytes;
		size -= bytes;
		buf = (char*)buf + bytes;
	}
	return ret;
}

int main(int argc, char* argv[]) {
	int ret = 0;
	parse_args(argc, argv);

	int out[2];
	int in[2];

	pipe(out);
	pipe(in);

	int child = fork();
	if (child == -1) {
		perror("fork");
		ret = 1;
	} else if (child == 0) {
		close(out[1]);
		close(in[0]);
		int* buf = (int*)malloc(COUNT * sizeof(int));
		int msg = 0;

		// Synchronize.
		write(in[1], &msg, sizeof(msg));

		while (readfull(out[0], buf, COUNT * sizeof(int)) == COUNT * sizeof(int)) {
			msg = childsum(buf, COUNT);
			write(in[1], &msg, sizeof(msg));
		}
	} else {
		close(out[0]);
		close(in[1]);
		int* buf = (int*)malloc(COUNT * sizeof(int));
		int i, msg;

		// Synchronize.
		read(in[0], &msg, sizeof(msg));

		struct timespec t0, t1;
		clock_gettime(CLOCK_REALTIME, &t0);
		for (i = 0; i < ITERATIONS; i++) {
			parentfill(buf, COUNT, i);
			const int expected = COUNT * (COUNT - 1) / 2 + 1 + i * COUNT;
			writefull(out[1], buf, sizeof(int) * COUNT);
			read(in[0], &msg, sizeof(msg));
			if (__builtin_expect(msg != expected, 0)) {
				fprintf(stderr, "%s: %d/%d: %d != %d\n", argv[0], i, ITERATIONS, msg, expected);
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
