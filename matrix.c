// Student Name: Liam Bush
// Student Number: 24227223
// Student Name: Amir Husain
// Student Number: 23380159

#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>

// Allocate float32 matrix
float *alloc_matrix(int H, int W) {
    return (float *)malloc(H * W * sizeof(float));
}

// Load matrix from file
float *load_matrix(const char *filename, int *outH, int *outW) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Error: Could not open file '%s' for reading.\n", filename);
        return NULL;
    }
    int fileH, fileW;
    if (fscanf(f, "%d %d", &fileH, &fileW) != 2) {
        printf("Error: Could not read matrix dimensions from '%s'.\n", filename);
        fclose(f);
        return NULL;
    }
    *outH = (int)fileH;
    *outW = (int)fileW;
    float *arr = alloc_matrix(fileH, fileW);
    for (int i = 0; i < fileH; i++) {
        for (int j = 0; j < fileW; j++) {
            if (fscanf(f, "%f", &arr[i * fileW + j]) != 1) {
                printf("Error: Could not read matrix value at (%d,%d) from '%s'.\n", i+1, j+1, filename);
                fclose(f);
                free(arr);
                return NULL;
            }
        }
    }
    fclose(f);
    return arr;
}

// Save matrix to file
void save_matrix(const char *filename, float *arr, int H, int W) {
    FILE *f = fopen(filename, "w");
    fprintf(f, "%d %d\n", H, W);
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            fprintf(f, "%.3f ", arr[i * W + j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

// Print matrix
void print_matrix(float *arr, int H, int W) {
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
            printf("%.3f ", arr[i * W + j]);
		}
		printf("\n");
	}
    printf("\n");
}