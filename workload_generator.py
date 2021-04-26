import argparse
import random

DUMMY_VAL = '0' * 1000


def zeta(n, theta):
    z = 0.0
    for i in range(n):
        z += 1. / ((i + 1) ** theta)
    return z

class ZipfianGenerator(object):
    def __init__(self, n, theta):
        self.n = n
        self.theta = theta
        zeta_2 = zeta(2, theta)
        self.alpha = 1. / (1. - theta)
        self.zeta_n = zeta(n, theta)
        self.eta = (1 - ((2. / n) ** (1. - theta))) / (1 - zeta_2 / self.zeta_n)
        self.previous = 0
        self.next_value()

    def next_value(self):
        u = random.uniform(0., 1.)
        uz = u * self.zeta_n

        if uz < 1.0:
            return 0

        if uz < 1.0 + 0.5 ** self.theta:
            return 1

        result = (self.previous + 1 +  int(self.n * ((self.eta * u - self.eta + 1) ** self.alpha))) % self.n
        self.previous = result
        return result

def get_correlated_key(key, n, generator):
    random_key = generator.next_value()
    return random_key + key % n


def generate_workload(**kwargs):
    output_file = kwargs.get('out')
    theta = kwargs.get('theta')
    write_rate = kwargs.get('write_rate')
    num_records = kwargs.get('num_records')
    transcript_size = kwargs.get('transcript_size')
    print_histogram = kwargs.get('print_histogram', False)

    print('Creating Zipfian generator...')
    generator = ZipfianGenerator(num_records, theta)

    print('Generating workload file...')
    hist = [0 for _ in range(min(100, num_records))]
    with open(output_file, 'w') as out:
        num_generated = 0
        while num_generated < transcript_size:
            key = generator.next_value()
            if key < len(hist):
                hist[key] += 1
            write = random.uniform(0., 1.) <= write_rate
            if write:
                out.write('PUT {} {}\n'.format(key, DUMMY_VAL))
            else:
                out.write('GET {}\n'.format(key))
            num_generated += 1
            second_key = get_correlated_key(key, num_records, generator) 
            if second_key < len(hist):
                hist[second_key] += 1
            write = random.uniform(0., 1.) <= write_rate
            if write:
                out.write('PUT {} {}\n'.format(second_key, DUMMY_VAL))
            else:
                out.write('GET {}\n'.format(second_key))
            num_generated += 1

    if print_histogram:
        print('Histogram of top 100 keys: {}'.format(hist))
    print('All done! :-)')


def main():
    parser = argparse.ArgumentParser(description='Generates workload file.')
    parser.add_argument('-o', '--out', type=str, metavar='OUTPUT_FILE', required=True, help='Output workload file.')
    parser.add_argument('-t', '--theta', default=0.99, type=float,
                        help='Theta for Zipfian distribution (between 0.0 and 1.0).')
    parser.add_argument('-w', '--write-rate', default=0.0, type=float, help='Write proportion (between 0.0 and 1.0).')
    parser.add_argument('-n', '--num-records', default=1000000, type=int, help='Number of records')
    parser.add_argument('-s', '--transcript-size', default=10000000, type=int, help='Size of workload transcript')
    parser.add_argument('--print-histogram', action='store_true', help='Print histogram of top 100 keys.')
    args = parser.parse_args()
    generate_workload(**(vars(args)))


if __name__ == '__main__':
    main()