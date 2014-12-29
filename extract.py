import sys
import math

def analyze(series):
    avg = sum(series) / len(series)
    stddev = math.sqrt(sum([(avg - x)**2 for x in series]) / len(series))
    return avg, stddev

data = {}
names = {}
counts = {}
with open(sys.argv[1], 'r') as f:
    for line in f:
        name, c, x = line.strip().split()
        c = int(c)
        x = float(x)
        if name not in names:
            names[name] = True
        if c not in counts:
            counts[c] = True
        if (name, c) not in data:
            data[(name, c)] = []
        data[(name, c)].append(x)

names = sorted(names.keys())
counts = sorted(counts.keys())

print ',' + ','.join(names)
for c in counts:
    print '%d,%s' % (c, ','.join([str(analyze(data[(name, c)])[0]) for name in names]))
