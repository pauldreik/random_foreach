# random_foreach
code for randomly generating all numbers in a range exactly once. Intended to be used for exhaustive testing, where you want to touch interesting input early.

This code was developed and eventually lead to a
talk on Stockholm Cpp 20200123. Slides are available [here](https://www.pauldreik.se/talks/20200123_crypto/)

The idea is to generate all numbers in a range in a random order exactly once, by using a block cipher.

The block cipher uses a Feistel ceipher. A CRTP base class is used, so one can focus less on Feistel and more on the rounding function which is what makes the difference between the ciphers.

## Compiling
Runs on gcc/clang but may be adapted to other compilers (it uses intrinsics).
You will also need the testu01 library (on Debian/Ubuntu, apt install libtestu01-0-dev should be sufficient.).

## Benchmarking
You need perf to run the benchmark program.

## Author and license
Paul Dreik 2019/2020.

Boost software license 1.0
