#ifndef MATRIX_IO_H
#define MATRIX_IO_H

float *alloc_matrix(int H, int W);
void save_matrix(const char *filename, float *arr, int H, int W);
float *load_matrix(const char *filename, int *outH, int *outW);
void print_matrix(float *arr, int H, int W);

#endif
