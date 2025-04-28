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

# Define base directories and paths
BASE_DIR=".."
GEM5_BIN="build/RISCV/gem5.opt"
GEM5_CONFIG="configs/example/bvb-board.py"
RESOURCE_BASE_DIR="${BASE_DIR}/build/bin"
RESOURCE_RV64GC_RV="${RESOURCE_BASE_DIR}/hnsw_search_rv64gc_rv"
RESOURCE_RV64GCV_RV="${RESOURCE_BASE_DIR}/hnsw_search_rv64gcv_rv"
RESOURCE_RV64GCV_RVV="${RESOURCE_BASE_DIR}/hnsw_search_rv64gcv_rvv"
DATA_BASE_DIR="${BASE_DIR}/data"

# dimensions=(8 16 32 64 128 256 512 1024 2048 3072 4096) # 12
# dimensions=(8 16 32 64 128 256 512 1024 2048 4096)
dimensions=(2048 3072 4096)
# dimensions=(8 16 32 64 128 256 2048 3072 4096)
# topks=(1 5 10 50 100)
# topks=(1 5 10 50)
topks=(100)
train=1000
test=100

# vlens=(128 256 512 1024)
vlens=(512)
# vlens=(128 256)
elen=64
l1d_size="32KiB"

for vlen in "${vlens[@]}"; do
    # Create directories if they don't exist
    LOGS_DIR="${BASE_DIR}/rvv-logs-vlen${vlen}-elen${elen}"
    OUTPUT_DIR="${BASE_DIR}/rvv-output-vlen${vlen}-elen${elen}"

    for dir in "${LOGS_DIR}" "${OUTPUT_DIR}"; do
        if [ ! -d "${dir}" ]; then
            mkdir -p "${dir}"
        fi
    done

    for dim in "${dimensions[@]}"; do
        for topk in "${topks[@]}"; do
            DATASET_NAME="random-euclidean-${dim}-${train}-${test}"
            TEST_NAME="${DATASET_NAME}-top${topk}"
            M5OUT_DIR_RVV="${OUTPUT_DIR}/m5out-rvv-${TEST_NAME}"
            M5OUT_DIR_RV="${OUTPUT_DIR}/m5out-rv-${TEST_NAME}"
            LOG_FILE_RVV="${LOGS_DIR}/rvv-${TEST_NAME}.log"
            LOG_FILE_RV="${LOGS_DIR}/rv-${TEST_NAME}.log"
            DATASET_PATH="${DATA_BASE_DIR}/${DATASET_NAME}.bin"
            PROG_ARGS="--topk ${topk} --dataset ${DATASET_PATH}"

            # RV64GCV_RVV
            # (
            #     echo "Running rvv test for dimension $dim and topk $topk..."
            #     { time "${GEM5_BIN}" --outdir="${M5OUT_DIR_RVV}" \
            #         "${GEM5_CONFIG}" \
            #         --vlen $vlen \
            #         --elen $elen \
            #         --l1d_size $l1d_size \
            #         --resource "${RESOURCE_RV64GCV_RVV}" \
            #         --prog_args="${PROG_ARGS}" \
            #         ; } 2>&1 > "${LOG_FILE_RVV}"
            #     echo "RVV Test for dimension $dim and topk $topk completed."
            # ) &

            # while [ "$(jobs -r | wc -l)" -ge "$(get_max_jobs)" ]; do
            #     sleep 1
            # done
            
            # RV64GCV_RV
            # (
            #     echo "Running rv test for dimension $dim and topk $topk..."
            #     { time "${GEM5_BIN}" --outdir="${M5OUT_DIR_RV}" \
            #         "${GEM5_CONFIG}" \
            #         --vlen $vlen \
            #         --elen $elen \
            #         --l1d_size $l1d_size \
            #         --resource "${RESOURCE_RV64GCV_RV}" \
            #         --prog_args="${PROG_ARGS}" \
            #         ; } 2>&1 > "${LOG_FILE_RV}"
            #     echo "RV Test for dimension $dim and topk $topk completed."
            # ) &
            
            # while [ "$(jobs -r | wc -l)" -ge "$(get_max_jobs)" ]; do
            #     sleep 1
            # done

            # RV64GC_RV
            (
                echo "Running rv test for dimension $dim and topk $topk..."
                { time "${GEM5_BIN}" --outdir="${M5OUT_DIR_RV}" \
                    "${GEM5_CONFIG}" \
                    --vlen $vlen \
                    --elen $elen \
                    --l1d_size $l1d_size \
                    --resource "${RESOURCE_RV64GC_RV}" \
                    --prog_args="${PROG_ARGS}" \
                    ; } 2>&1 > "${LOG_FILE_RV}"
                echo "RV Test for dimension $dim and topk $topk completed."
            ) &
            
            while [ "$(jobs -r | wc -l)" -ge "$(get_max_jobs)" ]; do
                sleep 1
            done
        done
    done
done

wait

echo "All tests completed."

# time build/RISCV/gem5.opt --outdir=../m5out-rv256 configs/example/bvb-board.py --vlen 1024 --resource ../build/bin/hnsw_search_riscv --prog_args="--topk 100 --dataset ${BASE_DIR}/data/random-euclidean-512-1000-100.bin"
