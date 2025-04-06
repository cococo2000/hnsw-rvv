#!/bin/bash

set -ex
max_jobs=78

cd gem5

# dimensions=(8 16 32 64 128 256 512 1024 2048 3072 4096)
dimensions=(8 16 32 64 128 256 512 1024 2048 4096)
# topks=(1 5 10 50 100)
topks=(100)
train=1000
test=100

vlens=(128 256 512 1024)
elen=64

for vlen in "${vlens[@]}"; do
    if [ ! -d "../logs-vlen${vlen}-elen${elen}" ]; then
        mkdir -p ../logs-vlen${vlen}-elen${elen}
    fi

    if [ ! -d "../output-vlen${vlen}-elen${elen}" ]; then
        mkdir -p ../output-vlen${vlen}-elen${elen}
    fi

    for dim in "${dimensions[@]}"; do
        for topk in "${topks[@]}"; do
            (
                echo "Running rvv test for dimension $dim and topk $topk..."
                { time build/RISCV/gem5.opt --outdir=../output-vlen${vlen}-elen${elen}/m5out-rvv-random-euclidean-${dim}-${train}-${test}-top${topk} \
                    configs/example/bvb-board.py \
                    --vlen $vlen \
                    --elen $elen \
                    --resource ../build/bin/hnsw_search_riscv_rvv \
                    --prog_args="--topk ${topk} --dataset /home/zxge/VDB/hnsw/data/random-euclidean-${dim}-${train}-${test}.bin" \
                    ; } 2>&1 > ../logs-vlen${vlen}-elen${elen}/rvv-random-euclidean-${dim}-${train}-${test}-top${topk}.log
                echo "RVV Test for dimension $dim and topk $topk completed."
            ) &

            while [ "$(jobs -r | wc -l)" -ge "$max_jobs" ]; do
                sleep 1
            done
            
            (
                echo "Running rv test for dimension $dim and topk $topk..."
                { time build/RISCV/gem5.opt --outdir=../output-vlen${vlen}-elen${elen}/m5out-rv-random-euclidean-${dim}-${train}-${test}-top${topk} \
                    configs/example/bvb-board.py \
                    --vlen $vlen \
                    --elen $elen \
                    --resource ../build/bin/hnsw_search_riscv \
                    --prog_args="--topk ${topk} --dataset /home/zxge/VDB/hnsw/data/random-euclidean-${dim}-${train}-${test}.bin" \
                    ; } 2>&1 > ../logs-vlen${vlen}-elen${elen}/rv-random-euclidean-${dim}-${train}-${test}-top${topk}.log
                echo "RV Test for dimension $dim and topk $topk completed."
            ) &
            
            while [ "$(jobs -r | wc -l)" -ge "$max_jobs" ]; do
                sleep 1
            done
        done
    done
done

wait

echo "All tests completed."

# time build/RISCV/gem5.opt --outdir=../m5out-rv256 configs/example/bvb-board.py --vlen 1024 --resource ../build/bin/hnsw_search_riscv --prog_args="--topk 100 --dataset /home/zxge/VDB/hnsw/data/random-euclidean-512-1000-100.bin" 
