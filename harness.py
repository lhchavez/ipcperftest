#!/usr/bin/python

import subprocess

counts = [2**i for i in xrange(15)]
programs = ['pipetest', 'transacttest', 'shmpipetest', 'shmtest']
runs = 100
iterations = 100000
total = runs * len(counts) * len(programs)
done = 0

with open('data.csv', 'a') as f:
	for p in programs:
		for c in counts:
			for j in xrange(runs):
				try:
					line = '%s %d %s' % (p, c, subprocess.check_output(['./%s' % p, '--iterations', str(iterations), '--count', str(c)]).strip())
				except subprocess.CalledProcessError as e:
					line = '%s %d error' % (p, c)
				done += 1
				print '%5.2f%% %-40s\r' % (100.0 * done / float(total), line),
				f.write(line + '\n')
print
