# HNSW Vector Search Implementation with RISC-V Vector Extension

A high-performance vector search implementation based on HNSW (Hierarchical Navigable Small World) algorithm, with optimizations for RISC-V Vector Extension (RVV).

## Overview

This project implements a vector search system using the HNSW algorithm, with special focus on RISC-V architecture optimization. It provides efficient vector similarity search capabilities with support for various vector dimensions and search parameters.

## Features

- **HNSW Implementation**
  - Efficient hierarchical graph-based indexing
  - Configurable parameters (M, ef_construction)
  - Support for various vector dimensions

- **RISC-V Optimization**
  - RISC-V Vector Extension (RVV) support
  - Optimized distance calculations
  - Vectorized operations for improved performance

- **Data Management**
  - Binary dataset format support
  - Random dataset generation
  - Efficient data loading and storage

- **Performance Evaluation**
  - Recall rate calculation
  - Detailed performance timing
  - Comprehensive testing framework

## Project Structure

```
.
├── gem5/                 # gem5 simulator integration
├── test/                 # Test code and benchmarks
├── include/              # Header files
│   └── hnswlib/         # HNSW library
├── script/              # Utility scripts
├── results/             # Results directory
├── build/               # Build directory
├── data/                # Dataset directory
├── tools/               # Utility programs
├── utils/               # Utility functions
└── src/                 # Source code
```

## Building the Project

### Prerequisites

- RISC-V toolchain
- gem5 simulator (optional, for simulation)
- C++14 compatible compiler
- Make build system

### Build Commands

```bash
# Build all targets
make

# Clean build files
make clean

# Rebuild everything
make rebuild

# Force recompilation
make force
```

## Usage

### Creating Datasets

```bash
./build/bin/create_dataset --dim 32 --train 1000 --test 100 --topk 10 --dataset my_dataset
```

### Running Search

```bash
./build/bin/hnsw_search --dataset data/my_dataset.bin --topk 10
```

### Running Tests

```bash
# Run all tests
./bvb-test-all.sh

# Run specific test configuration
./bvb-work.sh
```

## Testing Framework

The project includes a comprehensive testing framework that supports:

- Multiple vector dimensions
- Various top-k values
- Different vector lengths
- Parallel test execution
- Performance comparison between RV and RVV implementations

## Performance Evaluation

The system provides detailed performance metrics:

- Search time
- Recall rate
- Memory usage
- Vector operation efficiency

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- HNSW algorithm implementation based on [hnswlib](https://github.com/nmslib/hnswlib)
- RISC-V Vector Extension (RVV) support from [riscv-v-spec](https://github.com/riscv/riscv-v-spec)
- gem5 simulator integration from [gem5](https://github.com/gem5/gem5)
