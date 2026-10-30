#include <stdio.h>

// Benchmark 14: Numerical Mathematics 4th-Order Runge-Kutta Coupled Oscillator ODE
int main(int argc, char* argv[]) {
    // Oscillator Parameters: x'' + 2*zeta*omega*x' + omega^2*x = 0
    int omega_sq = 16;  // Natural frequency squared
    int damping = 2;    // Damping factor
    int dt = 1;         // Time step (scaled by 10)

    // Initial Conditions: x(0) = 100, v(0) = 0
    int x = 100;
    int v = 0;

    // Dead Energy Drift constant
    int dead_hamiltonian = (v * v) + (omega_sq * x * x);

    // Derivative function: dx/dt = v, dv/dt = -omega_sq*x - damping*v
    // Stage k1
    int k1_x = v;
    int k1_v = -1 * (omega_sq * x + damping * v) / 10;

    // Stage k2 (Half step)
    int x_mid1 = x + (k1_x * dt) / 20;
    int v_mid1 = v + (k1_v * dt) / 20;
    int k2_x = v_mid1;
    int k2_v = -1 * (omega_sq * x_mid1 + damping * v_mid1) / 10;

    // Stage k3 (Half step)
    int x_mid2 = x + (k2_x * dt) / 20;
    int v_mid2 = v + (k2_v * dt) / 20;
    int k3_x = v_mid2;
    int k3_v = -1 * (omega_sq * x_mid2 + damping * v_mid2) / 10;

    // Stage k4 (Full step)
    int x_end = x + (k3_x * dt) / 10;
    int v_end = v + (k3_v * dt) / 10;
    int k4_x = v_end;
    int k4_v = -1 * (omega_sq * x_end + damping * v_end) / 10;

    // Weighted RK4 Integration Update
    int delta_x = (k1_x + 2 * k2_x + 2 * k3_x + k4_x) / 6;
    int delta_v = (k1_v + 2 * k2_v + 2 * k3_v + k4_v) / 6;

    x += delta_x;
    v += delta_v;

    printf("Oscillator Position x: %d\n", x);
    printf("Oscillator Velocity v: %d\n", v);
    return 0;
}
