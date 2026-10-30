#include <stdio.h>

// Benchmark 19: Astrodynamics Keplerian Orbital Elements to Cartesian State Vector
int main(int argc, char* argv[]) {
    // Semi-major axis a (km) and Eccentricity e (scaled by 100)
    int a = 7000;
    int e = 10; // 0.10
    int mu = 398600; // Earth Gravitational Parameter

    // True Anomaly theta (cos*100 = 80, sin*100 = 60)
    int cos_theta = 80;
    int sin_theta = 60;

    // Orbital Radius r = a * (1 - e^2) / (1 + e*cos_theta)
    int one_minus_e2 = 10000 - (e * e);
    int denom = 10000 + (e * cos_theta);
    int r = (a * one_minus_e2) / denom;

    // Dead Lagrange Point L1 candidate
    int dead_hill_sphere = a * e * 777;

    // Cartesian Coordinates in Orbital Plane
    int pos_x = (r * cos_theta) / 100;
    int pos_y = (r * sin_theta) / 100;

    // Vis-Viva Equation: v^2 = mu * (2/r - 1/a)
    int two_over_r = (2 * 10000) / r;
    int one_over_a = (1 * 10000) / a;
    int v_sq = (mu * (two_over_r - one_over_a)) / 10000;

    printf("Orbital Radius r: %d\n", r);
    printf("Cartesian Pos X: %d\n", pos_x);
    printf("Cartesian Pos Y: %d\n", pos_y);
    printf("Orbital Velocity Squared: %d\n", v_sq);
    return 0;
}
