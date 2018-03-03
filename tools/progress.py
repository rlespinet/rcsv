import sys
import time

class ProgressBar:

    def __init__(self, steps):
        self.cur_step = 0;
        self.max_step = steps;


    def restart(self, steps=None):
        if steps is not None:
            self.max_step = steps
        self.cur_step = 0;

    def increment(self):
        self.cur_step += 1
        if self.cur_step >= self.max_step:
            self.cur_step = self.max_step

        p = float(self.cur_step) / float(self.max_step)
        n = int(40 * p)
        sys.stdout.write('[')
        sys.stdout.write('#' * n)
        sys.stdout.write('-' * (40 - n))
        sys.stdout.write(']')
        sys.stdout.write('[%3d%%]\r' % int(100 * p))
