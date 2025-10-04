// Student Name: Liam Bush
// Student Number: 24227223
// Student Name: Amir Husain
// Student Number: 23380159

// To Do:
// - input handling
//      - DONE (check?): kH and kW handling (not sure how to have multiple character flags)
// - DONE: generate/save matrix
// - DONE: convolution basics from input
// - DONE: padding logic
// - DONE: stride logic
// - introducing MPI
// - testing
// - report writting

#include <stdio.h> 
#include <unistd.h>
#include <string.h> 
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

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
            fprintf(fp, "%.3f ", f[map_r][map_c]); // Format to 2 decimal places
        }
        fprintf(fp, "\n");
    }

    // Close the file
    fclose(fp);
}

int main(int argc, char *argv[]) {
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
    conv2d_stride(f, H, W, g, kH, kW, sH, sW, o);
    clock_t CPU_end = clock();
    double WALL_end = omp_get_wtime(); 
    
    double CPU_time = (double)(CPU_end - CPU_begin) / CLOCKS_PER_SEC; //time in s
    double WALL_time = WALL_end - WALL_begin;

    // Save output if requested
    if (o) save_matrix(output_file, o, o_H, o_W);
    printf("Output (o)\n");
    print_matrix(o, o_H, o_W);  

    // Performance
    printf("The CPU time spent for %dx%d * %dx%d was %fs\n", H, W, kH, kW, CPU_time);
    printf("The WALL time spent for %dx%d * %dx%d was %fs\n", H, W, kH, kW, WALL_time);

    free(f);
    free(g);
    free(o);
}