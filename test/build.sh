set -ex

/opt/riscv/bin/riscv64-unknown-linux-gnu-g++ -O2 --static -DUSE_RVV test_rvv.cpp ../utils/dataset.cpp -o test_rvv
/opt/riscv/bin/riscv64-unknown-linux-gnu-g++ -O2 --static           test_rvv.cpp ../utils/dataset.cpp -o test_rv
g++ -O2 --static test_rvv.cpp ../utils/dataset.cpp -o test_x86

# /opt/riscv/bin/riscv64-unknown-linux-gnu-g++ -O2 -march=rv64gcv --static -DUSE_RVV test_rvv.cpp ../utils/dataset.cpp -o test_rvv
# /opt/riscv/bin/riscv64-unknown-linux-gnu-g++ -O2 -march=rv64gc  --static           test_rvv.cpp ../utils/dataset.cpp -o test_rv
 
/opt/riscv/bin/riscv64-unknown-linux-gnu-objdump -d test_rvv >  test_rvv.s
/opt/riscv/bin/riscv64-unknown-linux-gnu-objdump -d test_rv >  test_rv.s


# /opt/riscv/bin/riscv64-unknown-linux-gnu-g++ -O2 --static test.cpp ../utils/dataset.cpp -o test

/opt/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.1/bin/riscv64-unknown-linux-gnu-g++ -O2 -mcpu=c908v --static -DUSE_RVV test_rvv.cpp ../utils/dataset.cpp -o test_rvv_xuantie
/opt/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.1/bin/riscv64-unknown-linux-gnu-g++ -O2 -mcpu=c908v --static           test_rvv.cpp ../utils/dataset.cpp -o test_rv_xuantie
