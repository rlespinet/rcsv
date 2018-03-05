import os
import glob
import gc

from time import time

import numpy as np
import pandas as pd
import rcsv

readers = {
    'rcsv':  lambda path: rcsv.read(path),
    'numpy': lambda path: np.loadtxt(path, delimiter=',', dtype=np.float32),
    'panda': lambda path: pd.read_csv(path),
}

csv_path = glob.glob('*.csv')

for path in csv_path:

    print('--- ' + path)

    for name, reader in readers.items():
        start = time()
        reader(path)
        end = time()

        print('%s  : %.4f' % (name, end - start))

    gc.collect()
