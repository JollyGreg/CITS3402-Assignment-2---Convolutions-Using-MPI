// Student Name: Liam Bush
// Student Number: 24227223
// Student Name: Amir Husain
// Student Number: 23380159

// To Do:
// - input handling
//      - DONE (check?) (looks awesome, haven't seen that way of doing it before): kH and kW handling (not sure how to have multiple character flags)
// - DONE: generate/save matrix
// - DONE: convolution basics from input
// - DONE: padding logic
// - DONE: stride logic
// - introducing MPI
//      - DONE: Initializing MPI
//      - doing convolutions on array
//          - DONE: need to have edit the conv2d_stride func so that each process only does its share of the work (needs double checking)
//      - DONE: gathering all answers together
// - testing
// - report writting


// Notes:
// check since all ranks are running main do they unnecessarily all alocate matrix space and other setup
//      does it matter if each rank allocate matrix space itself or should only rank 0 do it and send to ranks?
//      potentially use MPI_Bcast to share common data (like matrix dimensions) among all ranks
//      each rank should only allocate memory for its portion of the data
// check if each rank is only doing its share (i.e. no overlap)
// DONE:when matrix is small ranks might be unnecessarily used (i think since 4 ranks being used, bad when rows < 4)
//      fix by allocating work based on number of output elements rather than rows
// i think CPU time is only for rank 0, so MPI time not included in CPU time 

#include <stdio.h> 
#include <unistd.h>
#include <string.h> 
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"
#include "conv2d_stride.h"

#include <bits/getopt_core.h>



void randomize_matrix(float **f, int H, int W) {
    // Follows same structure as load_matrix, just has rand input
    f = malloc(sizeof(float*) * (W));
    if (!f) { perror("malloc failure, initializing rows"), exit(EXIT_FAILURE); }

    for (int row = 0; row < W; row++) {
        f[row] = calloc(H, sizeof(float));
        if (!f[row]) { perror("calloc failure"), exit(EXIT_FAILURE); }
    }

    // Fill the colummns for each row
    for (int row = 0; row < W; row++) {
        for (int col = 0; col < H; col++) {
            f[row][col] = (float)rand() / (float)(RAND_MAX / 2);
        }
    }
}

void write_matrix(char *filename, float **f, int H, int W) {
    FILE *fp;

    // Open the file in write mode ("w")
    fp = fopen(filename, "w");

    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Write each float to the file, followed by a newline
    fprintf(fp, "%i %i\n", W, H);
    for (int map_r = 0; map_r < W; map_r++) {
        for (int map_c = 0; map_c < H; map_c++) {
            fprintf(fp, "%.3f ", f[map_r][map_c]); // Format to 3 decimal places
        }
        fprintf(fp, "\n");
    }

    // Close the file
    fclose(fp);
}

// https://www.geeksforgeeks.org/c/sum-of-an-array-using-mpi/

int main(int argc, char *argv[]) {
    // Seed random number generator
    srand(42);

    int rank = 0, size = 1;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char *feature_map_file = NULL;
    char *kernel_file = NULL;
    char *output_file = NULL;

    int H = 0, W = 0, kH = 0, kW = 0, sH = 0, sW = 0;

    // Parse arguments
	for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-H")) H = (int)atoll(argv[++i]);
        else if (!strcmp(argv[i], "-W")) W = (int)atoll(argv[++i]);
        else if (!strcmp(argv[i], "-kH")) kH = (int)atoll(argv[++i]);
        else if (!strcmp(argv[i], "-kW")) kW = (int)atoll(argv[++i]);
        else if (!strcmp(argv[i], "-f")) feature_map_file = argv[++i];
        else if (!strcmp(argv[i], "-g")) kernel_file = argv[++i];
        else if (!strcmp(argv[i], "-o")) output_file = argv[++i];
        else if (!strcmp(argv[i], "-sH")) sH = (int)atoll(argv[++i]);
        else if (!strcmp(argv[i], "-sW")) sW = (int)atoll(argv[++i]);
        else {
            printf("Unknown argument: %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
	}

    float *f = NULL, *g = NULL;
    
    // Only rank 0 handles file I/O and matrix generation
    if (rank == 0) {
        // Feature matrix
        // Dimensions present: generate matrix
        if (H > 0 && W > 0) {
            f = alloc_matrix(H, W);
            for (int i = 0; i < H; i++) {
                for (int j = 0; j < W; j++) {
                    f[i * W + j] = (float) rand() / RAND_MAX;
                }
            }

            // File present: save
            if (feature_map_file){
                save_matrix(feature_map_file, f, H, W);
            }
        } 

        // Only file no dimensions: load
        else if (feature_map_file) {
            f = load_matrix(feature_map_file, &H, &W);
        } 

        // Kernel matrix
        // Dimensions present: generate 
        if (kH > 0 && kW > 0) {
            g = alloc_matrix(kH, kW);
            for (int i = 0; i < kH; i++) {
                for (int j = 0; j < kW; j++) {
                    g[i * kW + j] = (float) rand() / RAND_MAX;
                }
            }
            // File present: save
            if (kernel_file){
                save_matrix(kernel_file, g, kH, kW);
            }
        } 

        // Only file: load
        else if (kernel_file) {
            g = load_matrix(kernel_file, &kH, &kW);
        }

        if (f == NULL || g == NULL){
            printf("Failed to load matrices.\n");
            if (f) free(f);
            if (g) free(g);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Start timer on rank 0 after all ranks are synchronized
    clock_t CPU_begin = 0;
    double WALL_begin = 0.0;
    if (rank == 0) {
        CPU_begin = clock();
        WALL_begin = omp_get_wtime();
    } 

    // Broadcast dimensions to all ranks
    MPI_Bcast(&H, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&W, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&kH, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&kW, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Calculate which input elements each rank needs
    int outH = (H + sH - 1) / sH;
    int outW = (W + sW - 1) / sW;
    int total_elements = outH * outW;
    int elements_per_process = total_elements / size;
    int start_element = rank * elements_per_process;
    int end_element = (rank == size - 1) ? total_elements : start_element + elements_per_process;
    
    // Calculate anchor for kernel positioning
    int anchorH = kH / 2;
    int anchorW = kW / 2;
    if (kH % 2 == 0) anchorH = kH / 2 - 1;
    if (kW % 2 == 0) anchorW = kW / 2 - 1;
    
    // Allocate based on maximum possible indices needed
    int max_local_elements = (end_element - start_element) * kH * kW;
    float *local_f = (float*)malloc(max_local_elements * sizeof(float));
    
    // Collect all input indices this rank needs and store as floats
    int idx_count = 0;
    for (int element = start_element; element < end_element; element++) {
        int out_i = element / outW;
        int out_j = element % outW;
        int base_i = out_i * sH;
        int base_j = out_j * sW;
        
        // Check all kernel positions for this output element
        for (int m = 0; m < kH; m++) {
            for (int n = 0; n < kW; n++) {
                int x = base_i + m - anchorH;
                int y = base_j + n - anchorW;
                
                if (x >= 0 && x < H && y >= 0 && y < W) {
                    int idx = x * W + y;
                    local_f[idx_count] = (float)idx;  // Store index as float temporarily
                    idx_count++;
                }
            }
        }
    }
    
    printf("Rank %d needs %d input elements\n", rank, idx_count);
    
    // Get the needed values from rank 0
    if (rank == 0) {
        // Replace indices with actual values for rank 0
        for (int i = 0; i < idx_count; i++) {
            int idx = (int)local_f[i];  // Convert back to int index
            local_f[i] = f[idx];        // Replace with actual value
        }
        
        // Send needed values to other ranks
        for (int r = 1; r < size; r++) {
            // Receive count of indices from rank r
            int r_count;
            MPI_Recv(&r_count, 1, MPI_INT, r, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Receive the indices from rank r
            float *r_indices = (float*)malloc(r_count * sizeof(float));
            MPI_Recv(r_indices, r_count, MPI_FLOAT, r, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Convert indices to values and send back
            for (int i = 0; i < r_count; i++) {
                int idx = (int)r_indices[i];  // Convert to index
                r_indices[i] = f[idx];        // Replace with value
            }
            MPI_Send(r_indices, r_count, MPI_FLOAT, r, 2, MPI_COMM_WORLD);
            
            free(r_indices);
        }
    } else {
        // Send count and indices to rank 0
        MPI_Send(&idx_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(local_f, idx_count, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
        
        // Receive values from rank 0 (now they contain actual values)
        MPI_Recv(local_f, idx_count, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    if (rank != 0) {
        //f = alloc_matrix(H, W);
        g = alloc_matrix(kH, kW);
    }
    
    // Broadcast kernel g to all ranks
    MPI_Bcast(g, kH * kW, MPI_FLOAT, 0, MPI_COMM_WORLD);
    

    // Print the feature and kernel matrix (rank 0 only)
    if (rank == 0) {
        printf("Features (f)\n");
        print_matrix(f, H, W);
        printf("Kernels (g)\n");
        print_matrix(g, kH, kW);
    }

    // Allocate output matrix for all ranks
    int o_H = (H + sH - 1) / sH;
    int o_W = (W + sW - 1) / sW;
    float *o = alloc_matrix(o_H, o_W);
    



    mpi_conv2d_stride(local_f, H, W, g, kH, kW, sH, sW, o, MPI_COMM_WORLD);
    //conv2d_stride(f, H, W, g, kH, kW, sH, sW, o);

    // All ranks return from mpi_conv2d_stride; results are gathered on rank 0
    // Only rank 0 calculates timing and prints results
    if (rank == 0) {
        clock_t CPU_end = clock();
        double WALL_end = omp_get_wtime(); 
        double CPU_time = (double)(CPU_end - CPU_begin) / CLOCKS_PER_SEC; //time in seconds
        double WALL_time = WALL_end - WALL_begin;

        // Save and print output
        if (output_file) save_matrix(output_file, o, o_H, o_W);
        printf("Output (o)\n");
        print_matrix(o, o_H, o_W);

        // Performance results
        printf("sH = %d, sW = %d\n", sH, sW);
        printf("The CPU time spent for %dx%d * %dx%d was %fs\n", H, W, kH, kW, CPU_time);
        printf("The WALL time spent for %dx%d * %dx%d was %fs\n", H, W, kH, kW, WALL_time);
    }

    free(f);
    free(g);
    free(o);

    MPI_Finalize();
}