import random
import numpy as np
import argparse
from tqdm import tqdm

from progress import ProgressBar

parser = argparse.ArgumentParser(description='Simple CSV Generator',
                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)

parser.add_argument('--float-range',
                    help='Range of the generated floats',
                    type=float, nargs=2, metavar=('min', 'max'), default=(-1e10, 1e10))
parser.add_argument('--digits-range',
                    help='Range for the number of significant digits for each field',
                    type=float, nargs=2, metavar=('min', 'max'), default=(0, 10))
parser.add_argument('--prefix-ws-range',
                    help='Range for the number of trailing whitespace before' +
                    ' each comma and at the end of each line',
                    type=float, nargs=2, metavar=('min', 'max'), default=(0, 3))
parser.add_argument('--suffix-ws-range',
                    help='Range for the number of trailing whitespace after' +
                    ' each comma and at the end of each line',
                    type=float, nargs=2, metavar=('min', 'max'), default=(0, 3))
parser.add_argument('--formatting',
                    help='Printf style format letters that can be used to generate output',
                    type=str, metavar='fmt', default='ef')
parser.add_argument('--output', '-o',
                    help='Output file to save to',
                    type=str, metavar='filename', default='out.csv')
parser.add_argument('rows',
                    help='Number of rows of the generated CSV',
                    type=int)
parser.add_argument('cols',
                    help='Number of columns of the generated CSV',
                    type=int)

args = parser.parse_args()

rows = args.rows
cols = args.cols

min_float, max_float = args.float_range
min_digits, max_digits = args.digits_range
min_prefix_ws, max_prefix_ws = args.prefix_ws_range
min_suffix_ws, max_suffix_ws = args.suffix_ws_range

output = args.output

with open(output, 'w') as f:

    progress = ProgressBar(rows)
    for i in range(rows):

        x = np.random.uniform(min_float, max_float, cols)
        d = np.random.randint(min_digits, max_digits+1, cols)
        t = np.random.choice(list(args.formatting), cols)

        s = [' ' * i for i in np.random.randint(min_prefix_ws, max_prefix_ws+1, cols)]
        e = [' ' * i for i in np.random.randint(min_suffix_ws, max_suffix_ws+1, cols)]

        fmts = ['{xi:.{di}{ti}}'.format(xi=xi, di=di, ti=ti) for xi, di, ti in zip(x, d, t)]

        f.write(','.join([si + fmt + ei for si, ei, fmt in zip(s, e, fmts)]))
        f.write('\n')

        progress.increment()
