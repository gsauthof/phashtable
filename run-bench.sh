#!/bin/bash

set -x

for i in $(seq 3); do
    taskset -c 5 ./bench  --benchmark_out_format=csv --benchmark_out=bench-$i.csv
done


