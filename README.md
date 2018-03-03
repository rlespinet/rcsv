# Introduction

There's a point in your life where you might want to read large CSV
files in Python. At this point you might realize that even though your
file is simple and consists only of comma separated floating point
values, there is no well known tool that loads it very efficiently.
This repository is an attempt to load simple CSV files in a numpy
array at disk speed, meaning that you could not get any faster than
that.

At this point you might wonder "Ok, what are the failure cases ?", and
since I ask you this question you might think that there is none, but
actually that's not entirely true, here are some characteristics

 * The file has to be well formated (the code does not perform any
   checks when loading atm)

 * The file is loaded in an array of float, every field that cannot be
   read as a float (by `strtof` basically), will end up having a value
   of `NaN` (as opposed to loadtxt which fails in this case)

# Version

This code works for Python 3 and doesn't use any library. It uses
POSIX Threads, it has been tested on linux but it should work on
Mac. I'll probably extend it to Windows when I have the time.

# Installing

    git clone https://github.com/rlespinet/rcsv.git --recurse
    cd rcsv
    python setup.py install

# Benchmarks

To benchmark the program, I'm using a random CSV generator that is
located in directory tools/. This is a simple python program that
generates a matrix of random, and write each coefficient in a file,
each coefficient having a random format (random number of digit with
standard or scientific notation also chosen randomly).

| File size | Rows x Cols | numpy    | panda   | rcsv   |
|:---------:|:-----------:|:--------:|:-------:|:------:|
| 52Ko      | 50x50       | 0.0032   | 0.0027  | 0.0025 |
| 484Ko     | 500x50      | 0.0247   | 0.0072  | 0.0042 |
| 4.848Mo   | 500x500     | 0.1955   | 0.0602  | 0.0153 |
| 48.46Mo   | 5000x500    | 1.9180   | 0.5346  | 0.0933 |
| 96.86Mo   | 50000x100   | 4.1536   | 1.1553  | 0.1704 |
| 484.7Mo   | 50000x500   | 19.4205  | 5.3127  | 0.8233 |
| 969.3Mo   | 10000x5000  | 37.4691  | 12.8547 | 2.0082 |
| 1.743Go   | 20000x5000  | 74.7051  | 25.8144 | 3.1170 |
| 3.487Go   | 20000x10000 | 151.0585 | 65.1991 | 7.9086 |

I use it to generate files with a size in the range of 500Ko to
3GB. The results are provided in the table above and the graphs below.

![alt text](https://github.com/rlespinet/rcsv/blob/master/docs/imgs/benchmark1.png/ "Benchmark from 500Ko to 100Mo")

![alt text](https://github.com/rlespinet/rcsv/blob/master/docs/imgs/benchmark2.png/ "Benchmark from 500Mo to 3.5Go")


Please send me an email at remi@lespi.net if it fails to load your CSV
and you think it shouldn't (especially if a simple np.loadtxt can load
it) or if loading your CSV is slower with this program than with
another one.
