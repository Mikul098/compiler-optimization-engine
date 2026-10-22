#include <stdio.h>

// Benchmark 04: Multilayer Perceptron Deep Neural Forward Inference Pass
int main(int argc, char* argv[]) {
    // Input Feature Vector (3 inputs)
    int x1 = 12;
    int x2 = 25;
    int x3 = -8;

    // Layer 1 Weights (3 inputs x 3 neurons)
    int w11 = 2;  int w12 = 1;  int w13 = 3;
    int w21 = -1; int w22 = 4;  int w23 = 2;
    int w31 = 3;  int w32 = -2; int w33 = 1;

    // Layer 1 Biases
    int b1 = 5;
    int b2 = -10;
    int b3 = 15;

    // Dead Weight Regularization Check
    int dead_l2_norm = (w11 * w11) + (w12 * w12) + (w13 * w13) + (w21 * w21);

    // Common Feature Combinations (CSE opportunities)
    int feature_sum_12 = x1 + x2;
    int feature_prod_13 = x1 * x3;

    // Layer 1 Hidden Activations (Linear Comb)
    int z1 = (x1 * w11) + (x2 * w12) + (x3 * w13) + b1;
    int z2 = (x1 * w21) + (x2 * w22) + (x3 * w23) + b2;
    int z3 = (x1 * w31) + (x2 * w32) + (x3 * w33) + b3;

    // ReLU Activation Function Approximation (Simulated piecewise)
    int h1 = z1;
    int h2 = z2;
    int h3 = z3;

    // Layer 2 Weights (3 hidden x 2 outputs)
    int v11 = 3; int v12 = -2;
    int v21 = 1; int v22 = 4;
    int v31 = 2; int v32 = 1;

    int out_bias1 = 100;
    int out_bias2 = 200;

    // Redundant Intermediate Math
    int dead_hidden_covariance = h1 * h2 * h3;

    // Output Layer Logits
    int out1 = (h1 * v11) + (h2 * v21) + (h3 * v31) + out_bias1;
    int out2 = (h1 * v12) + (h2 * v22) + (h3 * v32) + out_bias2;

    // Softmax-like Probability Disparity
    int logit_diff = out1 - out2;
    int classification_confidence = (out1 * 100) / (out1 + out2);

    printf("Logit Disparity: %d\n", logit_diff);
    printf("Prediction Confidence: %d\n", classification_confidence);
    return 0;
}
