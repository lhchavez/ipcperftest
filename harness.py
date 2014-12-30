#!/usr/bin/python

import argparse
import subprocess

parser = argparse.ArgumentParser(description='Run the ipc performance test suite')
parser.add_argument('--runs', type=int, default=100,
        help='Number of times to run each test program')
parser.add_argument('--iterations', type=int, default=10000,
        help='Number of iterations per test run')
args = parser.parse_args()

counts = [2**i for i in xrange(15)]
programs = {
	'pipetest': 'pipes',
	'transacttest': 'shm+transact',
	'shmpipetest': 'shm+pipes',
	'shmtest': 'shm+semaphore'
}
total = args.runs * len(counts) * len(programs)
done = 0

with open('data.csv', 'a') as f:
	for p in programs:
		for c in counts:
			for j in xrange(args.runs):
				try:
					line = '%s %d %s' % (programs[p], c, subprocess.check_output(['./%s' % p, '--iterations', str(args.iterations), '--count', str(c)]).strip())
				except subprocess.CalledProcessError as e:
					line = '%s %d error' % (p, c)
				done += 1
				print '%5.2f%% %-40s\r' % (100.0 * done / float(total), line),
				f.write(line + '\n')
print
