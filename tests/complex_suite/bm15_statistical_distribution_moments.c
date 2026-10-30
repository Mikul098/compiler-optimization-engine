#include <stdio.h>

// Benchmark 15: Statistical Analysis Central Moments, Variance, Skewness & Kurtosis
int main(int argc, char* argv[]) {
    // Sample Data Array (5 observations)
    int d1 = 12;
    int d2 = 24;
    int d3 = 18;
    int d4 = 30;
    int d5 = 16;

    // First Raw Moment: Arithmetic Mean
    int n = 5;
    int sum = d1 + d2 + d3 + d4 + d5;
    int mean = sum / n;

    // Deviations from Mean (CSE Candidates)
    int dev1 = d1 - mean;
    int dev2 = d2 - mean;
    int dev3 = d3 - mean;
    int dev4 = d4 - mean;
    int dev5 = d5 - mean;

    // Dead Geometric Mean approximation
    int dead_geom_prod = d1 * d2 * d3 * d4 * d5;

    // Second Central Moment: Variance (dev^2)
    int dev1_sq = dev1 * dev1;
    int dev2_sq = dev2 * dev2;
    int dev3_sq = dev3 * dev3;
    int dev4_sq = dev4 * dev4;
    int dev5_sq = dev5 * dev5;
    int var_sum = dev1_sq + dev2_sq + dev3_sq + dev4_sq + dev5_sq;
    int variance = var_sum / n;

    // Third Central Moment: Skewness (dev^3)
    int dev1_cb = dev1_sq * dev1;
    int dev2_cb = dev2_sq * dev2;
    int dev3_cb = dev3_sq * dev3;
    int dev4_cb = dev4_sq * dev4;
    int dev5_cb = dev5_sq * dev5;
    int skew_sum = dev1_cb + dev2_cb + dev3_cb + dev4_cb + dev5_cb;
    int m3 = skew_sum / n;

    // Fourth Central Moment: Kurtosis (dev^4)
    int dev1_quad = dev1_sq * dev1_sq;
    int dev2_quad = dev2_sq * dev2_sq;
    int dev3_quad = dev3_sq * dev3_sq;
    int dev4_quad = dev4_sq * dev4_sq;
    int dev5_quad = dev5_sq * dev5_sq;
    int kurt_sum = dev1_quad + dev2_quad + dev3_quad + dev4_quad + dev5_quad;
    int m4 = kurt_sum / n;

    printf("Calculated Mean: %d\n", mean);
    printf("Sample Variance: %d\n", variance);
    printf("Skewness Moment: %d\n", m3);
    printf("Kurtosis Moment: %d\n", m4);
    return 0;
}
