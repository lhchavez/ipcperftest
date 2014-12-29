.PHONY: all
all: pipetest shmtest shmpipetest transacttest

pipetest: pipetest.c | ipcperf.h Makefile
	gcc -O2 -g -o $@ $^ -Wno-unused-result

shmtest: shmtest.c | ipcperf.h Makefile
	gcc -O2 -o $@ $^ -lrt -lpthread -Wno-unused-result

shmpipetest: shmpipetest.c | ipcperf.h Makefile
	gcc -O2 -o $@ $^ -lrt -Wno-unused-result

transacttest: transacttest.c | ipcperf.h transact Makefile
	gcc -O2 -o $@ $^ -lrt -Wno-unused-result

transact:
	sudo mknod $@ c 2038 0 && sudo chmod 666 $@

.PHONY: clean
clean:
	rm pipetest shmtest transacttest
