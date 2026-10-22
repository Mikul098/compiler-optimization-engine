#include <stdio.h>

// Benchmark 20: Quantitative Finance High-Frequency Trading Order-Book Liquidity
int main(int argc, char* argv[]) {
    // Top 3 Bid/Ask Levels (Price in cents, Volume in shares)
    int bid_p1 = 10050; int bid_v1 = 500;
    int bid_p2 = 10045; int bid_v2 = 800;
    int bid_p3 = 10040; int bid_v3 = 1200;

    int ask_p1 = 10055; int ask_v1 = 400;
    int ask_p2 = 10060; int ask_v2 = 900;
    int ask_p3 = 10065; int ask_v3 = 1500;

    // Top Level Spread and Mid-Price
    int spread = ask_p1 - bid_p1;
    int mid_price = (ask_p1 + bid_p1) / 2;

    // Total Book Depth
    int total_bid_vol = bid_v1 + bid_v2 + bid_v3;
    int total_ask_vol = ask_v1 + ask_v2 + ask_v3;

    // Dead Market-Maker inventory penalty
    int dead_adverse_selection = (total_bid_vol * spread * 888) / 100;

    // Volume-Weighted Micro-Price: (bid_p1 * ask_v1 + ask_p1 * bid_v1) / (bid_v1 + ask_v1)
    int top_vol_sum = bid_v1 + ask_v1;
    int micro_price = (bid_p1 * ask_v1 + ask_p1 * bid_v1) / top_vol_sum;

    // Order Flow Imbalance
    int order_imbalance = ((total_bid_vol - total_ask_vol) * 100) / (total_bid_vol + total_ask_vol);

    printf("Top-of-Book Spread: %d\n", spread);
    printf("Micro-Price: %d\n", micro_price);
    printf("Order Flow Imbalance: %d\n", order_imbalance);
    return 0;
}
