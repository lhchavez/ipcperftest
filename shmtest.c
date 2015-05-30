#define _GNU_SOURCE
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/resource.h>

#include "ipcperf.h"

int main(int argc, char* argv[]) {
	int ret = 0;
	parse_args(argc, argv);

	sem_t* sems = (sem_t*)create_shm(sizeof(int) * (COUNT + 1) + sizeof(sem_t) * 2);
	sem_t* write = &sems[0];
	sem_t* read = &sems[1];
	int* x = (int*)&sems[2];

	if (sem_init(write, 1, 0) == -1) {
		perror("sem_init");
		return 1;
	}

	if (sem_init(read, 1, 0) == -1) {
		perror("sem_init");
		return 1;
	}

	int child = fork();
	if (child == -1) {
		perror("fork");
		return 1;
	} else if (child == 0) {
		while (1) {
			sem_wait(write);
			if (__builtin_expect(*x != 0, 0)) {
				break;
			}
			*x = childsum(x + 1, COUNT);
			sem_post(read);
		}
	} else {
		int i;

		struct timespec t0, t1;
		clock_gettime(CLOCK_REALTIME, &t0);
		for (i = 0; i < ITERATIONS; i++) {
			*x = 0;
			parentfill(x + 1, COUNT, i);
			const int expected = COUNT * (COUNT - 1) / 2 + 1 + i * COUNT;
			sem_post(write);
			sem_wait(read);
			if (__builtin_expect(*x != expected, 0)) {
				fprintf(stderr, "%s: %d/%d: %d != %d\n", argv[0], i, ITERATIONS, *x, expected);
				ret = 1;
				break;
			}
		}
		clock_gettime(CLOCK_REALTIME, &t1);

		*x = 1;
		sem_post(write);

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
