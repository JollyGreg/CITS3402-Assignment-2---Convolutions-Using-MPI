// Student Name: Liam Bush
// Student Number: 24227223
// Student Name: Amir Husain
// Student Number: 23380159

#ifndef MATRIX
#define MATRIX

float *alloc_matrix(int H, int W);
void save_matrix(const char *filename, float *arr, int H, int W);
float *load_matrix(const char *filename, int *outH, int *outW);
void print_matrix(float *arr, int H, int W);

#endif
