// Student Name: Liam Bush
// Student Number: 24227223
// Student Name: Amir Husain
// Student Number: 23380159

#include "conv2d_stride.h"
#include <omp.h>
#include <mpi.h>
#include <stdio.h>

// the mpi version of convolution should have an index value that dictates where the convolution starts and ends.
void mpi_conv2d_stride(float *f, int H, int W, float *g, int kH, int kW, int sH, int sW, float *output, MPI_Comm comm) {
    int pid, np;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    printf("pid: %i, np: %i\n", pid, np);

    int array_split_per_process = H*W / np;
    printf("each slave process will compute in increments of %i\n", array_split_per_process);

    // master process that gathers output together
    if (pid == 0) {
        
        for (int i = 1; i < np - 1; i++) {
            // each slave process returns their output 
            //MPI_Recv(&array_split_per_process, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &status);
        }
    } 
    // slave processes, receive arrays and compute using conv2d_stride
    else {
        int index = array_split_per_process * pid;
        printf("pid: %i, index: %i\n", pid, index);

        // allocate a temporary matrix for just the size the process needs?
        //conv2d_stride(f, H, W, g, kH, kW, sH, sW, o);
    }


    // For odd-sized kernels, the anchor is the exact center.
    // For even-sized kernels, anchor is chosen so the kernel is slightly top-left biased.
    int anchorH = kH / 2;
    int anchorW = kW / 2;
    if (kH % 2 == 0){
        anchorH = kH / 2 - 1;
    }
    if (kW % 2 == 0){
        anchorW = kW / 2 - 1;
    }

    // Initialize output counter
    int out_idx = 0;

    // Loop over output positions with stride
    for (int i = 0; i < H; i += sH) {
        for (int j = 0; j < W; j += sW) {
            float sum = 0.0f;

            // Loop over every element in the kernel
            for (int m = 0; m < kH; m++) {
                for (int n = 0; n < kW; n++) {
                    // The corresponding input coordinates
                    int x = i + m - anchorH;
                    int y = j + n - anchorW;
                    
                    // If input coordinates are inside bounds, fetch value
                    // Otherwise, treat as zero (zero-padding)
                    float val = 0.0f;
                    if (x >= 0 && x < H && y >= 0 && y < W) {
                        val = f[x * W + y];
                    }
                    // Multiply input value by kernel value and accumulate
                    sum += val * g[m * kW + n];
                }
            }
            // Store result in output array (flattened)
            output[out_idx] = sum;
            out_idx++;
        }
    }
}

void conv2d_stride(float *f, int H, int W, float *g, int kH, int kW, int sH, int sW, float *output) {
    // For odd-sized kernels, the anchor is the exact center.
    // For even-sized kernels, anchor is chosen so the kernel is slightly top-left biased.
    int anchorH = kH / 2;
    int anchorW = kW / 2;
    if (kH % 2 == 0){
        anchorH = kH / 2 - 1;
    }
    if (kW % 2 == 0){
        anchorW = kW / 2 - 1;
    }

    // Initialize output counter
    int out_idx = 0;

    // Loop over output positions with stride
    for (int i = 0; i < H; i += sH) {
        for (int j = 0; j < W; j += sW) {
            float sum = 0.0f;

            // Loop over every element in the kernel
            for (int m = 0; m < kH; m++) {
                for (int n = 0; n < kW; n++) {
                    // The corresponding input coordinates
                    int x = i + m - anchorH;
                    int y = j + n - anchorW;
                    
                    // If input coordinates are inside bounds, fetch value
                    // Otherwise, treat as zero (zero-padding)
                    float val = 0.0f;
                    if (x >= 0 && x < H && y >= 0 && y < W) {
                        val = f[x * W + y];
                    }
                    // Multiply input value by kernel value and accumulate
                    sum += val * g[m * kW + n];
                }
            }
            // Store result in output array (flattened)
            output[out_idx] = sum;
            out_idx++;
        }
    }
}