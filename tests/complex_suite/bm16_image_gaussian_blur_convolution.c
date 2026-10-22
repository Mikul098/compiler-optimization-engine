#include <stdio.h>

// Benchmark 16: Image Processing 2D Separable Gaussian Convolution Kernel
int main(int argc, char* argv[]) {
    // 3x3 Gaussian Discrete Kernel Weights
    int k_corner = 1;
    int k_edge = 2;
    int k_center = 4;
    int kernel_weight_sum = 16; // 4*1 + 4*2 + 1*4 = 16

    // Input 3x3 Image Patch Pixels (8-bit grayscale values 0-255)
    int p00 = 100; int p01 = 120; int p02 = 110;
    int p10 = 140; int p11 = 200; int p12 = 150;
    int p20 = 90;  int p21 = 130; int p22 = 105;

    // Dead Edge-Detection Sobel Pre-pass (Dead Code)
    int dead_sobel_gx = (p02 + 2 * p12 + p22) - (p00 + 2 * p10 + p20);
    int dead_sobel_gy = (p20 + 2 * p21 + p22) - (p00 + 2 * p01 + p02);
    int dead_sobel_mag = (dead_sobel_gx * dead_sobel_gx) + (dead_sobel_gy * dead_sobel_gy);

    // Common Pixel Sums across rows (CSE)
    int row0_edges = p00 + p02;
    int row2_edges = p20 + p22;

    // Convolution Weighted Multiplication
    int conv_corners = (row0_edges + row2_edges) * k_corner;
    int conv_edges = (p01 + p10 + p12 + p21) * k_edge;
    int conv_center = p11 * k_center;

    // Filtered Output Pixel
    int filtered_pixel = (conv_corners + conv_edges + conv_center) / kernel_weight_sum;

    // Contrast Adjustment
    int contrast_boost = 10;
    int final_pixel = filtered_pixel + contrast_boost;

    printf("Blurred Convolved Center Pixel: %d\n", final_pixel);
    return 0;
}
