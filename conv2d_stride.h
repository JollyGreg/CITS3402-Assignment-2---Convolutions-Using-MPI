// Student Name: Liam Bush
// Student Number: 24227223
// Student Name: Amir Husain
// Student Number: 23380159

#ifndef CONV2D_STRIDE
#define CONV2D_STRIDE

// 2D Convolution with stride function
void conv2d_stride(
    float *f, int H, int W,        // input feature map and dimensions
    float *g, int kH, int kW,      // kernel and dimensions
    int sH, int sW,                // stride in height and width
    float *output                  // output array
);

#endif