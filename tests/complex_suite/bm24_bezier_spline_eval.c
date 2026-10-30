#include <stdio.h>

// Benchmark 24: Computer Graphics Cubic Bezier Spline Bernstein Polynomial & Hodograph
int main(int argc, char* argv[]) {
    // 4 Control Points P0, P1, P2, P3 (X, Y)
    int p0_x = 0;   int p0_y = 0;
    int p1_x = 10;  int p1_y = 50;
    int p2_x = 80;  int p2_y = 60;
    int p3_x = 100; int p3_y = 0;

    // Parameter t = 0.5 (scaled by 100 -> 50)
    int t = 50;
    int u = 100 - t; // (1 - t)

    // Bernstein Basis Polynomials:
    // B0(t) = (1-t)^3
    // B1(t) = 3t(1-t)^2
    // B2(t) = 3t^2(1-t)
    // B3(t) = t^3
    int u_sq = (u * u) / 100;
    int u_cb = (u_sq * u) / 100;

    int t_sq = (t * t) / 100;
    int t_cb = (t_sq * t) / 100;

    int b0 = u_cb;
    int b1 = (3 * t * u_sq) / 100;
    int b2 = (3 * t_sq * u) / 100;
    int b3 = t_cb;

    // Dead Arc-Length Chord approximation
    int dead_chord_len = (p3_x - p0_x) * (p3_y - p0_y) * 444;

    // Evaluated Curve Point C(t)
    int curve_x = (b0 * p0_x + b1 * p1_x + b2 * p2_x + b3 * p3_x) / 100;
    int curve_y = (b0 * p0_y + b1 * p1_y + b2 * p2_y + b3 * p3_y) / 100;

    // First Derivative Velocity Vector (Hodograph: 3*(Pi+1 - Pi))
    int d0_x = 3 * (p1_x - p0_x);
    int d1_x = 3 * (p2_x - p1_x);
    int d2_x = 3 * (p3_x - p2_x);

    int vel_x = (u_sq * d0_x + 2 * u * t * d1_x / 100 + t_sq * d2_x) / 100;

    printf("Bezier Spline Point X: %d\n", curve_x);
    printf("Bezier Spline Point Y: %d\n", curve_y);
    printf("Tangent Velocity X: %d\n", vel_x);
    return 0;
}
