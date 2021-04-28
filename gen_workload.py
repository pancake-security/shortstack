import bisect
import random
import math
from functools import reduce
import sys

import numpy


class ZipfGenerator(object):
    def __init__(self, alpha, max_val, n):
        tmp = [1. / (math.pow(float(i), alpha)) for i in range(1, max_val + 1)]
        zeta = reduce(lambda sums, x: sums + [sums[-1] + x], tmp, [0])
        self.zeta_dist = [x / zeta[-1] for x in zeta]
        self.n = n
        self.i = 0

    def __iter__(self):
        return self

    def __next__(self):
        return self.next()

    def next(self):
        if self.i == self.n:
            raise StopIteration
        self.i += 1
        return bisect.bisect(self.zeta_dist, random.random()) - 1


def gen_zipf(alpha, max_val, n):
    tmp = numpy.power(numpy.arange(1, max_val + 1), -alpha)
    zeta = numpy.r_[0.0, numpy.cumsum(tmp)]
    zeta_dist = [x / zeta[-1] for x in zeta]
    v = numpy.searchsorted(zeta_dist, numpy.random.random(n))
    samples = [t - 1 for t in v]
    return samples


def flip(p):
    return 'GET' if random.random() < p else 'PUT'

outfile = sys.argv[1]
num_keys = int(sys.argv[2])
trace_len = int(sys.argv[3])
theta = float(sys.argv[4])
read_ratio = float(sys.argv[5])
obj_size = int(sys.argv[6])

dummy_val = '0'*obj_size

# alpha = 1.0/(1.0 - theta)
alpha = theta

print(alpha)

trace = gen_zipf(alpha, num_keys, trace_len)

f = open(outfile, 'w')

for k in trace:
    op = flip(read_ratio)
    f.write(op)
    f.write(' ' + str(k))
    if(op == 'PUT'):
        f.write(' ' + dummy_val)
    f.write('\n')

f.close()

print('Trace written')