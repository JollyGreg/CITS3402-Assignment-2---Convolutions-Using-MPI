// Student Name: Liam Bush
// Student Number: 24227223
// Student Name: Amir Husain
// Student Number: 23380159

#include "conv2d_stride.h"
#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// the mpi version of convolution should have an index value that dictates where the convolution starts and ends.
void mpi_conv2d_stride(float *f, int H, int W, float *g, int kH, int kW, int sH, int sW, float *output, MPI_Comm comm) {
    int pid, np;
    MPI_Comm_rank(comm, &pid);
    MPI_Comm_size(comm, &np);
        
    // Calculate output dimensions
    int outH = (H + sH - 1) / sH;
    int outW = (W + sW - 1) / sW;
    
    // Divide work by output elements
    int total_elements = outH * outW;
    int elements_per_process = total_elements / np;
    int start_element = pid * elements_per_process;
    int end_element = (pid == np - 1) ? total_elements : start_element + elements_per_process;
    
    printf("Process %d: computing output elements %d to %d (%d elements)\n", 
           pid, start_element, end_element-1, end_element - start_element);
    
    // Allocate local output buffer
    int local_output_size = end_element - start_element;
    float *local_output = (float*)malloc(local_output_size * sizeof(float));

    // Anchor calculation
    int anchorH = kH / 2;
    int anchorW = kW / 2;
    if (kH % 2 == 0) anchorH = kH / 2 - 1;
    if (kW % 2 == 0) anchorW = kW / 2 - 1;
    
        
    // Each process computes its assigned elements with OpenMP parallelization
    #pragma omp parallel for schedule(static)
    for (int element = start_element; element < end_element; element++) {
        // Convert linear element index to 2D output coordinates
        int out_i = element / outW;
        int out_j = element % outW;
        
        // Convert output coordinates to input coordinates
        int i = out_i * sH;
        int j = out_j * sW;
        
        float sum = 0.0f;
        
        // Convolution kernel loop with reduction
        #pragma omp simd reduction(+:sum)
        for (int mn = 0; mn < kH * kW; mn++) {
            int m = mn / kW;
            int n = mn % kW;
            
            int x = i + m - anchorH;
            int y = j + n - anchorW;
            
            float val = 0.0f;
            if (x >= 0 && x < H && y >= 0 && y < W) {
                val = f[x * W + y];
            }
            sum += val * g[mn];
        }
        
        // Calculate local index for this element
        int idx = element - start_element;
        local_output[idx] = sum;
    }
    
    // Gather results at process 0
    if (pid == 0) {
        // Copy local result to final output
        memcpy(output, local_output, local_output_size * sizeof(float));
        
        // Receive from other processes
        int offset = local_output_size;
        for (int p = 1; p < np; p++) {
            int p_start = p * elements_per_process;
            int p_end = (p == np - 1) ? total_elements : p_start + elements_per_process;
            int p_size = p_end - p_start;
            
            MPI_Recv(output + offset, p_size, MPI_FLOAT, p, 0, comm, MPI_STATUS_IGNORE);
            offset += p_size;
        }
    } else {
        // Send local result to process 0
        MPI_Send(local_output, local_output_size, MPI_FLOAT, 0, 0, comm);
    }
    
    free(local_output);
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