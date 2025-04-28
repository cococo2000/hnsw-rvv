#!/bin/bash


# Add `-g -fno-omit-frame-pointer` in build.sh/Makefile

dataset=./data/ag_news-384-euclidean.bin

perf record -F 99 -g -- ./build/bin/hnsw_search --topk 100 --dataset $dataset
perf script > out.perf

cd Flamegraph
./stackcollapse-perf.pl ../out.perf > ../out.folded
./flamegraph.pl ../out.folded > ../hnsw_search.svg
