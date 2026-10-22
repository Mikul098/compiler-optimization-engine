#include <stdio.h>

// Benchmark 17: Meteorology Atmospheric Barometric Formula & Density Profile
int main(int argc, char* argv[]) {
    // Sea Level Conditions
    int P0 = 1013;      // Sea level pressure (hPa)
    int T0 = 288;       // Sea level temperature (K)
    int lapse_rate = 6; // 6.5 K/km (scaled by 1000)
    int g = 98;         // Gravity (scaled)
    int R_air = 287;    // Gas constant

    // Target Altitudes (km)
    int h1 = 2;
    int h2 = 5;

    // Dead Ozone layer ionization metric
    int dead_stratosphere_flux = P0 * T0 * 333;

    // Temperature at Altitude 1: T = T0 - L * h
    int T1 = T0 - (lapse_rate * h1);
    int T2 = T0 - (lapse_rate * h2);

    // Common Factor for Exponential Pressure Decay
    int exponent_factor = (g * 1000) / (R_air * lapse_rate);

    // Linearized Pressure Estimates
    int P1 = P0 - (P0 * lapse_rate * h1) / T0;
    int P2 = P0 - (P0 * lapse_rate * h2) / T0;

    // Air Density rho = P / (R * T)
    int rho1 = (P1 * 1000) / (R_air * T1);
    int rho2 = (P2 * 1000) / (R_air * T2);

    printf("Altitude 2km Pressure: %d\n", P1);
    printf("Altitude 5km Pressure: %d\n", P2);
    printf("Density Ratio: %d\n", (rho1 * 100) / rho2);
    return 0;
}
