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
//          - need to have edit the conv2d_stride func so that each process only does its share of the work
//      - gathering all answers together
// - testing
// - report writting

#include <stdio.h> 
#include <unistd.h>
#include <string.h> 
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>

#include "matrix.h"
#include "conv2d_stride.h"

#include <bits/getopt_core.h>


// Prototypes
void create_matrix(char *filename, float **f, int H, int W);
void write_matrix(char *filename, float **f, int H, int W);
void randomize_matrix(float **f, int H, int W);

void free_matrix(float **f, int H, int W);

// void conv2d_stride(float **f, int H, int W, float **g, int kH, int kW, int sH, int sW, float **output);
// float **f, // input feature map (padded)
// int H, int W, // global input size
// float **g, // kernel
// int kH, int kW, // kernel size
// int sH, int sW, // stride in height and width
// float **output // local output
// MPI_Comm comm // communicator


// void conv2d_stride(
//    float **f, int H, int W, 
//    float **g, int kH, int kW, 
//    int sH, int sW, float **output) {
//};

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
    MPI_Init(&argc, &argv);
    int pid, np;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    // master process
    if (pid == 0) {
        // Seed random number generator
        srand(42);

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

        // Print the feature and kernel matrix
        printf("Features (f)\n");
        print_matrix(f, H, W);
        printf("Kernels (g)\n");
        print_matrix(g, kH, kW);  
    
        // Start timer and convolute
        int o_H = (H + sH - 1) / sH;
        int o_W = (W + sW - 1) / sW;
        float *o = alloc_matrix(o_H, o_W);
        
        clock_t CPU_begin = clock();
        double WALL_begin = omp_get_wtime(); 

        // should sending be counted on the timer? idk
        int process_workload = H*W / np;

        for (int i = 1; i < np; i++) {            
            MPI_Send(&f, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&H, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&W, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&g, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&kH, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&kW, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&sH, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&sW, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&o, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&o, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&o, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        for (int i = 1; i < np; i++) {
            int success;
            MPI_Recv(&success, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Master - success: %i", success);
        }
        clock_t CPU_end = clock();
        double WALL_end = omp_get_wtime(); 
        double CPU_time = (double)(CPU_end - CPU_begin) / CLOCKS_PER_SEC; //time in seconds
        double WALL_time = WALL_end - WALL_begin;

        // Performance
        printf("sH = %d, sW = %d\n", sH, sW);
        printf("The CPU time spent for %dx%d * %dx%d was %fs\n", H, W, kH, kW, CPU_time);
        printf("The WALL time spent for %dx%d * %dx%d was %fs\n", H, W, kH, kW, WALL_time);

        // free(f);
        // free(g);
        // free(o);
    } 
    // slave processes should recieve their portions of the processing
    else {
        int H = 0, W = 0, kH = 0, kW = 0, sH = 0, sW = 0;
        float *f = NULL, *g = NULL, *o = NULL;
        int start_H, start_W;
        MPI_Recv(&f, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&H, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&W, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&g, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&kH, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&kW, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&sH, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&sW, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&o, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&start_H, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&start_W, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("pid: %i\n", pid);
        
        mpi_conv2d_stride(f, H, W, g, kH, kW, sH, sW, o, MPI_COMM_WORLD);

        // sends message to confirm completion
        int success = 1;
        MPI_Send(&success, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();

    // Save output if requested
    // if (output_file) save_matrix(output_file, o, o_H, o_W);
    // printf("Output (o)\n");
    // print_matrix(o, o_H, o_W);  
}