/**
 * ==============================================================================
 * File: frontend/src/services/compilerEngine.js
 * Purpose: Full-fidelity JavaScript emulation of the Toy Compiler Pipeline.
 * 
 * Capabilities:
 *  - Tokenizer & Recursive-Descent AST Builder (respecting PEMDAS).
 *  - Three-Address Code (TAC) Quad Generator.
 *  - Step-by-Step Multi-Pass Optimizer:
 *      * ConstantFoldingPass (propagation & folding)
 *      * CommonSubexpressionPass (with canonical commutative signatures)
 *      * DeadCodeEliminationPass (backward liveness analysis)
 *      * PassManager fixed-point iteration recorder.
 *  - DiffEngine metrics computation (initial, final, reduction %, side-by-side rows).
 *  - Virtual Machine (Interpreter) execution of pre- and post-optimization TAC.
 * ==============================================================================
 */

export function parseToyProgram(source) {
  // Strip multi-line comments: /* ... */
  let cleanSource = source.replace(/\/\*[\s\S]*?\*\//g, '');
  // Strip preprocessor lines: #include ... or #define ...
  cleanSource = cleanSource.replace(/^[ \t]*#.*$/gm, '');

  const lines = cleanSource.split('\n');
  const statements = [];
  const errors = [];

  const typeKeywords = /^(?:int|float|double|char|long|short|unsigned|signed|void|auto|const|var|let|bool|size_t|constexpr|static|inline|extern|volatile)\s+/;

  lines.forEach((rawLine, lineIdx) => {
    let line = rawLine.trim();
    const commentIdx = line.indexOf('//');
    if (commentIdx !== -1) {
      line = line.substring(0, commentIdx).trim();
    }
    if (!line) return;

    // Handle function headers, control flow headers, labels, and block braces
    if (/^(?:(?:int|void|auto|char|long|bool|double|float)\s+)?(?:main|[a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*\{?$/.test(line) ||
        /^(?:if|while|for|switch)\s*\([^)]*\)\s*\{?$/.test(line) ||
        /^(?:case\s+[^:]+|default)\s*:/.test(line) ||
        /^(?:break|continue|goto\s+[a-zA-Z_][a-zA-Z0-9_]*);?$/.test(line) ||
        /^[a-zA-Z_][a-zA-Z0-9_]*\s*:$/.test(line) ||
        /^(?:do|try|catch\s*\([^)]*\)|else)(?:\s*\{)?$/.test(line) ||
        /^static_assert\s*\([^)]*\);?$/.test(line) ||
        line === '{' || line === '}') {
      return;
    }

    if (!line.endsWith(';')) {
      errors.push(`Syntax error on line ${lineIdx + 1}: Missing semicolon`);
      return;
    }
    line = line.slice(0, -1).trim();

    // Ignore return statements: return 0; or return;
    if (/^return(?:\s+.*)?$/.test(line)) {
      return;
    }

    // Strip leading type specifier (e.g. int a = 10 -> a = 10)
    line = line.replace(typeKeywords, '');

    // Check for printf: printf("...", x, y) or printf(x)
    const printfMulti = line.match(/^printf\s*\(\s*"[^"]*"\s*,\s*(.+)\s*\)$/);
    if (printfMulti) {
      const args = printfMulti[1].split(',');
      args.forEach(arg => {
        const expr = parseExpression(arg.trim());
        statements.push({ type: 'print', expr, raw: line });
      });
      return;
    }

    const printfSingle = line.match(/^printf\s*\(\s*([^",]+)\s*\)$/);
    if (printfSingle) {
      const expr = parseExpression(printfSingle[1].trim());
      statements.push({ type: 'print', expr, raw: line });
      return;
    }

    // Check for cout: cout << x; or cout << x << endl;
    const coutMatch = line.match(/^(?:std::)?cout\s*<<\s*([^<;]+)(?:\s*<<\s*(?:std::)?endl)?$/);
    if (coutMatch) {
      const expr = parseExpression(coutMatch[1].trim());
      statements.push({ type: 'print', expr, raw: line });
      return;
    }

    // Check for print statement: print x or print(x)
    const printMatch = line.match(/^print\s*\(?\s*(.+)\s*\)?$/);
    if (printMatch) {
      const expr = parseExpression(printMatch[1].trim());
      statements.push({ type: 'print', expr, raw: line });
      return;
    }

    // Check for compound assignments: id += expr, id -= expr, id *= expr, id /= expr, id %= expr, id &= expr, etc.
    const compoundMatch = line.match(/^([a-zA-Z_][a-zA-Z0-9_]*)\s*(\+=|-=|\*=|\/=|%=|&=|\|=|\^=)\s*(.+)$/);
    if (compoundMatch) {
      const varName = compoundMatch[1];
      const opStr = compoundMatch[2];
      const opChar = opStr[0];
      const exprStr = compoundMatch[3].trim();
      const rhsExpr = parseExpression(exprStr);
      const expr = {
        type: 'op',
        op: opChar,
        left: { type: 'var', name: varName },
        right: rhsExpr
      };
      statements.push({ type: 'assign', varName, expr, raw: line });
      return;
    }

    // Check for multiple comma-separated declarations/assignments: a = 10, b = 20, c
    if (line.includes(',')) {
      const parts = line.split(',');
      parts.forEach(part => {
        const p = part.trim();
        const m = p.match(/^([a-zA-Z_][a-zA-Z0-9_]*)(?:\s*=\s*(.+))?$/);
        if (m) {
          const varName = m[1];
          const expr = m[2] ? parseExpression(m[2].trim()) : { type: 'num', value: 0 };
          statements.push({ type: 'assign', varName, expr, raw: line });
        }
      });
      return;
    }

    // Check for assignment: id = expr
    const assignMatch = line.match(/^([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(.+)$/);
    if (assignMatch) {
      const varName = assignMatch[1];
      const exprStr = assignMatch[2].trim();
      const expr = parseExpression(exprStr);
      statements.push({ type: 'assign', varName, expr, raw: line });
      return;
    }

    // Plain variable declaration without assignment: int a;
    const declMatch = line.match(/^([a-zA-Z_][a-zA-Z0-9_]*)$/);
    if (declMatch) {
      const varName = declMatch[1];
      statements.push({ type: 'assign', varName, expr: { type: 'num', value: 0 }, raw: line });
      return;
    }

    errors.push(`Syntax error on line ${lineIdx + 1}: Unrecognized statement "${line}"`);
  });

  return { statements, errors };
}

// Tokenize arithmetic expression
function tokenizeExpr(str) {
  const tokens = [];
  let i = 0;
  while (i < str.length) {
    const ch = str[i];
    if (/\s/.test(ch)) {
      i++;
    } else if (/[0-9]/.test(ch)) {
      let num = '';
      while (i < str.length && /[0-9]/.test(str[i])) {
        num += str[i++];
      }
      tokens.push({ type: 'num', value: parseInt(num, 10) });
    } else if (/[a-zA-Z_]/.test(ch)) {
      let id = '';
      while (i < str.length && /[a-zA-Z0-9_]/.test(str[i])) {
        id += str[i++];
      }
      tokens.push({ type: 'var', name: id });
    } else if ('+-*/%&|^()'.includes(ch)) {
      tokens.push({ type: 'op', value: ch });
      i++;
    } else {
      i++;
    }
  }
  return tokens;
}

// Shunting-yard or recursive descent for PEMDAS
function parseExpression(exprStr) {
  const tokens = tokenizeExpr(exprStr);
  let pos = 0;

  function parsePrimary() {
    const tok = tokens[pos];
    if (!tok) return { type: 'num', value: 0 };
    if (tok.type === 'op' && tok.value === '-') {
      pos++;
      const operand = parsePrimary();
      return { type: 'op', op: '-', left: { type: 'num', value: 0 }, right: operand };
    }
    if (tok.type === 'op' && tok.value === '+') {
      pos++;
      return parsePrimary();
    }
    if (tok.type === 'num') {
      pos++;
      return { type: 'num', value: tok.value };
    }
    if (tok.type === 'var') {
      pos++;
      return { type: 'var', name: tok.name };
    }
    if (tok.type === 'op' && tok.value === '(') {
      pos++;
      const node = parseAddSub();
      if (tokens[pos] && tokens[pos].value === ')') {
        pos++;
      }
      return node;
    }
    pos++;
    return { type: 'num', value: 0 };
  }

  function parseMulDiv() {
    let left = parsePrimary();
    while (pos < tokens.length && (tokens[pos].value === '*' || tokens[pos].value === '/' || tokens[pos].value === '%')) {
      const op = tokens[pos].value;
      pos++;
      const right = parsePrimary();
      left = { type: 'op', op, left, right };
    }
    return left;
  }

  function parseAddSub() {
    let left = parseMulDiv();
    while (pos < tokens.length && (tokens[pos].value === '+' || tokens[pos].value === '-' || tokens[pos].value === '&' || tokens[pos].value === '|' || tokens[pos].value === '^')) {
      const op = tokens[pos].value;
      pos++;
      const right = parseMulDiv();
      left = { type: 'op', op, left, right };
    }
    return left;
  }

  return parseAddSub();
}

// Build AST Node Hierarchy for Graph Visualizer
export function buildASTGraph(statements) {
  const root = {
    id: 'root',
    type: 'root',
    name: 'Program (Root)',
    children: []
  };

  statements.forEach((stmt, idx) => {
    if (stmt.type === 'assign') {
      const assignNode = {
        id: `assign_${idx}`,
        type: 'assign',
        name: `Assignment: ${stmt.varName} =`,
        children: [exprToASTNode(stmt.expr, `val_${idx}`)]
      };
      root.children.push(assignNode);
    } else if (stmt.type === 'print') {
      const printNode = {
        id: `print_${idx}`,
        type: 'print',
        name: 'Print Statement',
        children: [exprToASTNode(stmt.expr, `arg_${idx}`)]
      };
      root.children.push(printNode);
    }
  });

  return root;
}

function exprToASTNode(expr, prefix = 'node') {
  if (!expr) return { id: prefix, type: 'num', name: '0', children: [] };
  if (expr.type === 'num') {
    return {
      id: `${prefix}_num`,
      type: 'num',
      name: `Number: ${expr.value}`,
      value: expr.value,
      children: []
    };
  }
  if (expr.type === 'var') {
    return {
      id: `${prefix}_var`,
      type: 'var',
      name: `Variable: ${expr.name}`,
      varName: expr.name,
      children: []
    };
  }
  if (expr.type === 'op') {
    const opNames = { '+': 'Add', '-': 'Subtract', '*': 'Multiply', '/': 'Divide' };
    return {
      id: `${prefix}_op`,
      type: 'op',
      name: `BinaryOp: ${expr.op} (${opNames[expr.op] || 'Op'})`,
      op: expr.op,
      children: [
        exprToASTNode(expr.left, `${prefix}_L`),
        exprToASTNode(expr.right, `${prefix}_R`)
      ]
    };
  }
  return { id: prefix, type: 'num', name: 'Unknown', children: [] };
}

// Generate Linear Three-Address Code (TAC)
export function generateTAC(statements) {
  let tempCount = 1;
  const instructions = [];

  function newTemp() {
    return `t${tempCount++}`;
  }

  function emitExpr(expr) {
    if (expr.type === 'num') return String(expr.value);
    if (expr.type === 'var') return expr.name;
    if (expr.type === 'op') {
      const arg1 = emitExpr(expr.left);
      const arg2 = emitExpr(expr.right);
      const result = newTemp();
      const opMap = { '+': 'ADD', '-': 'SUB', '*': 'MUL', '/': 'DIV' };
      instructions.push({
        op: opMap[expr.op] || 'ADD',
        arg1,
        arg2,
        result
      });
      return result;
    }
    return '';
  }

  statements.forEach(stmt => {
    if (stmt.type === 'assign') {
      const val = emitExpr(stmt.expr);
      instructions.push({
        op: 'ASSIGN',
        arg1: val,
        arg2: '',
        result: stmt.varName
      });
    } else if (stmt.type === 'print') {
      const val = emitExpr(stmt.expr);
      instructions.push({
        op: 'PRINT',
        arg1: val,
        arg2: '',
        result: ''
      });
    }
  });

  return instructions;
}

export function instructionToString(instr) {
  switch (instr.op) {
    case 'ADD': return `${instr.result} = ${instr.arg1} + ${instr.arg2}`;
    case 'SUB': return `${instr.result} = ${instr.arg1} - ${instr.arg2}`;
    case 'MUL': return `${instr.result} = ${instr.arg1} * ${instr.arg2}`;
    case 'DIV': return `${instr.result} = ${instr.arg1} / ${instr.arg2}`;
    case 'ASSIGN': return `${instr.result} = ${instr.arg1}`;
    case 'PRINT': return `print ${instr.arg1}`;
    case 'NOP': return 'nop';
    default: return `${instr.result} = ${instr.arg1}`;
  }
}

function isNumeric(str) {
  if (!str) return false;
  return /^-?\d+$/.test(str.trim());
}

// 1. Constant Folding Pass
function runConstantFoldingPass(instructions) {
  const knownConstants = new Map();
  const modified = [];
  let changes = 0;

  for (let i = 0; i < instructions.length; i++) {
    const instr = { ...instructions[i] };
    const original = instructionToString(instr);

    // Propagation
    if (knownConstants.has(instr.arg1)) {
      instr.arg1 = String(knownConstants.get(instr.arg1));
    }
    if (knownConstants.has(instr.arg2)) {
      instr.arg2 = String(knownConstants.get(instr.arg2));
    }

    // Folding
    const isArithmetic = ['ADD', 'SUB', 'MUL', 'DIV'].includes(instr.op);
    if (isArithmetic && isNumeric(instr.arg1) && isNumeric(instr.arg2)) {
      const v1 = parseInt(instr.arg1, 10);
      const v2 = parseInt(instr.arg2, 10);
      let res = 0;
      let divZero = false;

      if (instr.op === 'ADD') res = v1 + v2;
      else if (instr.op === 'SUB') res = v1 - v2;
      else if (instr.op === 'MUL') res = v1 * v2;
      else if (instr.op === 'DIV') {
        if (v2 === 0) divZero = true;
        else res = Math.trunc(v1 / v2);
      }

      if (!divZero) {
        instr.op = 'ASSIGN';
        instr.arg1 = String(res);
        instr.arg2 = '';
        knownConstants.set(instr.result, res);
      }
    } else if (instr.op === 'ASSIGN' && isNumeric(instr.arg1)) {
      knownConstants.set(instr.result, parseInt(instr.arg1, 10));
    }

    if (instructionToString(instr) !== original) {
      changes++;
    }
    modified.push(instr);
  }

  return { instructions: modified, changes };
}

// 2. Common Subexpression Elimination Pass (with Commutativity)
function getExpressionSignature(op, arg1, arg2) {
  let a1 = arg1;
  let a2 = arg2;
  if ((op === 'ADD' || op === 'MUL') && a1.localeCompare(a2) > 0) {
    const tmp = a1;
    a1 = a2;
    a2 = tmp;
  }
  return `${op} ${a1} ${a2}`;
}

function runCommonSubexpressionPass(instructions) {
  const availableExprs = new Map();
  const modified = [];
  let changes = 0;

  for (let i = 0; i < instructions.length; i++) {
    const instr = { ...instructions[i] };
    const original = instructionToString(instr);

    // Redefinition invalidation: invalidate if arg1, arg2, or resultVar matches instr.result
    if (instr.result) {
      for (const [sig, res] of Array.from(availableExprs.entries())) {
        const parts = sig.split(' ');
        if (parts[1] === instr.result || parts[2] === instr.result || res === instr.result) {
          availableExprs.delete(sig);
        }
      }
    }

    const isMath = ['ADD', 'SUB', 'MUL', 'DIV'].includes(instr.op);
    if (isMath) {
      const sig = getExpressionSignature(instr.op, instr.arg1, instr.arg2);
      if (availableExprs.has(sig)) {
        instr.op = 'ASSIGN';
        instr.arg1 = availableExprs.get(sig);
        instr.arg2 = '';
      } else {
        availableExprs.set(sig, instr.result);
      }
    }

    if (instructionToString(instr) !== original) {
      changes++;
    }
    modified.push(instr);
  }

  return { instructions: modified, changes };
}

// 3. Dead Code Elimination Pass
function runDeadCodeEliminationPass(instructions) {
  const usedVars = new Set();
  const activeInstructions = [];
  let changes = 0;

  const hasPrint = instructions.some(instr => instr.op === 'PRINT');
  if (!hasPrint) {
    instructions.forEach(instr => {
      if (instr.result && !/^t\d+$/.test(instr.result)) {
        usedVars.add(instr.result);
      }
    });
  }

  // Backward liveness
  for (let i = instructions.length - 1; i >= 0; i--) {
    const instr = instructions[i];

    if (instr.op === 'PRINT') {
      activeInstructions.push(instr);
      if (!isNumeric(instr.arg1) && instr.arg1) {
        usedVars.add(instr.arg1);
      }
    } else {
      if (usedVars.has(instr.result)) {
        activeInstructions.push(instr);
        if (!isNumeric(instr.arg1) && instr.arg1) {
          usedVars.add(instr.arg1);
        }
        if (!isNumeric(instr.arg2) && instr.arg2) {
          usedVars.add(instr.arg2);
        }
      } else {
        // Dead assignment eliminated
        changes++;
      }
    }
  }

  activeInstructions.reverse();
  return { instructions: activeInstructions, changes };
}

// Pass Manager Fixed-Point Loop with Intermediate Snapshots
export function runPassManager(initialInstructions) {
  let instructions = initialInstructions.map(i => ({ ...i }));
  const history = [
    {
      step: 0,
      name: 'Initial Unoptimized TAC',
      passName: 'Baseline Generation',
      instructions: instructions.map(i => ({ ...i })),
      summary: 'Raw linearized quad instructions prior to optimization passes.'
    }
  ];

  let iteration = 1;
  let changed = true;
  const maxIterations = 20;

  while (changed && iteration <= maxIterations) {
    changed = false;
    const beforeState = instructions.map(instructionToString).join('\n');

    // Pass 1: Constant Folding
    const cf = runConstantFoldingPass(instructions);
    instructions = cf.instructions;
    history.push({
      step: history.length,
      iteration,
      name: `Loop ${iteration}: Constant Folding`,
      passName: 'ConstantFoldingPass',
      changes: cf.changes,
      instructions: instructions.map(i => ({ ...i })),
      summary: cf.changes > 0 
        ? `Propagated known constants and folded arithmetic operations (${cf.changes} instructions modified).`
        : 'No further constant folding opportunities detected.'
    });

    // Pass 2: Common Subexpression Elimination
    const cse = runCommonSubexpressionPass(instructions);
    instructions = cse.instructions;
    history.push({
      step: history.length,
      iteration,
      name: `Loop ${iteration}: Common Subexpressions`,
      passName: 'CommonSubexpressionPass',
      changes: cse.changes,
      instructions: instructions.map(i => ({ ...i })),
      summary: cse.changes > 0
        ? `Identified and substituted canonical duplicate subexpressions (${cse.changes} redundant operations removed).`
        : 'No duplicate signatures available.'
    });

    // Pass 3: Dead Code Elimination
    const dce = runDeadCodeEliminationPass(instructions);
    instructions = dce.instructions;
    history.push({
      step: history.length,
      iteration,
      name: `Loop ${iteration}: Dead Code Elimination`,
      passName: 'DeadCodeEliminationPass',
      changes: dce.changes,
      instructions: instructions.map(i => ({ ...i })),
      summary: dce.changes > 0
        ? `Eliminated orphaned intermediate assignments (${dce.changes} dead instructions pruned).`
        : 'All surviving variables are active.'
    });

    const afterState = instructions.map(instructionToString).join('\n');
    if (beforeState !== afterState) {
      changed = true;
      iteration++;
    }
  }

  return {
    finalInstructions: instructions,
    totalIterations: iteration,
    history
  };
}

// Diff Engine Computation
export function computeDiff(beforeState, afterState) {
  const beforeCount = beforeState.length;
  const afterCount = afterState.length;
  const reduced = beforeCount - afterCount;
  const percentage = beforeCount > 0 ? ((reduced / beforeCount) * 100).toFixed(1) : 0;

  const maxSize = Math.max(beforeCount, afterCount);
  const rows = [];

  for (let i = 0; i < maxSize; i++) {
    const beforeInstr = i < beforeCount ? beforeState[i] : null;
    const afterInstr = i < afterCount ? afterState[i] : null;

    rows.push({
      index: i + 1,
      beforeStr: beforeInstr ? instructionToString(beforeInstr) : '',
      afterStr: afterInstr ? instructionToString(afterInstr) : '',
      isEliminated: !afterInstr && !!beforeInstr,
      isKept: !!afterInstr && !!beforeInstr,
      isNewOrModified: afterInstr && beforeInstr && instructionToString(afterInstr) !== instructionToString(beforeInstr)
    });
  }

  return {
    beforeCount,
    afterCount,
    reduced,
    percentage,
    rows
  };
}

// Virtual Machine (Interpreter)
export function executeTAC(instructions) {
  const memory = new Map();
  const output = [];
  const memoryLogs = [];

  function getValue(arg) {
    if (!arg) return 0;
    if (isNumeric(arg)) return parseInt(arg, 10);
    return memory.get(arg) || 0;
  }

  instructions.forEach((instr, idx) => {
    switch (instr.op) {
      case 'ASSIGN':
        memory.set(instr.result, getValue(instr.arg1));
        break;
      case 'ADD':
        memory.set(instr.result, getValue(instr.arg1) + getValue(instr.arg2));
        break;
      case 'SUB':
        memory.set(instr.result, getValue(instr.arg1) - getValue(instr.arg2));
        break;
      case 'MUL':
        memory.set(instr.result, getValue(instr.arg1) * getValue(instr.arg2));
        break;
      case 'DIV': {
        const d = getValue(instr.arg2);
        memory.set(instr.result, d !== 0 ? Math.trunc(getValue(instr.arg1) / d) : 0);
        break;
      }
      case 'PRINT':
        output.push(getValue(instr.arg1));
        break;
      case 'NOP':
      default:
        break;
    }

    memoryLogs.push({
      step: idx + 1,
      instruction: instructionToString(instr),
      registers: Object.fromEntries(memory.entries())
    });
  });

  return {
    output,
    memorySnapshot: Object.fromEntries(memory.entries()),
    memoryLogs
  };
}

// Complete Full Pipeline Runner
export async function runFullPipeline(sourceCode) {
  const startTime = performance.now();

  // Try calling native backend API first
  let nativeResult = null;
  try {
    const res = await fetch('/api/compile', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ code: sourceCode })
    });
    if (res.ok) {
      nativeResult = await res.json();
    }
  } catch (e) {
    // Backend API offline; fallback seamlessly to client-side compiler engine
  }

  // Client-side parser and pass manager
  const { statements, errors } = parseToyProgram(sourceCode);
  if (errors.length > 0) {
    const elapsed = (performance.now() - startTime).toFixed(1);
    return {
      success: false,
      errors,
      elapsed,
      astGraph: null,
      initialTAC: [],
      optimizedTAC: [],
      diff: null,
      passHistory: [],
      verification: null,
      astImage: nativeResult?.astImage || null,
      nativeStdout: nativeResult?.stdout || null
    };
  }

  const astGraph = buildASTGraph(statements);
  const initialTAC = generateTAC(statements);
  const { finalInstructions, totalIterations, history: passHistory } = runPassManager(initialTAC);
  const diff = computeDiff(initialTAC, finalInstructions);

  // Run Virtual Machine Interpreter on both versions
  const unoptimizedVM = executeTAC(initialTAC);
  const optimizedVM = executeTAC(finalInstructions);

  const isMatched = JSON.stringify(unoptimizedVM.output) === JSON.stringify(optimizedVM.output);
  const elapsed = (performance.now() - startTime).toFixed(1);

  return {
    success: true,
    elapsed,
    statements,
    astGraph,
    initialTAC,
    optimizedTAC: finalInstructions,
    totalIterations,
    passHistory,
    diff,
    verification: {
      passed: isMatched,
      unoptimizedOutput: unoptimizedVM.output,
      optimizedOutput: optimizedVM.output,
      memory: optimizedVM.memorySnapshot,
      memoryLogs: optimizedVM.memoryLogs
    },
    astImage: nativeResult?.astImage || null,
    nativeStdout: nativeResult?.stdout || null
  };
}
