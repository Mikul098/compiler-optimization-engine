export const TEST_SUITES = [
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
