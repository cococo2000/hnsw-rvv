#!/bin/bash

set -ex

get_max_jobs() {
    cpu_cores=$(nproc)
    load=$(awk '{print $1}' /proc/loadavg)
    max_jobs=$(awk -v load="$load" -v cores="$cpu_cores" 'BEGIN {
        if (load == 0) load = 0.01;
        base_jobs = cores;
        max_jobs_calculated = int(base_jobs * cores / load);
        max_jobs_max = cores * 2;
        max_jobs_min = cores / 2;
        if (max_jobs_calculated > max_jobs_max) {
            print max_jobs_max
        } else if (max_jobs_calculated < max_jobs_min) {
            print max_jobs_min
        } else {
            print max_jobs_calculated
        }
    }')
    echo "$max_jobs"
}

cd gem5

dimensions=(8 16 32 64 128 256 512 1024 2048 3072 4096) # 12
# dimensions=(8 16 32 64 128 256 512 1024 2048 4096)
# dimensions=(512 1024)
# dimensions=(8 16 32 64 128 256 2048 3072 4096)
# topks=(1 5 10 50 100)
# topks=(1 5 10 50)
topks=(100)
train=1000
test=100

# Define base directories
BASE_DIR=".."

LOGS_DIR="${BASE_DIR}/hifive-logs"
OUTPUT_DIR="${BASE_DIR}/hifive-output"

for dir in "${LOGS_DIR}" "${OUTPUT_DIR}"; do
    if [ ! -d "${dir}" ]; then
        mkdir -p "${dir}"
    fi
done

for dim in "${dimensions[@]}"; do
    for topk in "${topks[@]}"; do
        DATASET_NAME="random-euclidean-${dim}-${train}-${test}"
        TEST_NAME="${DATASET_NAME}-top${topk}"
        M5OUT_DIR="${OUTPUT_DIR}/m5out-hifive-${TEST_NAME}"
        LOG_FILE="${LOGS_DIR}/hifive-${TEST_NAME}.log"
        DATASET_PATH="${BASE_DIR}/data/${DATASET_NAME}.bin"

        (
            echo "Running hifive test for dimension $dim and topk $topk..."
            { time build/RISCV/gem5.opt --outdir="${M5OUT_DIR}" \
                configs/example/bvb-riscvmatched.py \
                --resource ../hnsw_search_sifive \
                --prog_args="--topk ${topk} --dataset ${DATASET_PATH}" \
                ; } 2>&1 > "${LOG_FILE}"
            echo "hifive Test for dimension $dim and topk $topk completed."
        ) &

        while [ "$(jobs -r | wc -l)" -ge "$(get_max_jobs)" ]; do
            sleep 1
        done
    done
done

wait

echo "All tests completed."

# time build/RISCV/gem5.opt --outdir=../m5out-rv256 configs/example/bvb-board.py --vlen 1024 --resource ../build/bin/hnsw_search_riscv --prog_args="--topk 100 --dataset ${BASE_DIR}/data/random-euclidean-512-1000-100.bin" 
