#include <stdio.h>

int main() {
    // 1. Initial State & Configuration
    int base = 10;
    int multiplier = 3;
    int mode = 2;
    int status = 0;

    // 2. Nested IF - ELSE IF - ELSE Branching
    int branch_res = 0;
    if (base > 50) {
        branch_res = 100;
    } else if (base > 5) {
        if (multiplier == 3) {
            branch_res = base * multiplier; // 10 * 3 = 30
        } else {
            branch_res = base + multiplier;
        }
    } else {
        branch_res = -1;
    }

    // 3. FOR Loop with Accumulation and Step
    int for_sum = 0;
    for (int i = 1; i <= 5; i++) {
        for_sum += i * i; // 1 + 4 + 9 + 16 + 25 = 55
    }

    // 4. WHILE Loop with Conditional Logic & Decrement
    int while_acc = 0;
    int countdown = 6;
    while (countdown > 0) {
        if (countdown % 2 == 0) {
            while_acc += countdown; // adds 6, 4, 2 -> 12
        } else {
            while_acc += 1;         // adds 1 for 5, 3, 1 -> 3
        }
        countdown--;
    }

    // 5. DO - WHILE Loop
    int do_val = 1;
    int do_iterations = 0;
    do {
        do_val *= 2;        // 1 -> 2 -> 4 -> 8 -> 16
        do_iterations++;
    } while (do_iterations < 4);

    // 6. SWITCH - CASE with Break and Default
    int switch_bonus = 0;
    switch (mode) {
        case 1:
            switch_bonus = 10;
            break;
        case 2:
            switch_bonus = 50;
            break;
        case 3:
            switch_bonus = 100;
            break;
        default:
            switch_bonus = 0;
            break;
    }

    // 7. Ternary Conditional Operator
    int threshold = 25;
    int ternary_val = (branch_res >= threshold) ? 500 : 100; // 30 >= 25 -> 500

    // 8. Total Verification Aggregation
    int grand_total = branch_res + for_sum + while_acc + do_val + switch_bonus + ternary_val;

    // 9. Outputs
    printf(branch_res);
    printf(for_sum);
    printf(while_acc);
    printf(do_val);
    printf(switch_bonus);
    printf(ternary_val);
    printf(grand_total);

    return 0;
}
