// Student Name: Liam Bush
// Student Number: 24227223
// Student Name: Amir Husain
// Student Number: 23380159

#include <mpi.h>
#ifndef MPI_CONV2D_STRIDE
#define MPI_CONV2D_STRIDE

// 2D Convolution with stride function
void mpi_conv2d_stride(
    float *f, int H, int W,        // input feature map and dimensions
    float *g, int kH, int kW,      // kernel and dimensions
    int sH, int sW,                // stride in height and width
    float *output,                 // output array
    MPI_Comm comm                  // communicator
);


#endif