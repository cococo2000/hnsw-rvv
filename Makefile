# Compiler settings
# Default C++ compiler for host compilation
CXX := g++
# Cross compiler for RISC-V target
RISCV_CXX := /opt/riscv/bin/riscv64-unknown-linux-gnu-g++
# RISCV_CXX := /opt/riscv/bin/riscv64-unknown-elf-g++

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
# Compile with different SIMD instruction sets
	$(CXX) $(CXXFLAGS) -msse -o $@_sse $^ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -mavx -o $@_avx $^ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -mavx512f -o $@_avx512 $^ $(LDFLAGS)
# Cross compile for RISC-V
	$(RISCV_CXX) $(CXXFLAGS)           -o $@_riscv     $^ $(LDFLAGS)
	$(RISCV_CXX) $(CXXFLAGS) -DUSE_RVV -o $@_riscv_rvv $^ $(LDFLAGS)
# $(RISCV_CXX) $(CXXFLAGS) -march=rv64gc                        -o $@_riscv     $^ $(LDFLAGS)
# $(RISCV_CXX) $(CXXFLAGS) -march=rv64gcv -mabi=lp64d -DUSE_RVV -o $@_riscv_rvv $^ $(LDFLAGS)
# Rule for object files
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

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
