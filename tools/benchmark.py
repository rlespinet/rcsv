import os
import multiprocessing
import psutil
import argparse

from time import time, sleep

import numpy as np
import pandas as pd
import rcsv

readers_map = {
    'numpy': lambda path: np.loadtxt(path, delimiter=',', dtype=np.float32),
    'rcsv':  lambda path: rcsv.read(path),
    'panda': lambda path: pd.read_csv(path),
}

parser = argparse.ArgumentParser(description='Simple Benchmarking tool',
                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)

helps = {
    'tool':"""
Change the benchmarking tool :
   memory : Profile all memory usage
   timer  : Only measure time
""",
    'numpy':"""
Profile numpy
""",
    'panda':"""
Profile panda
""",
    'rcsv':"""
Profile rcsv
""",
    'files':"""
Files to be benchmarked
""",
}

parser.add_argument('--numpy', help=helps['numpy'], action='store_true')
parser.add_argument('--rcsv',  help=helps['rcsv'],  action='store_true')
parser.add_argument('--panda', help=helps['panda'], action='store_true')
parser.add_argument('--tool',  help=helps['tool'],  type=str, choices=['memory', 'timer'], default='timer')
parser.add_argument('files',   help=helps['files'], type=str, nargs='+')

args = parser.parse_args()

readers = []
if args.numpy: readers.append(('numpy', readers_map['numpy']))
if args.rcsv:  readers.append(('rcsv', readers_map['rcsv']))
if args.panda: readers.append(('panda', readers_map['panda']))

if not readers: readers = list(readers_map.items())


def profile_memory_usage(process):

    start = time()
    process.start()

    pshdl = psutil.Process(process.pid)

    min_sleep = 0.01
    max_sleep = 0.25

    drss_precision_aim = 1.0/100.0
    backoff_margin = 25.0/100.0

    backoff_coeff = 1.5

    backoff_margin_inf = drss_precision_aim * (1.0 - backoff_margin)
    backoff_margin_sup = drss_precision_aim * (1.0 + backoff_margin)

    cur_sleep = min_sleep
    last_rss = 0

    while (process.is_alive()):

        infos = pshdl.memory_full_info()
        print(time() - start, end=',')
        print(infos.rss, infos.vms, infos.shared, sep=',', end=',')
        print(infos.text, infos.data, infos.dirty, sep=',', end=',')
        print(infos.uss, infos.pss, infos.swap, sep=',')

        rss = infos.rss

        if abs(rss - last_rss) < backoff_margin_inf * last_rss:
            cur_sleep *= backoff_coeff
        elif abs(rss - last_rss) > backoff_margin_sup * last_rss:
            cur_sleep /= backoff_coeff

        cur_sleep = min(cur_sleep, max_sleep)
        cur_sleep = max(cur_sleep, min_sleep)

        sleep(cur_sleep)

    # Just for safety, probably not needed
    process.join()

def time_process(process):
    start = time()
    process.start()
    process.join()
    end = time()
    print('%s  : %.4f' % (name, end - start))


for path in args.files:

    print('#', path)

    for name, reader in readers:

        print('## ', name)
        process = multiprocessing.Process(target=reader, args=(path,))

        if args.tool == 'memory':
            profile_memory_usage(process)
        elif args.tool == 'timer':
            time_process(process)
