COMPILER="/opt/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.1/bin/riscv64-unknown-linux-gnu-g++"
CXXFLAGS="-std=c++14 -Wall -Wno-unused-function -Wunused-variable -O2"
SRC_FILES="src/hnsw_search.cpp utils/dataset.cpp"
OUTPUT_DIR="build-licheepi4a"


# Compile mcpu Seccess
# # r920
# $COMPILER $CXXFLAGS -mcpu=r920 -DUSE_RVV -o $OUTPUT_DIR/hnsw_search_r920_rvv $SRC_FILES -static
# $COMPILER $CXXFLAGS -mcpu=r920           -o $OUTPUT_DIR/hnsw_search_r920_rv  $SRC_FILES -static
# # c920
$COMPILER $CXXFLAGS -march=rv64gcv0p7 -DUSE_RVV -o $OUTPUT_DIR/hnsw_search_c910_rvv $SRC_FILES -static
$COMPILER $CXXFLAGS -march=rv64gcv0p7           -o $OUTPUT_DIR/hnsw_search_c910_rv  $SRC_FILES -static


# No support / Compile RVV Error
# r910
# c910

# On LicheePi 4A Fail
# c908v
# c907fdvm
# c920v2
# c920v3
# r908fdv

# On LicheePi 4A Succeed
# c906fdv
