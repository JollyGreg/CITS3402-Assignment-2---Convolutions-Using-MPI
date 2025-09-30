// Student Name: Liam Bush
// Student Number: 24227223
// Student Name:
// Student Number: 

#include <stdio.h> 
#include <unistd.h>
#include <string.h> 
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

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
    int rows_num = W;
    int cols_num = H;

    f = malloc(sizeof(float*) * (rows_num));
    if (!f) { perror("malloc failure, initializing rows"), exit(EXIT_FAILURE); }

    for (int row = 0; row < rows_num; row++) {
        f[row] = calloc(cols_num, sizeof(float));
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
    int rows_num = W;
    int cols_num = H;

    f = malloc(sizeof(float*) * (rows_num));
    if (!f) { perror("malloc failure, initializing rows"), exit(EXIT_FAILURE); }

    for (int row = 0; row < rows_num; row++) {
        f[row] = calloc(cols_num, sizeof(float));
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
    int rows_num = W;
    for (int row = 0; row < rows_num; row++) {
        free(f[row]);
    }
    free(f);
}

int main(int argc, char *argv[]) {
    int opt;

    while((opt = getopt(argc, argv, "f:g:o:H:W:w:h:t")) != -1) 
    {
        switch(opt) 
        { 
        } 
    } printf("\n");
}