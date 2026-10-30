#include <stdio.h>

// Benchmark 03: DSP Digital Audio Biquad IIR Filter 4-Stage Cascade
int main(int argc, char* argv[]) {
    // Filter Coefficients for Stage 1 (Low-Pass Peaking)
    int b0_1 = 2;
    int b1_1 = 4;
    int b2_1 = 2;
    int a1_1 = -1;
    int a2_1 = 1;

    // Filter Coefficients for Stage 2 (High-Pass Notch)
    int b0_2 = 3;
    int b1_2 = -6;
    int b2_2 = 3;
    int a1_2 = 2;
    int a2_2 = -1;

    // Input Audio Sample Stream (Integer Fixed-Point PCM)
    int x0 = 100;
    int x1 = 150;
    int x2 = 80;
    int x3 = 200;

    // Common Subexpression: Pre-calculated feedback combinations
    int alpha = b0_1 + b2_1;
    int beta = b1_1 * 2;
    int gamma = a1_1 + a2_1;

    // Dead filter response metric
    int dead_nyquist_gain = alpha * beta * gamma;

    // Stage 1 Filter Operations
    // Sample 0
    int w0_1 = x0;
    int y0_1 = b0_1 * w0_1;

    // Sample 1
    int w1_1 = x1 - a1_1 * w0_1;
    int y1_1 = b0_1 * w1_1 + b1_1 * w0_1;

    // Sample 2
    int w2_1 = x2 - a1_1 * w1_1 - a2_1 * w0_1;
    int y2_1 = b0_1 * w2_1 + b1_1 * w1_1 + b2_1 * w0_1;

    // Sample 3
    int w3_1 = x3 - a1_1 * w2_1 - a2_1 * w1_1;
    int y3_1 = b0_1 * w3_1 + b1_1 * w2_1 + b2_1 * w1_1;

    // Stage 2 Filter Operations (Cascaded: y_stage1 becomes x_stage2)
    int w0_2 = y0_1;
    int y0_2 = b0_2 * w0_2;

    int w1_2 = y1_1 - a1_2 * w0_2;
    int y1_2 = b0_2 * w1_2 + b1_2 * w0_2;

    int w2_2 = y2_1 - a1_2 * w1_2 - a2_2 * w0_2;
    int y2_2 = b0_2 * w2_2 + b1_2 * w1_2 + b2_2 * w0_2;

    int w3_2 = y3_1 - a1_2 * w2_2 - a2_2 * w1_2;
    int y3_2 = b0_2 * w3_2 + b1_2 * w2_2 + b2_2 * w1_2;

    // Accumulated Output Energy
    int total_output_energy = y0_2 + y1_2 + y2_2 + y3_2;
    printf("Filtered Audio Energy: %d\n", total_output_energy);
    return 0;
}
