// Student Name: Liam Bush
// Student Number: 24227223
// Student Name: Amir Husain
// Student Number: 23380159

// To Do:
// - input handling
//      - kH and kW handling (not sure how to have multiple character flags)
// - convolution basics from input
// - padding logic
// - stride logic
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

#include <bits/getopt_core.h>


// Prototypes
void create_matrix(char *filename, float **f, int H, int W);
void write_matrix(char *filename, float **f, int H, int W);
void randomize_matrix(float **f, int H, int W);

void free_matrix(float **f, int H, int W);

void conv2d_stride(float **f, int H, int W, float **g, int kH, int kW, int sH, int sW, float **output);
// float **f, // input feature map (padded)
// int H, int W, // global input size
// float **g, // kernel
// int kH, int kW, // kernel size
// int sH, int sW, // stride in height and width
// float **output // local output
// MPI_Comm comm // communicator


void conv2d_stride(
    float **f, int H, int W, 
    float **g, int kH, int kW, 
    int sH, int sW, float **output) {

};

void create_matrix(char *filename, float **f, int H, int W) {
    // Open a file in read mode
    FILE *fptr = fopen(filename, "r");
    if (fptr == NULL) { perror("Error opening file"); exit(EXIT_FAILURE); }

    // Store the content of the header
    if(fscanf(fptr, "%d %d", &W, &H) != 2) {
        perror("Header read Failed"); fclose(fptr) ; exit(EXIT_FAILURE);
    }

    // Initialize Matrix and store the contents of the file
    f = malloc(sizeof(float*) * (W));
    if (!f) { perror("malloc failure, initializing rows"), exit(EXIT_FAILURE); }

    for (int row = 0; row < W; row++) {
        f[row] = calloc(H, sizeof(float));
        if (!f[row]) { perror("calloc failure"), exit(EXIT_FAILURE); }
    }

    // Read the rest of the lines
    for (int row = 0; row < W; row++) {
        for (int col = 0; col < H; col++) {
            if (fscanf(fptr, "%f", &f[row][col]) != 1) {
                perror("data read failed"); fclose(fptr); exit(EXIT_FAILURE);
            }
        }
    }

    // Close the file
    fclose(fptr);
}

void randomize_matrix(float **f, int H, int W) {
    // Follows same structure as create_matrix, just has rand input
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

void free_matrix(float **f, int H, int W) {
    if (!f) return;
    for (int row = 0; row < W; row++) {
        free(f[row]);
    }
    free(f);
}

int main(int argc, char *argv[]) {
    int opt;

    char *feature_map_file = NULL;
    char *kernel_file = NULL;
    char *output = NULL;
    int test = 0;

    char *H = NULL;
    char *W = NULL;
    char *kH = NULL;
    char *kW = NULL;
    char *sH = NULL;
    char *sW = NULL;

    while((opt = getopt(argc, argv, "f:g:H:W:kH:kW:sH:sW:o:")) != -1) 
    {
        switch(opt) 
        { 
            case 'f': 
                printf("Feature Map: %s\n", optarg); 
                feature_map_file = optarg;
                break; 
            case 'g': 
                printf("Kernel: %s\n", optarg);
                kernel_file = optarg;
                break; 
            case 'H':
                printf("Map Height:%s\n", optarg);
                H = optarg;
                break;
            case 'W':
                printf("Map Width:%s\n", optarg);
                W = optarg;
                break; 
            case 'kH': // Not sure how to handle multiple char flags
                printf("Kernel Height:%s\n", optarg);
                kH = optarg;
                break; 
            case 'kW':
                printf("Kernel Width:%s\n", optarg);
                kW = optarg;
                break; 
            case 'sH':
                printf("Stride Height:%s\n", optarg);
                sH = optarg;
                break; 
            case 'sW':
                printf("Stride Width:%s\n", optarg);
                sW = optarg;
                break; 
            case 'o': 
                printf("Output File:%s\n", optarg);
                output = optarg;
                break; 
        } 
    } printf("\n");
}