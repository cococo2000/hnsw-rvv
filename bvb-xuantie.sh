#!/bin/bash

dimensions=(8 16 32 64 128 256 512 1024 2048 3072 4096) # 12
# topks=(1 5 10 50 100)
# topks=(1 5 10 50)
topks=(100)
train=1000
test=100

runs=3

cpu=c910
# cpu=c908v
# cpu=c906fdv
# cpu=c907fdvm
# cpu=c920v2
# cpu=c920v3
# cpu=r908fdv

rv_program=./build/bin/hnsw_search_${cpu}_rv
rvv_program=./build/bin/hnsw_search_${cpu}_rvv

# Create logs directory if it doesn't exist
mkdir -p logs-${cpu}

# Run the program three times
for ((i=1; i<=$runs; i++)); do
    for dim in "${dimensions[@]}"; do
        for topk in "${topks[@]}"; do
            echo "Starting run $i"
            DATASET_NAME="random-euclidean-${dim}-${train}-${test}"
            TEST_NAME="${DATASET_NAME}-top${topk}"
            LOG_FILE="logs-${cpu}/rv-${TEST_NAME}-run${i}.log"
            $rv_program --topk $topk --dataset data/${DATASET_NAME}.bin | tee $LOG_FILE
            LOG_FILE="logs-${cpu}/rvv-${TEST_NAME}-run${i}.log"
            $rvv_program --topk $topk --dataset data/${DATASET_NAME}.bin | tee $LOG_FILE
            echo "Completed run $i, log saved to $LOG_FILE"
        done
    done
done

dataset=./data/ag_news-384-euclidean.bin

mkdir -p logs-ag_news

Ms=(8 16 32 64 128)
efs=(100 128 200 256 300 400 512)

for ((i=1; i<=$runs; i++)); do
    for M in "${Ms[@]}"; do
        for ef in "${efs[@]}"; do
            echo "Starting run $i"
            LOG_FILE="logs-ag_news/rvv-M${M}-ef${ef}-run${i}.log"
            $rvv_program --topk 100 --dataset $dataset --M $M --ef $ef | tee $LOG_FILE
            sleep 3
            LOG_FILE="logs-ag_news/rv-M${M}-ef${ef}-run${i}.log"
            $rv_program --topk 100 --dataset $dataset --M $M --ef $ef | tee $LOG_FILE
            sleep 3
            echo "ag_news Completed run $i, log saved to $LOG_FILE"
        done
    done
done


# tar -czvf test-xuantie.tar.gz data build bvb-xuantie.sh
