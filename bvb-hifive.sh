#!/bin/bash
runs=3

dataset=./data/ag_news-384-euclidean.bin

base_program_riscv=./build/bin/hnsw_search_riscv

mkdir -p logs-ag_news

Ms=(8 16 32 64 128)
efs=(100 128 200 256 300 400 512)

for ((i=1; i<=$runs; i++)); do
    for M in "${Ms[@]}"; do
        for ef in "${efs[@]}"; do
            echo "Starting run $i"
            LOG_FILE="logs-ag_news/base-M${M}-ef${ef}-run${i}.log"
            $base_program_riscv --topk 100 --dataset $dataset --M $M --ef $ef | tee $LOG_FILE
            # ./build/bin/hnsw_search --topk 100 --dataset $dataset --M $M --ef $ef | tee $LOG_FILE
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

# Create logs directory if it doesn't exist
mkdir -p logs-riscv

# Run the program three times
for ((i=1; i<=$runs; i++)); do
    for dim in "${dimensions[@]}"; do
        for topk in "${topks[@]}"; do
            echo "Starting run $i"
            DATASET_NAME="random-euclidean-${dim}-${train}-${test}"
            TEST_NAME="${DATASET_NAME}-top${topk}"

            LOG_FILE="logs-riscv/base-${TEST_NAME}-run${i}.log"
            $base_program_riscv --topk $topk --dataset data/${DATASET_NAME}.bin | tee $LOG_FILE
            sleep 10
            echo "Completed run $i, log saved to $LOG_FILE"
        done
    done
done
