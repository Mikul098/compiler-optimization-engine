export const TEST_SUITES = [
  {
    id: 'test_c_cpp_complete_cheatsheet',
    name: 'C & C++ Complete Cheatsheet: Full Keyword & Syntax Master Suite',
    category: 'Full C/C++ Master Suite',
    badge: 'C23 & C++23 Master',
    reductionTarget: '32.2%',
    expectedOutput: '2205',
    description: 'Exhaustive verification of C23 & C++23 features: hexadecimal, binary & char literals, bitwise and alternative tokens, ternary expressions, control flow, functions, compound assignments, casts, and streams.',
    code: `// Comprehensive verification test for C and C++ Complete Keyword & Syntax Cheat Sheet
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
    printf("Computed: %d\\n", total_result);

    return 0;
}`
  },
  {
    id: 'bm25_master_grand_challenge',
    name: 'Benchmark 25: Master Capstone Grand Challenge (Standard C)',
    category: 'Industrial C Benchmark',
    badge: 'Standard C Pipeline',
    reductionTarget: '98.6%',
    expectedOutput: '5600',
    description: 'A comprehensive multi-phase C benchmark featuring functions, types, heavy constant cascades, commutative CSE, re-assignments, and dead code pruning.',
    code: `#include <stdio.h>

// Benchmark 25: Master Capstone Multi-Domain Optimization Stress Grand Challenge
int main(int argc, char* argv[]) {
    // 1. Heavy Cascading Constant Folding
    int c1 = 10;
    int c2 = 20;
    int c3 = c1 * c2;          // 200
    int c4 = c3 + 100;         // 300
    int c5 = c4 * 2;           // 600
    int c6 = c5 / 3;           // 200
    int c7 = c6 - 50;          // 150
    int c8 = c7 * 4;           // 600

    // 2. Commutative Common Subexpressions
    int a = 15;
    int b = 35;
    int e1 = a + b;            // 50
    int e2 = b + a;            // Commutative match -> e1
    int e3 = a * b;            // 525
    int e4 = b * a;            // Commutative match -> e3
    int sub_combined = e1 + e2 + e3 + e4;

    // 3. Massive Dead Code Eradication
    int dead_var1 = c8 * sub_combined;
    int dead_var2 = dead_var1 + 999;
    int dead_var3 = dead_var2 * 2;
    int dead_var4 = dead_var3 / 5;
    int dead_var5 = dead_var4 - 100;

    // 4. Dynamic Re-assignment Overwrites
    int dynamic_x = 100;
    int dynamic_y = 50;
    int term1 = dynamic_x + dynamic_y; // 150

    dynamic_x = 200; // Invalidation occurs here
    int term2 = dynamic_x + dynamic_y; // 250 (must NOT reuse term1!)

    dynamic_x = 100; // Re-established
    int term3 = dynamic_x + dynamic_y; // 150

    // Compound assignments
    int accumulator = 0;
    accumulator += c8;
    accumulator += sub_combined;
    accumulator += term1;
    accumulator += term2;
    accumulator += term3;

    // 5. High-Order Non-Linear Synthesis
    int scale_factor = 2;
    int bias_offset = 1000;
    int grand_total = (accumulator * scale_factor) + bias_offset;

    // 6. Output Verification
    printf("Grand Total Checkpoint: %d\\n", grand_total);
    return 0;
}`
  },
  {
    id: 'bm01_3d_graphics_matrix_pipeline',
    name: 'Benchmark 01: 3D Graphics Affine Matrix Pipeline (Standard C)',
    category: 'Industrial C Benchmark',
    badge: '3D Matrix Shaders',
    reductionTarget: '99.1%',
    expectedOutput: '449406',
    description: '3D Graphics 4x4 Affine Matrix Transform, Perspective Projection, Vertex Shader pipeline with common subexpression elimination.',
    code: `#include <stdio.h>

int main(int argc, char* argv[]) {
    // Model-View Matrix Elements
    int m00 = 2; int m01 = 0; int m02 = 1; int m03 = 10;
    int m10 = 0; int m11 = 3; int m12 = 0; int m13 = 20;
    int m20 = 1; int m21 = 0; int m22 = 2; int m23 = -5;

    // Projection Scaling
    int fov_scale = 4;
    int aspect_ratio = 2;
    int near_plane = 10;
    int far_plane = 100;

    // Vertices
    int v1_x = 5; int v1_y = 8; int v1_z = 12;
    int v2_x = 10; int v2_y = 15; int v2_z = 24;

    // Transform
    int v1_world_x = m00 * v1_x + m01 * v1_y + m02 * v1_z + m03;
    int v1_world_y = m10 * v1_x + m11 * v1_y + m12 * v1_z + m13;
    int v1_world_z = m20 * v1_x + m21 * v1_y + m22 * v1_z + m23;

    int depth_delta = far_plane - near_plane;
    int depth_sum = far_plane + near_plane;
    int proj_factor = fov_scale * aspect_ratio;

    int v1_proj_x = v1_world_x * proj_factor;
    int v1_proj_y = v1_world_y * proj_factor;
    int v1_proj_z = (v1_world_z * depth_sum) / depth_delta;

    int v2_world_x = m00 * v2_x + m01 * v2_y + m02 * v2_z + m03;
    int v2_world_y = m10 * v2_x + m11 * v2_y + m12 * v2_z + m13;
    int v2_world_z = m20 * v2_x + m21 * v2_y + m22 * v2_z + m23;

    int v2_proj_x = v2_world_x * proj_factor;
    int v2_proj_y = v2_world_y * proj_factor;
    int v2_proj_z = (v2_world_z * depth_sum) / depth_delta;

    int center_x = (v1_proj_x + v2_proj_x) / 2;
    int center_y = (v1_proj_y + v2_proj_y) / 2;
    int center_z = (v1_proj_z + v2_proj_z) / 2;

    center_x += 100;
    center_y += 100;

    int screen_hash = center_x * 1000 + center_y * 10 + center_z;
    printf("3D Viewport Screen Hash: %d\\n", screen_hash);
    return 0;
}`
  },
  {
    id: 'test11_final_boss',
    name: 'Test 11: Final Boss Evaluation Suite',
    category: 'Full Pipeline Capstone',
    badge: 'Dual Novelty Capstone',
    reductionTarget: '93.3%',
    expectedOutput: '350',
    description: 'The master benchmark combining constant folding, commutative subexpression elimination, dead variable eradication, and VM semantic verification.',
    code: `// 1. Math that should be Constant Folded
base = 10;
multiplier = 5;
fixed_val = base * multiplier; 

// 2. Common Subexpressions
a = 100;
b = 200;
c = a + b;
d = a + b;     // CSE should catch this

// 3. Dead Code (Calculated but never printed)
ghost_var = fixed_val * c;

// 4. Final complex math combining everything
final_answer = fixed_val + d;

// 5. Output
print final_answer;`
  },
  {
    id: 'test10_master',
    name: 'Test 10: Master Optimization Suite',
    category: 'Simultaneous Passes',
    badge: 'Multi-Pass Synergy',
    reductionTarget: '91.7%',
    expectedOutput: '300',
    description: 'Triggers constant folding, common subexpressions, and dead code elimination simultaneously on a geometric area calculation.',
    code: `// Master test to trigger all three optimizations simultaneously
pi = 3;
radius = 10;
r_squared = radius * radius;
area1 = pi * r_squared;

// Redundant math and dead code
area2 = pi * (radius * radius); 
dead_var = area1 + area2;

print area1;`
  },
  {
    id: 'test9_cascade',
    name: 'Test 9: Cascading Fixed-Point Multi-Loop',
    category: 'Fixed-Point Iteration',
    badge: '3-Pass Convergence',
    reductionTarget: '88.9%',
    expectedOutput: '500',
    description: 'Demonstrates cascading dependencies where each optimization pass unblocks the next across 3 convergence cycles.',
    code: `// This requires at least two full loops to completely optimize.
a = 10;
b = 20;
c = a * b;      // Folds to 200 on Loop 1
d = c + 50;     // 'c' propagates to 200, folds to 250 on Loop 2
e = d * 2;      // 'd' propagates to 250, folds to 500 on Loop 3
print e;`
  },
  {
    id: 'test8_cse',
    name: 'Test 8: CSE & Commutative Signature Matching',
    category: 'Optimization Pass',
    badge: 'Commutative CSE',
    reductionTarget: '33.3%',
    expectedOutput: '0',
    description: 'Demonstrates signature canonicalization for commutative multiplication (b * c == c * b).',
    code: `// Testing basic CSE and commutativity
x = b * c;
y = a + x;
z = c * b;      // Should be recognized as identical to 'x'
final = a + z;  // Should be recognized as identical to 'y'
print final;`
  },
  {
    id: 'test7_dce',
    name: 'Test 7: Dead Code Elimination (DCE)',
    category: 'Optimization Pass',
    badge: 'Liveness Pruning',
    reductionTarget: '85.7%',
    expectedOutput: '150',
    description: 'Performs backward liveness analysis to eliminate calculations whose results are never printed or referenced.',
    code: `// Code with orphaned variables and math that gets folded
x = 100;        // Used later
y = 200;        // Never used (Dead Code)
z = 5 * 10;     // Folds to 50, but never printed (Dead Code)
final = x + 50; 
print final;`
  },
  {
    id: 'test6_const_fold',
    name: 'Test 6: Constant Propagation & Folding',
    category: 'Optimization Pass',
    badge: 'Compile-Time Math',
    reductionTarget: '85.7%',
    expectedOutput: '250',
    description: 'Propagates literal values and pre-computes arithmetic expressions at compile-time.',
    code: `// The compiler should resolve all of this math without executing it.
a = 10;
b = 20;
c = a * b;      // Should fold to 200
d = c + 50;     // Should fold to 250
print d;`
  },
  {
    id: 'test5_subexpressions',
    name: 'Test 5: Subexpression Tree Lowering',
    category: 'IR Lowering',
    badge: 'Redundancy Baseline',
    reductionTarget: '0%',
    expectedOutput: '0',
    description: 'Evaluates baseline TAC generation with duplicate subtrees before pass optimizations.',
    code: `a = b * c;
d = a + b * c;
print(d);`
  },
  {
    id: 'test4_complex_tac',
    name: 'Test 4: Deeply Nested TAC Flattening',
    category: 'IR Lowering',
    badge: 'Quad Lowering',
    reductionTarget: '88.9%',
    expectedOutput: '1000',
    description: 'Flattens compound formula p * (1 + (r / 100) * t) into a 4-temporary linear instruction sequence.',
    code: `// Calculate compound interest variables (mock formula)
p = 1000;
r = 5;
t = 10;
temp_calc = p * (1 + (r / 100) * t);
print(temp_calc);`
  },
  {
    id: 'test2_precedence',
    name: 'Test 2: Operator Precedence & PEMDAS',
    category: 'Parsing & AST',
    badge: 'Grammar Tree',
    reductionTarget: '0%',
    expectedOutput: '14, 20',
    description: 'Validates that multiplication has higher precedence than addition and grouping parentheses override precedence.',
    code: `a = 2 + 3 * 4;
b = (2 + 3) * 4;
print(a);
print(b);`
  },
  {
    id: 'test1_basic',
    name: 'Test 1: Basic Sanity & Print',
    category: 'Parsing & AST',
    badge: 'Baseline Sanity',
    reductionTarget: '0%',
    expectedOutput: '10, 30',
    description: 'Basic sanity test confirming symbol tokenization, assignment statements, and stdout printing.',
    code: `a = 10;
b = 20;
print(a);
c = a + b;
print(c);`
  },
  {
    id: 'test3_syntax_error',
    name: 'Test 3: Syntax Diagnostic Detection',
    category: 'Negative Test',
    badge: 'Graceful Error',
    reductionTarget: 'N/A',
    expectedOutput: 'Parse failed!',
    description: 'Verifies parser syntax diagnostics, reporting line numbers accurately without compiler abort.',
    code: `a = 2 + * 3;
print(a)`
  }
];
