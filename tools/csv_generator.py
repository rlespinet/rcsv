import random
import numpy as np

rows = 50000
cols = 500

min_float = -1e10
max_float = 1e10

min_digits = 0
max_digits = 10

with open('out.csv', 'w') as f:
    for i in range(rows):
        x = np.random.uniform(min_float, max_float, cols)
        d = np.random.randint(min_digits, max_digits+1, cols)
        t = np.where(np.random.randint(0, 2, cols), 'e', 'f')
        fmts = ['{xi:.{di}{ti}}'.format(xi=xi, di=di, ti=ti) for xi, di, ti in zip(x, d, t)]

        f.write(' , '.join(fmts))
        f.write('\n')
