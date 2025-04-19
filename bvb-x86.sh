#!/bin/bash
runs=3

dataset=./data/ag_news-384-euclidean.bin

mkdir -p logs-ag_news

Ms=(8 16 32 64 128)
efs=(100 128 200 256 300 400 512)

for ((i=1; i<=$runs; i++)); do
    for M in "${Ms[@]}"; do
        for ef in "${efs[@]}"; do
            echo "Starting run $i"
            LOG_FILE="logs-ag_news/base-M${M}-ef${ef}-run${i}.log"
            ./build/bin/hnsw_search --topk 100 --dataset $dataset --M $M --ef $ef | tee $LOG_FILE
            sleep 10
            LOG_FILE="logs-ag_news/sse-M${M}-ef${ef}-run${i}.log"
            ./build/bin/hnsw_search_sse --topk 100 --dataset $dataset --M $M --ef $ef | tee $LOG_FILE
            sleep 10
            LOG_FILE="logs-ag_news/avx-M${M}-ef${ef}-run${i}.log"
            ./build/bin/hnsw_search_avx --topk 100 --dataset $dataset --M $M --ef $ef | tee $LOG_FILE
            sleep 10
            LOG_FILE="logs-ag_news/avx512-M${M}-ef${ef}-run${i}.log"
            ./build/bin/hnsw_search_avx512 --topk 100 --dataset $dataset --M $M --ef $ef | tee $LOG_FILE
            sleep 10
            LOG_FILE="logs-ag_news/sse42-M${M}-ef${ef}-run${i}.log"
            ./build/bin/hnsw_search_sse42 --topk 100 --dataset $dataset --M $M --ef $ef | tee $LOG_FILE
            sleep 10
            echo "ag_news Completed run $i, log saved to $LOG_FILE"
        done
    done
done



dimensions=(8 16 32 64 128 256 512 1024 2048 3072 4096) # 12
# topks=(1 5 10 50 100)
# topks=(1 5 10 50)
topks=(100)
train=1000
test=100


base_program=./build/bin/hnsw_search
sse_program=./build/bin/hnsw_search_sse
sse42_program=./build/bin/hnsw_search_sse42
avx_program=./build/bin/hnsw_search_avx
avx512_program=./build/bin/hnsw_search_avx512

# Create logs directory if it doesn't exist
mkdir -p logs-x86-base
mkdir -p logs-x86-sse42
mkdir -p logs-x86-sse
mkdir -p logs-x86-avx
mkdir -p logs-x86-avx512

# Run the program three times
for ((i=1; i<=$runs; i++)); do
    for dim in "${dimensions[@]}"; do
        for topk in "${topks[@]}"; do
            echo "Starting run $i"
            DATASET_NAME="random-euclidean-${dim}-${train}-${test}"
            TEST_NAME="${DATASET_NAME}-top${topk}"

            LOG_FILE="logs-x86-base/base-${TEST_NAME}-run${i}.log"
            $base_program --topk $topk --dataset data/${DATASET_NAME}.bin | tee $LOG_FILE
            sleep 10
            LOG_FILE="logs-x86-sse/sse-${TEST_NAME}-run${i}.log"
            $sse_program --topk $topk --dataset data/${DATASET_NAME}.bin | tee $LOG_FILE
            sleep 10
            LOG_FILE="logs-x86-avx/avx-${TEST_NAME}-run${i}.log"
            $avx_program --topk $topk --dataset data/${DATASET_NAME}.bin | tee $LOG_FILE
            sleep 10
            LOG_FILE="logs-x86-avx512/avx512-${TEST_NAME}-run${i}.log"
            $avx512_program --topk $topk --dataset data/${DATASET_NAME}.bin | tee $LOG_FILE
            sleep 10
            LOG_FILE="logs-x86-sse42/sse42-${TEST_NAME}-run${i}.log"
            $sse42_program --topk $topk --dataset data/${DATASET_NAME}.bin | tee $LOG_FILE
            sleep 10

            echo "Completed run $i, log saved to $LOG_FILE"
        done
    done
done
