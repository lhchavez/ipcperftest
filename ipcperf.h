#include <fcntl.h>
#include <sched.h>
#include <semaphore.h>
#include <sys/mman.h>

int ITERATIONS = 1000000;
int COUNT = 1;

void parse_args(int argc, char* argv[]) {
	int i;
	for (i = 0; i < argc; ++i) {
		if (!strcmp(argv[i], "--affinity")) {
			cpu_set_t mask;
			CPU_ZERO(&mask);
			CPU_SET(0, &mask);
			sched_setaffinity(0, sizeof(mask), &mask);
		} else if (!strcmp(argv[i], "--iterations") && i + 1 < argc) {
			ITERATIONS = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "--count") && i + 1 < argc) {
			COUNT = atoi(argv[++i]);
		}
	}
}

#if !defined(NO_SHM)
void* create_shm(size_t size) {
	// Align to 4k
	size += (~(size - 1) & 0xFFF);

	int oflags = O_RDWR | O_CREAT | O_TRUNC;
	const char* filename = "/ipcperf";
	int fd = shm_open(filename, oflags, S_IRUSR | S_IWUSR);

	if (fd == -1) {
		perror("shm_open");
		return NULL;
	}

	if (shm_unlink(filename) == -1) {
		perror("shm_unlink");
		return NULL;
	}

	int fd_flags = fcntl(fd, F_GETFD);
	if (fd_flags == -1) {
		perror("F_GETFD");
		return NULL;
	}
	fd_flags &= ~FD_CLOEXEC;
	if (fcntl(fd, F_SETFD, fd_flags) == -1) {
		perror("F_SETFD");
		return NULL;
	}

	if (ftruncate(fd, size) == -1) {
		perror("ftruncate");
		return NULL;
	}

	void* shm = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (shm == (void*)-1) {
		perror("mmap");
		return NULL;
	}

	return shm;
}
#endif

inline int childsum(int* arr, int count) {
	int sum = 1, i;
	for (i = 0; i < count; i++) {
		sum += *(arr++);
	}
	return sum;
}

inline void parentfill(int* arr, int count) {
	int i;
	for (i = 0; i < count; i++) {
		*(arr++) = i;
	}
}
