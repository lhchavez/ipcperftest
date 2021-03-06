.PHONY: all
all: pipetest shmtest shmpipetest transacttest

pipetest: pipetest.c | ipcperf.h Makefile
	gcc -O2 -o $@ $^ -Wno-unused-result

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
	rm pipetest shmtest shmpipetest transacttest

.PHONY: test
test: | pipetest shmtest shmpipetest transacttest
	rm data.csv 2> /dev/null; python harness.py --iterations 1000 && python extract.py data.csv
