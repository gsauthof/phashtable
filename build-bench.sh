#!/bin/bash

set -x


# adjust -march flag for other targets

g++ -std=gnu++17 -Wall -DSLOTS_TO_TEST=2776 -O3 -march=goldmont-plus \
    bench.cc phash_table.c \
    -lbenchmark -pthread -o bench

