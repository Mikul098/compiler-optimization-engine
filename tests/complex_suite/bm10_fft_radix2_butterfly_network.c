#include <stdio.h>

// Benchmark 10: Digital Signal Processing 4-Point Radix-2 FFT Butterfly Network
int main(int argc, char* argv[]) {
    // 4 Complex Input Samples (Real & Imaginary)
    int x0_re = 10; int x0_im = 0;
    int x1_re = 20; int x1_im = 5;
    int x2_re = -15; int x2_im = 10;
    int x3_re = 5;  int x3_im = -5;

    // Twiddle Factors W_4^0 = 1, W_4^1 = -j
    // Stage 1 Butterfly Pairs: (0, 2) and (1, 3)
    int s1_0_re = x0_re + x2_re;
    int s1_0_im = x0_im + x2_im;

    int s1_1_re = x0_re - x2_re;
    int s1_1_im = x0_im - x2_im;

    int s1_2_re = x1_re + x3_re;
    int s1_2_im = x1_im + x3_im;

    int s1_3_re = x1_re - x3_re;
    int s1_3_im = x1_im - x3_im;

    // Common Twiddle Multiplication Subexpression
    // Multiplication by W_4^1 = -j -> (re + j im)*(-j) = im - j re
    int rot_3_re = s1_3_im;
    int rot_3_im = -1 * s1_3_re;

    // Dead frequency bucket metric
    int dead_spectrum_leakage = (s1_0_re * s1_2_re) + (s1_1_re * s1_3_re);

    // Stage 2 Butterfly Outputs (Final FFT Bins X0, X1, X2, X3)
    int X0_re = s1_0_re + s1_2_re;
    int X0_im = s1_0_im + s1_2_im;

    int X1_re = s1_1_re + rot_3_re;
    int X1_im = s1_1_im + rot_3_im;

    int X2_re = s1_0_re - s1_2_re;
    int X2_im = s1_0_im - s1_2_im;

    int X3_re = s1_1_re - rot_3_re;
    int X3_im = s1_1_im - rot_3_im;

    // Spectral Energy Magnitude
    int mag0 = (X0_re * X0_re) + (X0_im * X0_im);
    int mag1 = (X1_re * X1_re) + (X1_im * X1_im);
    int total_spectral_power = mag0 + mag1;

    printf("Bin 0 Real: %d\n", X0_re);
    printf("Total Spectral Power: %d\n", total_spectral_power);
    return 0;
}
