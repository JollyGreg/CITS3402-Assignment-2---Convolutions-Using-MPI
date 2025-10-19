# Parallel 2D Convolution (MPI + OpenMP)

## Overview

This project implements stride-based 2D convolution for large matrices using a hybrid parallel approach:
- MPI distributes work across ranks
- OpenMP parallelises computation within each rank

It supports random matrix generation, file I/O, configurable feature and kernel and stride sizes. 

## Build Instructions

The Makefile supports two environments. Default is "kaya" (cc); you can also build for "setonix" :

```bash
# Default (kaya)
make

# Explicit targets
make kaya
make setonix

# Clean
make clean
```

This produces the executable `Convolution_stride`.

## Usage

Run the program (with srun/mpiexec if using multiple ranks):

```bash
# Single process
srun /mpiexec -np  n ./Convolution_stride -H <height> -W <width> -kH <kernel_height> -kW <kernel_width> \
  [-f <feature_file>] [-g <kernel_file>] [-o <output_file>] \
  [-sH <stride_height>] [-sW <stride_width>] [-p <0|1>] [-serial <0|1>]
```

### Arguments

- `-H <height>`: Input matrix height (optional if `-f` provided)
- `-W <width>`: Input matrix width (optional if `-f` provided)
- `-kH <kernel_height>`: Kernel height (optional if `-g` provided)
- `-kW <kernel_width>`: Kernel width (optional if `-g` provided)
- `-f <feature_file>`: Feature matrix file path (optional); if set without `-H/-W`, the matrix is loaded
- `-g <kernel_file>`: Kernel matrix file path (optional); if set without `-kH/-kW`, the matrix is loaded
- `-o <output_file>`: Output file path to save the result (optional)
- `-sH <stride_height>`: Vertical stride (default: required to be provided)
- `-sW <stride_width>`: Horizontal stride (default: required to be provided)
- `-p <0|1>`: Print matrices (default: 1)
- `-serial <0|1>`: If 1 on rank 0, runs serial `conv2d_stride` instead of MPI version (for comparison)


## Examples

Generate random matrices and run (single process):
```bash
# Example with kaya (4 ranks)
mpirun -np 1 ./Convolution_stride -H 1024 -W 1024 -kH 3 -kW 3 -sH 1 -sW 1
```

Run with MPI across 8 ranks, suppress printing, save output:
```bash
mpirun -np 8 ./Convolution_stride -H 2048 -W 2048 -kH 5 -kW 5 -sH 2 -sW 2 -p 0 -o out_2048.txt
```

Load matrices from files and save output:
```bash
mpirun -np 4 ./Convolution_stride -f data/f0.txt -g data/g0.txt -sH 1 -sW 1 -o data/out.txt -p 0
```

## Input/Output

- If matrix dimensions are provided without files, random matrices are generated on rank 0 and (optionally) saved to files if `-f`/`-g` are given.
- If file names are provided without dimensions, matrices are loaded from those files.
- Output is saved to the specified file if `-o` is given; it is also printed when `-p 1`.

