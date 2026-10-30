#include <stdio.h>

// Benchmark 23: Autonomous Systems 1D Linear Kalman Filter State Estimator
int main(int argc, char* argv[]) {
    // True state, Initial Estimate, Covariance
    int x_hat = 0;       // State estimate (position)
    int p_cov = 1000;    // Error covariance
    int q_noise = 20;    // Process noise
    int r_noise = 50;    // Measurement noise

    // System Transition Model: x_k = x_{k-1} + u
    int u_control = 10;  // Control velocity input

    // Measurements from Sensor over 3 steps
    int z1 = 12;
    int z2 = 23;
    int z3 = 31;

    // Dead Sensor Drift calibration
    int dead_allan_variance = q_noise * r_noise * 1234;

    // --- Time Step 1 ---
    // 1. Predict
    int x_pred1 = x_hat + u_control;
    int p_pred1 = p_cov + q_noise;

    // 2. Kalman Gain K = P / (P + R)
    int k_gain1 = (p_pred1 * 100) / (p_pred1 + r_noise);

    // 3. Update
    int innov1 = z1 - x_pred1;
    x_hat = x_pred1 + (k_gain1 * innov1) / 100;
    p_cov = ((100 - k_gain1) * p_pred1) / 100;

    // --- Time Step 2 ---
    // 1. Predict
    int x_pred2 = x_hat + u_control;
    int p_pred2 = p_cov + q_noise;

    // 2. Kalman Gain
    int k_gain2 = (p_pred2 * 100) / (p_pred2 + r_noise);

    // 3. Update
    int innov2 = z2 - x_pred2;
    x_hat = x_pred2 + (k_gain2 * innov2) / 100;
    p_cov = ((100 - k_gain2) * p_pred2) / 100;

    // --- Time Step 3 ---
    int x_pred3 = x_hat + u_control;
    int p_pred3 = p_cov + q_noise;
    int k_gain3 = (p_pred3 * 100) / (p_pred3 + r_noise);
    int innov3 = z3 - x_pred3;
    x_hat = x_pred3 + (k_gain3 * innov3) / 100;
    p_cov = ((100 - k_gain3) * p_pred3) / 100;

    printf("Kalman Filter Estimated State: %d\n", x_hat);
    printf("Estimated State Covariance: %d\n", p_cov);
    return 0;
}
