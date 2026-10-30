import {
  parseToyProgram,
  buildASTGraph,
  generateTAC,
  runPassManager,
  executeTAC
} from '../frontend/src/services/compilerEngine.js';

console.log('=====================================================');
console.log('   FULL ENGINE & MATHEMATICAL CORRECTNESS TEST SUITE ');
console.log('=====================================================\n');

let totalTests = 0;
let passedTests = 0;
let failedTests = 0;

function assertTest(name, code, expectedOutput) {
  totalTests++;
  try {
    const { statements, errors } = parseToyProgram(code);
    if (errors.length > 0) {
      throw new Error(`Parse Errors: ${errors.join('; ')}`);
    }

    const unoptTAC = generateTAC(statements);
    const unoptRun = executeTAC(unoptTAC);

    const { finalInstructions } = runPassManager(unoptTAC);
    const optRun = executeTAC(finalInstructions);

    const expected = Array.isArray(expectedOutput) ? expectedOutput : [expectedOutput];

    // Check 1: Unoptimized matches expected
    const unoptMatch = JSON.stringify(unoptRun.output) === JSON.stringify(expected);
    // Check 2: Optimized matches expected
    const optMatch = JSON.stringify(optRun.output) === JSON.stringify(expected);
    // Check 3: Semantic preservation (opt matches unopt)
    const semanticMatch = JSON.stringify(optRun.output) === JSON.stringify(unoptRun.output);

    if (unoptMatch && optMatch && semanticMatch) {
      console.log(`[PASS] ${name} -> Output: ${JSON.stringify(optRun.output)} (Reduced ${unoptTAC.length} -> ${finalInstructions.length} instrs)`);
      passedTests++;
    } else {
      console.error(`[FAIL] ${name}`);
      console.error(`       Expected:  ${JSON.stringify(expected)}`);
      console.error(`       Unopt Run: ${JSON.stringify(unoptRun.output)}`);
      console.error(`       Opt Run:   ${JSON.stringify(optRun.output)}`);
      failedTests++;
    }
  } catch (err) {
    console.error(`[ERROR] ${name}: ${err.message}`);
    failedTests++;
  }
}

// 1. Basic Arithmetic
assertTest('Addition & Subtraction', `
  int a = 100 + 45 - 25;
  printf(a);
`, 120);

assertTest('Multiplication & Division', `
  int a = 12 * 8 / 4;
  printf(a);
`, 24);

assertTest('Modulo Arithmetic', `
  int a = 47 % 10;
  printf(a);
`, 7);

// 2. Negative numbers & sign handling
assertTest('Negative numbers arithmetic', `
  int a = -10;
  int b = 25;
  int c = a + b;
  int d = a * 3;
  int e = -20 / 4;
  printf(c);
  printf(d);
  printf(e);
`, [15, -30, -5]);

assertTest('Signed negative division & modulo', `
  int x = -15;
  int y = x / 2;
  int z = x % 2;
  printf(y);
  printf(z);
`, [-7, -1]);

// 3. Hexadecimal and Binary Literals
assertTest('Hexadecimal & Binary Literals', `
  int hex = 0xFF;
  int bin = 0b1010;
  int sum = hex + bin;
  printf(sum);
`, 265);

// 4. Bitwise Operators
assertTest('Bitwise AND, OR, XOR', `
  int a = 0xF0 & 0x3C;
  int b = 0xF0 | 0x0F;
  int c = 0xAA ^ 0x55;
  printf(a);
  printf(b);
  printf(c);
`, [0x30, 0xFF, 0xFF]);

assertTest('Bitwise Left & Right Shift', `
  int a = 1 << 5;
  int b = 64 >> 2;
  printf(a);
  printf(b);
`, [32, 16]);

assertTest('Bitwise NOT (~)', `
  int a = ~0;
  int b = ~10;
  printf(a);
  printf(b);
`, [-1, -11]);

// 5. Relational & Equality Comparisons
assertTest('Relational Comparisons', `
  int c1 = 10 < 20;
  int c2 = 30 <= 30;
  int c3 = 50 > 100;
  int c4 = 40 >= 50;
  printf(c1);
  printf(c2);
  printf(c3);
  printf(c4);
`, [1, 1, 0, 0]);

assertTest('Equality & Inequality', `
  int eq = (42 == 42);
  int ne = (42 != 42);
  printf(eq);
  printf(ne);
`, [1, 0]);

// 6. Logical AND & OR
assertTest('Logical AND & OR (&&, ||)', `
  int a = (10 > 5) && (20 < 30);
  int b = (10 < 5) || (20 < 30);
  int c = (10 < 5) && (20 < 30);
  printf(a);
  printf(b);
  printf(c);
`, [1, 1, 0]);

// 7. Compound Assignments
assertTest('Compound Assignments (+=, -=, *=, /=, %=, &=, |=, ^=)', `
  int acc = 100;
  acc += 50;
  acc -= 20;
  acc *= 2;
  acc /= 4;
  acc %= 30;
  acc &= 0xFF;
  acc |= 0x01;
  acc ^= 0x03;
  printf(acc);
`, 6);

// 8. Comma-separated declarations & multiple assignments
assertTest('Multiple comma declarations', `
  int x = 10, y = 20, z = 30;
  int total = x + y + z;
  printf(total);
`, 60);

// 9. Complex Operator Precedence (PEMDAS + C precedence)
assertTest('Complex Operator Precedence', `
  int a = 2 + 3 * 4;
  int b = (2 + 3) * 4;
  int c = 100 - 20 / 2 + 5;
  int d = 1 << 2 + 1; // addition binds tighter than shift in C: 1 << 3 = 8
  int e = 5 + 3 > 4 && 2 * 3 == 6;
  printf(a);
  printf(b);
  printf(c);
  printf(d);
  printf(e);
`, [14, 20, 95, 8, 1]);

// 10. Algebraic Simplification identities
assertTest('Algebraic Simplification Identities', `
  int x = 42;
  int y = x * 1;
  int z = x + 0;
  int w = x - 0;
  int zero = x * 0;
  int div_one = x / 1;
  int same_sub = x - x;
  int same_div = x / x;
  int same_mod = x % x;
  int same_xor = x ^ x;
  int shift_zero = x << 0;
  printf(y);
  printf(z);
  printf(w);
  printf(zero);
  printf(div_one);
  printf(same_sub);
  printf(same_div);
  printf(same_mod);
  printf(same_xor);
  printf(shift_zero);
`, [42, 42, 42, 0, 42, 0, 1, 0, 0, 42]);

// 11. Common Subexpression Elimination
assertTest('Common Subexpression Elimination', `
  int a = 15;
  int b = 35;
  int e1 = a * b + 10;
  int e2 = b * a + 20; // b * a should reuse a * b
  int sum = e1 + e2;
  printf(sum);
`, 1080);

// 12. Dead Code & Reassignment Elimination
assertTest('Overwritten variables & Dead Code Elimination', `
  int x = 100;
  int dead1 = x * 999;
  int dead2 = dead1 + 500;
  x = 200; // overwrites previous x
  int active = x + 50;
  printf(active);
`, 250);

// 13. Deep Cascading Multi-Pass Optimization
assertTest('Deep Cascade Stress Test', `
  int c1 = 10;
  int c2 = 20;
  int c3 = c1 * c2; // 200
  int c4 = c3 + 100; // 300
  int c5 = c4 * 2; // 600
  int c6 = c5 / 3; // 200
  int c7 = c6 - 50; // 150
  int c8 = c7 * 4; // 600

  int dead_calc = c8 * 123 + 456;

  int a = 15;
  int b = 35;
  int term1 = a + b; // 50
  int term2 = b + a; // 50 (CSE)
  int term3 = a * b; // 525
  int combined = term1 + term2 + term3; // 625

  int final_val = c8 + combined; // 1225
  printf(final_val);
`, 1225);

// 14. Modern C / C++ syntax with comments and wrappers
assertTest('C/C++ Function Wrapper & Comments', `
  #include <stdio.h>
  #include <stdlib.h>

  int main() {
    /* Multi-line comment
       testing parser strip */
    int sensor_val = 1024; // 10-bit ADC
    int threshold = 512;
    int is_high = sensor_val > threshold;
    int voltage = (sensor_val * 3300) / 1024; // in millivolts: 3300
    printf(is_high);
    printf(voltage);
    return 0;
  }
`, [1, 3300]);

console.log('\n=====================================================');
console.log(` RESULTS: ${passedTests}/${totalTests} Passed (${failedTests} Failed)`);
console.log('=====================================================');

if (failedTests > 0) {
  process.exit(1);
} else {
  console.log(' ALL CODES COMPILED AND VERIFIED WITH 100% MATHEMATICAL ACCURACY!');
}
