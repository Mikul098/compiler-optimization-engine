#include <stdio.h>

// Benchmark 08: Statistical Machine Learning Polynomial Regression Normal Equations
int main(int argc, char* argv[]) {
    // 4 Sample Data Points (X, Y)
    int x1 = 1; int y1 = 3;
    int x2 = 2; int y2 = 7;
    int x3 = 3; int y3 = 13;
    int x4 = 4; int y4 = 21;

    // Powers of X
    int x1_2 = x1 * x1; int x1_3 = x1_2 * x1; int x1_4 = x1_2 * x1_2;
    int x2_2 = x2 * x2; int x2_3 = x2_2 * x2; int x2_4 = x2_2 * x2_2;
    int x3_2 = x3 * x3; int x3_3 = x3_2 * x3; int x3_4 = x3_2 * x3_2;
    int x4_2 = x4 * x4; int x4_3 = x4_2 * x4; int x4_4 = x4_2 * x4_2;

    // Sum Terms for Design Matrix X^T * X
    int sum_x = x1 + x2 + x3 + x4;
    int sum_x2 = x1_2 + x2_2 + x3_2 + x4_2;
    int sum_x3 = x1_3 + x2_3 + x3_3 + x4_3;
    int sum_x4 = x1_4 + x2_4 + x3_4 + x4_4;

    // Cross-term Sums for X^T * Y
    int sum_y = y1 + y2 + y3 + y4;
    int sum_xy = (x1 * y1) + (x2 * y2) + (x3 * y3) + (x4 * y4);
    int sum_x2y = (x1_2 * y1) + (x2_2 * y2) + (x3_2 * y3) + (x4_2 * y4);

    // Dead Outlier Leverage metric
    int dead_leverage = sum_x4 * sum_y * 123;

    // Determinant of 2x2 Linear Sub-system
    int n = 4;
    int det = (n * sum_x2) - (sum_x * sum_x);

    // Solved Coefficients (Slope a1, Intercept a0)
    int a1_numerator = (n * sum_xy) - (sum_x * sum_y);
    int a1 = a1_numerator / det;

    int a0_numerator = (sum_y * sum_x2) - (sum_x * sum_xy);
    int a0 = a0_numerator / det;

    // Predicted values at test query x_test = 5
    int x_test = 5;
    int y_pred = a0 + (a1 * x_test);

    // Residual Sum of Squares (RSS)
    int res1 = y1 - (a0 + a1 * x1);
    int res2 = y2 - (a0 + a1 * x2);
    int total_rss = (res1 * res1) + (res2 * res2);

    printf("Fitted Slope a1: %d\n", a1);
    printf("Fitted Intercept a0: %d\n", a0);
    printf("Prediction at X=5: %d\n", y_pred);
    return 0;
}
