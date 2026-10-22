// Comprehensive verification test for C and C++ Complete Keyword & Syntax Cheat Sheet
#include <iostream>
#include <cstdio>
#include <vector>
#include <cmath>

int main(void) {
    // 1. Literals (Hex, Binary, Octal, Char, Float, Boolean, Nullptr)
    int hex_val = 0xFF;         // 255
    int bin_val = 0b1010;       // 10
    int char_val = 'A';         // 65
    int float_trunc = 3.14;     // 3
    bool flag = true;           // 1
    int* null_ptr = nullptr;    // 0

    // 2. Predefined Macros & Limits
    int max_int = INT_MAX;      // 2147483647
    int char_bits = CHAR_BIT;   // 8
    int eof_val = EOF;          // -1

    // 3. Bitwise Operators & Alternative Tokens
    int b1 = 0b1100 bitand 0b1010;  // 0b1000 = 8
    int b2 = 0b1100 bitor 0b0011;   // 0b1111 = 15
    int b3 = 0b1100 xor 0b1010;     // 0b0110 = 6
    int b4 = 1 << 4;                // 16
    int b5 = 32 >> 2;               // 8

    // Alternative logical tokens
    int log_val = (10 > 5) and not (2 > 10) or (3 == 4); // 1

    // 4. Compound Assignments (standard and alternative tokens)
    int acc = 100;
    acc += 50;                  // 150
    acc -= 25;                  // 125
    acc *= 2;                   // 250
    acc /= 5;                   // 50
    acc %= 30;                  // 20
    acc and_eq 0xFF;            // 20
    acc or_eq 0x01;             // 21
    acc xor_eq 0x05;            // 16
    acc <<= 2;                  // 64
    acc >>= 1;                  // 32

    // 5. Ternary Operator (condition ? val_true : val_false)
    int ternary_res = (acc > 30) ? 999 : -999; // 999

    // 6. Built-in Math Functions (abs, min, max, std::abs)
    int abs_val = abs(-50);                 // 50
    int min_val = min(100, 200);            // 100
    int max_val = max(100, 200);            // 200

    // 7. Member and Pointer syntax simulation
    int p_x = 10;
    int p_y = 20;
    int point_sum = p_x + p_y;              // 30

    // 8. Casts (static_cast and C-style cast)
    int cast1 = static_cast<int>(50);       // 50
    int cast2 = (int)100;                   // 100

    // 9. Sizeof operator
    int sz_int = sizeof(int);               // 4
    int sz_val = sizeof(acc);               // 4

    // 10. Control flow: do-while loop
    int counter = 0;
    int loop_sum = 0;
    do {
        loop_sum += counter;
        counter++;
    } while (counter <= 5);                 // loop_sum = 15

    // 11. Control flow: switch-case with break and default
    int switch_val = 2;
    int switch_out = 0;
    switch (switch_val) {
        case 1:
            switch_out = 100;
            break;
        case 2:
            switch_out = 200;
            break;
        default:
            switch_out = -1;
            break;
    }

    // 12. Control flow: Goto and Label
    int goto_res = 1;
    goto jump_target;
    goto_res = 9999; // dead code
jump_target:
    goto_res = 42;

    // 13. Static Assert, Exception Handling & Coroutine syntax
    static_assert(1, "valid");
    try {
        int x_try = 10;
    } catch (...) {
        int x_catch = 0;
    }

    // 14. Modern keywords & type specifiers (constexpr, auto, const, etc.)
    constexpr auto const_factor = 2;
    const unsigned long long int big_val = 500;

    // 15. Final Aggregation and Verification Output
    int total_result = hex_val + bin_val + char_val + b1 + b2 + b3 + b4 + b5 +
                       acc + ternary_res + abs_val + min_val + max_val +
                       point_sum + cast1 + cast2 + sz_int + loop_sum +
                       switch_out + goto_res;

    // 16. Chained Stream Output & Printf Output
    std::cout << "Total Result: " << total_result << std::endl;
    printf("Computed: %d\n", total_result);

    return 0;
}
