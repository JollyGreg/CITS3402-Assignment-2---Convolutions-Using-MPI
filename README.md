# CITS3402-Assignment-2---Convolutions-Using-MPI
### Student: Liam Bush (24227223)
### Student: Amir Husain (23380159)

# Using the Program
```make```

### Using MPI with convolutions

```-n``` flag decides the number of processes that will be created using mpi

```mpiexec Convolution_stride -f "data/f_10x10" -g "data/g_3x3" -o "data/o_10x10" -H 10 -W 10 -kH 3 -kW 3 -sH 1 -sW 1```

### generate matrix and save to the files given

```./Convolution_stride -f "data/f_10x10" -g "data/g_3x3" -o "data/o_10x10" -H 10 -W 10 -kH 3 -kW 3 -sH 1 -sW 1```

### load matrix from files and save to the output file

```./Convolution_stride -f "data/f_10x10" -g "data/g_3x3" -o "data/o_10x10" -sH 1 -sW 1```

## REMINDER: REMOVE NOTES WHEN SUBMITTING
NOTE: I used a 1d array as the matrix just like flattening it out which apparently speeds up
Awesome, probably due to cache hits? would be able to write that in the report