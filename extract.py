import sys
import math

def analyze(data, idx):
    series = [val[idx] for val in data]
    avg = sum(series) / len(series)
    stddev = math.sqrt(sum([(avg - x)**2 for x in series]) / len(series))
    return avg, stddev

data = {}
names = {}
counts = {}
with open(sys.argv[1], 'r') as f:
    for line in f:
        name, c, user, sys, wall = line.strip().split()
        c = int(c)
        x = map(float, (user, sys, wall))
        if name not in names:
            names[name] = True
        if c not in counts:
            counts[c] = True
        if (name, c) not in data:
            data[(name, c)] = []
        data[(name, c)].append(x)

names = sorted(names.keys())
counts = sorted(counts.keys())

def intervals(mu, sigma):
    return mu, mu - 1.96 * sigma, mu + 1.96 * sigma

for idx, name in enumerate(['user', 'sys', 'wall']):
    print name
    print '\t' + ''.join(['%14s' % x[0:7] for x in names])
    for c in counts:
        print '%d\t%s' % (c, ''.join(['%8.2f(%4.2f)' % analyze(data[(name, c)], idx) for name in names]))
