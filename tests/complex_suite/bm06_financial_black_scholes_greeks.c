#include <stdio.h>

// Benchmark 06: Financial Engineering Black-Scholes Option Pricing Greeks
int main(int argc, char* argv[]) {
    // Underlying Asset & Option Contract Inputs (Scaled integers)
    int spot_price = 100;
    int strike_price = 95;
    int time_to_exp = 2;        // Years
    int risk_free_rate = 5;     // 5%
    int volatility = 20;        // 20%

    // Common Term Computations
    int moneyness = spot_price - strike_price;
    int variance = volatility * volatility;
    int rate_plus_half_var = risk_free_rate + (variance / 2);

    // Redundant risk model (dead code)
    int dead_var_stress_test = moneyness * variance * 999;
    int dead_liquidity_discount = dead_var_stress_test / 100;

    // d1 and d2 Approximation
    int d1_numerator = moneyness * 100 + (rate_plus_half_var * time_to_exp);
    int d1_denominator = volatility * time_to_exp;
    int d1 = d1_numerator / d1_denominator;
    int d2 = d1 - d1_denominator;

    // Common Subexpression: Normal CDF polynomial approximation factor
    int poly_factor = 2316419;
    int d1_sq = d1 * d1;
    int d2_sq = d2 * d2;

    // Delta (Sensitivity to Spot Price)
    int delta = (d1 * 100) / 10;

    // Gamma (Second-order price sensitivity)
    int gamma = 1000 / (spot_price * d1_denominator);

    // Vega (Sensitivity to Volatility)
    int vega = spot_price * d1_denominator;

    // Theta (Time Decay)
    int theta = -1 * ((spot_price * volatility) / (2 * time_to_exp));

    // Comprehensive Portfolio Risk Metric
    int portfolio_risk_index = delta + (gamma * 10) + (vega / 10) + theta;
    printf("Option Delta Metric: %d\n", delta);
    printf("Aggregate Portfolio Risk: %d\n", portfolio_risk_index);
    return 0;
}
