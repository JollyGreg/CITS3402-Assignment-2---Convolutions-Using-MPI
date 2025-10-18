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
// DONE:when matrix is small ranks might be unnecessarily used (i think since 4 ranks being used, bad when rows < 4)
//      fix by allocating work based on number of output elements rather than rows

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
    int print = 1;

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
        else if (!strcmp(argv[i], "-p")) print = (int)atoll(argv[++i]);
        else {
            printf("Unknown argument: %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
	}

    float *f = NULL, *g = NULL;
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
        return 0;
    }

    // Print the feature and kernel matrix (rank 0 only)
    if (rank == 0 && print == 1) {
        printf("Features (f)\n");
        print_matrix(f, H, W);
        printf("Kernels (g)\n");
        print_matrix(g, kH, kW);
    }

    // Calculate output dimensions
    int o_H = (H + sH - 1) / sH;
    int o_W = (W + sW - 1) / sW;
    
    // Only rank 0 allocates the output array
    float *o = NULL;
    if (rank == 0) {
        o = alloc_matrix(o_H, o_W);
    }
    
    // Synchronize all ranks before starting computation
    MPI_Barrier(MPI_COMM_WORLD);
    clock_t CPU_begin = clock();
    double WALL_begin = omp_get_wtime(); 

    mpi_conv2d_stride(f, H, W, g, kH, kW, sH, sW, o, MPI_COMM_WORLD);
    //conv2d_stride(f, H, W, g, kH, kW, sH, sW, o);


    clock_t CPU_end = clock();
    double WALL_end = omp_get_wtime(); 
    double CPU_time = (double)(CPU_end - CPU_begin) / CLOCKS_PER_SEC; //time in seconds
    double WALL_time = WALL_end - WALL_begin;

    // Save and print output from rank 0 only
    if (rank == 0) {
        if (output_file) save_matrix(output_file, o, o_H, o_W);
        if (print == 1) {
            printf("\nOutput (o)\n");
            print_matrix(o, o_H, o_W);
        }

        // Performance
        printf("sH = %d, sW = %d\n", sH, sW);
        printf("The CPU time spent for %dx%d * %dx%d was %fs\n", H, W, kH, kW, CPU_time);
        printf("The WALL time spent for %dx%d * %dx%d was %fs\n", H, W, kH, kW, WALL_time);
    }

    free(f);
    free(g);
    free(o);

    MPI_Finalize();
}