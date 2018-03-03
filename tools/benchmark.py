import os
import glob
import gc

from time import time

import numpy as np
import pandas as pd
import rcsv

csv_path = glob.glob('*.csv')

for path in csv_path:

    print('--- ' + path)

    start = time()
    rcsv.read(path)
    end = time()

    gc.collect()

    print('rcsv  : %.4f' % (end - start))

    start = time()
    np.loadtxt(path, delimiter=',', dtype=np.float32)
    end = time()

    gc.collect()

    print('numpy : %.4f' % (end - start))

    start = time()
    pd.read_csv(path)
    end = time()

    gc.collect()

    print('panda : %.4f' % (end - start))
