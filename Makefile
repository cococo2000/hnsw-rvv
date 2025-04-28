# Compiler settings
# Default C++ compiler for host compilation
CXX := g++
# Cross compiler for RISC-V target
RISCV_CXX := /opt/riscv/bin/riscv64-unknown-linux-gnu-g++
# RISCV_CXX := /opt/riscv/bin/riscv64-unknown-elf-g++
XUANTIE_CXX := /opt/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.1/bin/riscv64-unknown-linux-gnu-g++

# Directory structure
BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj
TOOLS_OBJ_DIR := $(OBJ_DIR)/tools
SRC_OBJ_DIR := $(OBJ_DIR)/src
UTILS_OBJ_DIR := $(OBJ_DIR)/utils

# Compiler flags
# -std=c++14: Use C++14 standard
# -Wall: Enable all warnings
# -Wno-unused-function: Disable warnings for unused functions
CXXFLAGS := -std=c++14 -Wall -Wno-unused-function -Wunused-variable
# Linker flags: enable static linking
LDFLAGS := -static

# Source files and executables
# Find all .cpp files in tools/ and src/ directories
SOURCES_TOOLS := $(wildcard tools/*.cpp)
SOURCES_SRC := $(wildcard src/*.cpp)
SOURCES_UTILS := $(wildcard utils/*.cpp)

# Generate object file names
OBJECTS_TOOLS := $(SOURCES_TOOLS:%.cpp=$(TOOLS_OBJ_DIR)/%.o)
OBJECTS_SRC := $(SOURCES_SRC:%.cpp=$(SRC_OBJ_DIR)/%.o)

# Generate executable names
EXECUTABLES_TOOLS := $(SOURCES_TOOLS:tools/%.cpp=$(BIN_DIR)/%)
EXECUTABLES_SRC := $(SOURCES_SRC:src/%.cpp=$(BIN_DIR)/%)

# HDF5 library configuration
# Get compilation flags for HDF5 using pkg-config
HDF5_CFLAGS := $(shell pkg-config --cflags hdf5)
# Get linker flags for HDF5 and add C++ HDF5 library
HDF5_LDFLAGS := $(shell pkg-config --libs hdf5) -lhdf5_cpp

# Debug configuration
# Usage: make DEBUG=1 for debug build
DEBUG ?= 0
ifeq ($(DEBUG), 1)
    # Debug build: add debugging symbols and DEBUG macro
    CXXFLAGS += -g -DDEBUG
else
    # Release build: enable optimization level 0
    CXXFLAGS += -O2
endif

# Create necessary directories
create_dirs:
	@mkdir -p $(BIN_DIR)

# Default target: build all executables
all: create_dirs $(EXECUTABLES_TOOLS) $(EXECUTABLES_SRC)

# Compilation rules
# Rule for tools executables
$(BIN_DIR)/%: tools/%.cpp $(SOURCES_UTILS)
	$(CXX) $(CXXFLAGS) $(HDF5_CFLAGS) -o $@ $^ $(HDF5_LDFLAGS)

# Rule for src executables
$(BIN_DIR)/%: src/%.cpp $(SOURCES_UTILS)
# Rule for object files
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
# Compile with different SIMD instruction sets
# $(CXX) $(CXXFLAGS) -mno-sse -o $@_nosse $^ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -msse     -o $@_sse    $^ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -msse4.2  -o $@_sse42  $^ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -mavx2    -o $@_avx    $^ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -mavx512f -o $@_avx512 $^ $(LDFLAGS)
# Cross compile for RISC-V on Gem5
	$(RISCV_CXX) $(CXXFLAGS) -march=rv64gc            -o $@_rv64gc_rv   $^ $(LDFLAGS)
	$(RISCV_CXX) $(CXXFLAGS) -march=rv64gcv           -o $@_rv64gcv_rv  $^ $(LDFLAGS)
	$(RISCV_CXX) $(CXXFLAGS) -march=rv64gcv -DUSE_RVV -o $@_rv64gcv_rvv $^ $(LDFLAGS)
# # Xuantie C908v on FPGA
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=c908v           -o $@_c908v_rv  $^ $(LDFLAGS)
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=c908v -DUSE_RVV -o $@_c908v_rvv $^ $(LDFLAGS)
# # Xuantie C906fdv on FPGA
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=c906fdv           -o $@_c906fdv_rv  $^ $(LDFLAGS)
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=c906fdv -DUSE_RVV -o $@_c906fdv_rvv $^ $(LDFLAGS)
# # Xuantie C907fdvm on FPGA
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=c907fdvm           -o $@_c907fdvm_rv  $^ $(LDFLAGS)
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=c907fdvm -DUSE_RVV -o $@_c907fdvm_rvv $^ $(LDFLAGS)
# # Xuantie C920v2 on FPGA
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=c920v2           -o $@_c920v2_rv  $^ $(LDFLAGS)
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=c920v2 -DUSE_RVV -o $@_c920v2_rvv $^ $(LDFLAGS)
# # Xuantie C920v3 on FPGA
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=c920v3           -o $@_c920v3_rv  $^ $(LDFLAGS)
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=c920v3 -DUSE_RVV -o $@_c920v3_rvv $^ $(LDFLAGS)
# # Xuantie R908fdv on FPGA
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=r908fdv           -o $@_r908fdv_rv  $^ $(LDFLAGS)
# 	$(XUANTIE_CXX) $(CXXFLAGS) -mcpu=r908fdv -DUSE_RVV -o $@_r908fdv_rvv $^ $(LDFLAGS)

# For Xuantie C910 LicheePi Module 4A
# $(XUANTIE_CXX) $(CXXFLAGS) -march=rv64gcv0p7           -o $@_xuantie     $^ $(LDFLAGS)
# $(XUANTIE_CXX) $(CXXFLAGS) -march=rv64gcv0p7 -DUSE_RVV -o $@_xuantie_rvv $^ $(LDFLAGS)

# riscv64gc vs riscv64gcv
# $(RISCV_CXX) $(CXXFLAGS) -march=rv64gc                        -o $@_riscv     $^ $(LDFLAGS)
# $(RISCV_CXX) $(CXXFLAGS) -march=rv64gcv -mabi=lp64d -DUSE_RVV -o $@_riscv_rvv $^ $(LDFLAGS)


# Clean target: remove all generated files and directories
clean:
	rm -rf $(BUILD_DIR)

# Force rebuild: clean and then build
rebuild: clean create_dirs all

# Force recompilation without cleaning
force:
	$(MAKE) -B all

# Declare phony targets
.PHONY: all clean rebuild create_dirs
