/**
 * ==============================================================================
 * File: frontend/src/services/compilerEngine.js
 * Purpose: Full-fidelity JavaScript emulation of the Toy Compiler Pipeline.
 *
 * Capabilities:
 *  - Tokenizer & Recursive-Descent AST Builder (respecting PEMDAS).
 *  - Three-Address Code (TAC) Quad Generator.
 *  - Step-by-Step Multi-Pass Optimizer (8 passes, mirrors C++ backend exactly):
 *      1. ConstantPropagationPass   - Forward-propagate known constants to expose folding
 *      2. ConstantFoldingPass       - Fold fully-constant arithmetic/bitwise/logical exprs
 *      3. AlgebraicSimplificationPass - Algebraic identities + strength reduction
 *      4. CopyPropagationPass       - Eliminate temporaries by propagating copies
 *      5. RedundantAssignmentPass   - Remove x=x self-assignments
 *      6. CommonSubexpressionPass   - Replace duplicate computations with a single copy
 *      7. DeadCodeEliminationPass   - Backward liveness analysis prunes dead assignments
 *      8. PeepholePass              - Local 2-instruction window micro-optimizations
 *      + PassManager fixed-point iteration recorder.
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

// Tokenize expression (supports hex 0x, binary 0b, decimal, ids, multi-char & single-char ops)
function tokenizeExpr(str) {
  const tokens = [];
  let i = 0;
  while (i < str.length) {
    const ch = str[i];
    if (/\s/.test(ch)) {
      i++;
    } else if (ch === '0' && (str[i + 1] === 'x' || str[i + 1] === 'X')) {
      // Hex literal: 0xFF, 0x1A
      let hex = '0x';
      i += 2;
      while (i < str.length && /[0-9a-fA-F]/.test(str[i])) {
        hex += str[i++];
      }
      tokens.push({ type: 'num', value: parseInt(hex, 16) });
    } else if (ch === '0' && (str[i + 1] === 'b' || str[i + 1] === 'B')) {
      // Binary literal: 0b1010
      let bin = '';
      i += 2;
      while (i < str.length && /[01]/.test(str[i])) {
        bin += str[i++];
      }
      tokens.push({ type: 'num', value: parseInt(bin, 2) });
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
      // Alternate keyword tokens
      if (id === 'and') tokens.push({ type: 'op', value: '&&' });
      else if (id === 'or') tokens.push({ type: 'op', value: '||' });
      else if (id === 'not') tokens.push({ type: 'op', value: '!' });
      else if (id === 'bitand') tokens.push({ type: 'op', value: '&' });
      else if (id === 'bitor') tokens.push({ type: 'op', value: '|' });
      else if (id === 'xor') tokens.push({ type: 'op', value: '^' });
      else if (id === 'compl') tokens.push({ type: 'op', value: '~' });
      else if (id === 'true') tokens.push({ type: 'num', value: 1 });
      else if (id === 'false') tokens.push({ type: 'num', value: 0 });
      else tokens.push({ type: 'var', name: id });
    } else if (str.substr(i, 2) === '<<') {
      tokens.push({ type: 'op', value: '<<' }); i += 2;
    } else if (str.substr(i, 2) === '>>') {
      tokens.push({ type: 'op', value: '>>' }); i += 2;
    } else if (str.substr(i, 2) === '==') {
      tokens.push({ type: 'op', value: '==' }); i += 2;
    } else if (str.substr(i, 2) === '!=') {
      tokens.push({ type: 'op', value: '!=' }); i += 2;
    } else if (str.substr(i, 2) === '<=') {
      tokens.push({ type: 'op', value: '<=' }); i += 2;
    } else if (str.substr(i, 2) === '>=') {
      tokens.push({ type: 'op', value: '>=' }); i += 2;
    } else if (str.substr(i, 2) === '&&') {
      tokens.push({ type: 'op', value: '&&' }); i += 2;
    } else if (str.substr(i, 2) === '||') {
      tokens.push({ type: 'op', value: '||' }); i += 2;
    } else if ('+-*/%&|^~!()<>'.includes(ch)) {
      tokens.push({ type: 'op', value: ch });
      i++;
    } else {
      i++;
    }
  }
  return tokens;
}

// Recursive-descent expression parser respecting full C operator precedence
function parseExpression(exprStr) {
  const tokens = tokenizeExpr(exprStr);
  let pos = 0;

  function parsePrimary() {
    const tok = tokens[pos];
    if (!tok) return { type: 'num', value: 0 };
    if (tok.type === 'op' && tok.value === '-') {
      pos++;
      const nextTok = tokens[pos];
      if (nextTok && nextTok.type === 'num') {
        pos++;
        return { type: 'num', value: -nextTok.value };
      }
      const operand = parsePrimary();
      return { type: 'op', op: '-', left: { type: 'num', value: 0 }, right: operand };
    }
    if (tok.type === 'op' && tok.value === '+') {
      pos++;
      return parsePrimary();
    }
    if (tok.type === 'op' && tok.value === '~') {
      pos++;
      const operand = parsePrimary();
      return { type: 'unary', op: '~', expr: operand };
    }
    if (tok.type === 'op' && tok.value === '!') {
      pos++;
      const operand = parsePrimary();
      return { type: 'op', op: '==', left: operand, right: { type: 'num', value: 0 } };
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
      const node = parseLogicalOr();
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
    while (pos < tokens.length && (tokens[pos].value === '+' || tokens[pos].value === '-')) {
      const op = tokens[pos].value;
      pos++;
      const right = parseMulDiv();
      left = { type: 'op', op, left, right };
    }
    return left;
  }

  function parseShift() {
    let left = parseAddSub();
    while (pos < tokens.length && (tokens[pos].value === '<<' || tokens[pos].value === '>>')) {
      const op = tokens[pos].value;
      pos++;
      const right = parseAddSub();
      left = { type: 'op', op, left, right };
    }
    return left;
  }

  function parseRelational() {
    let left = parseShift();
    while (pos < tokens.length && (tokens[pos].value === '<' || tokens[pos].value === '<=' || tokens[pos].value === '>' || tokens[pos].value === '>=')) {
      const op = tokens[pos].value;
      pos++;
      const right = parseShift();
      left = { type: 'op', op, left, right };
    }
    return left;
  }

  function parseEquality() {
    let left = parseRelational();
    while (pos < tokens.length && (tokens[pos].value === '==' || tokens[pos].value === '!=')) {
      const op = tokens[pos].value;
      pos++;
      const right = parseRelational();
      left = { type: 'op', op, left, right };
    }
    return left;
  }

  function parseBitwiseAnd() {
    let left = parseEquality();
    while (pos < tokens.length && tokens[pos].value === '&') {
      pos++;
      const right = parseEquality();
      left = { type: 'op', op: '&', left, right };
    }
    return left;
  }

  function parseBitwiseXor() {
    let left = parseBitwiseAnd();
    while (pos < tokens.length && tokens[pos].value === '^') {
      pos++;
      const right = parseBitwiseAnd();
      left = { type: 'op', op: '^', left, right };
    }
    return left;
  }

  function parseBitwiseOr() {
    let left = parseBitwiseXor();
    while (pos < tokens.length && tokens[pos].value === '|') {
      pos++;
      const right = parseBitwiseXor();
      left = { type: 'op', op: '|', left, right };
    }
    return left;
  }

  function parseLogicalAnd() {
    let left = parseBitwiseOr();
    while (pos < tokens.length && tokens[pos].value === '&&') {
      pos++;
      const right = parseBitwiseOr();
      left = { type: 'op', op: '&&', left, right };
    }
    return left;
  }

  function parseLogicalOr() {
    let left = parseLogicalAnd();
    while (pos < tokens.length && tokens[pos].value === '||') {
      pos++;
      const right = parseLogicalAnd();
      left = { type: 'op', op: '||', left, right };
    }
    return left;
  }

  return parseLogicalOr();
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
  if (expr.type === 'unary') {
    return {
      id: `${prefix}_unary`,
      type: 'op',
      name: `UnaryOp: ${expr.op}`,
      op: expr.op,
      children: [exprToASTNode(expr.expr, `${prefix}_U`)]
    };
  }
  if (expr.type === 'op') {
    const opNames = {
      '+': 'Add', '-': 'Subtract', '*': 'Multiply', '/': 'Divide', '%': 'Modulo',
      '&': 'BitwiseAnd', '|': 'BitwiseOr', '^': 'BitwiseXor',
      '<<': 'ShiftLeft', '>>': 'ShiftRight',
      '&&': 'LogicalAnd', '||': 'LogicalOr',
      '==': 'Equal', '!=': 'NotEqual',
      '<': 'LessThan', '>': 'GreaterThan', '<=': 'LessEqual', '>=': 'GreaterEqual'
    };
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
    if (!expr) return '0';
    if (expr.type === 'num') return String(expr.value);
    if (expr.type === 'var') return expr.name;
    if (expr.type === 'unary') {
      const arg1 = emitExpr(expr.expr);
      const result = newTemp();
      if (expr.op === '~') {
        instructions.push({ op: 'BNOT', arg1, arg2: '', result });
      } else {
        instructions.push({ op: 'ASSIGN', arg1, arg2: '', result });
      }
      return result;
    }
    if (expr.type === 'op') {
      const arg1 = emitExpr(expr.left);
      const arg2 = emitExpr(expr.right);
      const result = newTemp();
      const opMap = {
        '+': 'ADD', '-': 'SUB', '*': 'MUL', '/': 'DIV', '%': 'MOD',
        '&': 'BAND', '|': 'BOR', '^': 'BXOR',
        '<<': 'SHL', '>>': 'SHR',
        '&&': 'AND', '||': 'OR',
        '==': 'EQ', '!=': 'NE',
        '<': 'LT', '>': 'GT', '<=': 'LE', '>=': 'GE'
      };
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
    case 'ADD':  return `${instr.result} = ${instr.arg1} + ${instr.arg2}`;
    case 'SUB':  return `${instr.result} = ${instr.arg1} - ${instr.arg2}`;
    case 'MUL':  return `${instr.result} = ${instr.arg1} * ${instr.arg2}`;
    case 'DIV':  return `${instr.result} = ${instr.arg1} / ${instr.arg2}`;
    case 'MOD':  return `${instr.result} = ${instr.arg1} % ${instr.arg2}`;
    case 'BAND': return `${instr.result} = ${instr.arg1} & ${instr.arg2}`;
    case 'BOR':  return `${instr.result} = ${instr.arg1} | ${instr.arg2}`;
    case 'BXOR': return `${instr.result} = ${instr.arg1} ^ ${instr.arg2}`;
    case 'SHL':  return `${instr.result} = ${instr.arg1} << ${instr.arg2}`;
    case 'SHR':  return `${instr.result} = ${instr.arg1} >> ${instr.arg2}`;
    case 'AND':  return `${instr.result} = ${instr.arg1} && ${instr.arg2}`;
    case 'OR':   return `${instr.result} = ${instr.arg1} || ${instr.arg2}`;
    case 'EQ':   return `${instr.result} = ${instr.arg1} == ${instr.arg2}`;
    case 'NE':   return `${instr.result} = ${instr.arg1} != ${instr.arg2}`;
    case 'LT':   return `${instr.result} = ${instr.arg1} < ${instr.arg2}`;
    case 'GT':   return `${instr.result} = ${instr.arg1} > ${instr.arg2}`;
    case 'LE':   return `${instr.result} = ${instr.arg1} <= ${instr.arg2}`;
    case 'GE':   return `${instr.result} = ${instr.arg1} >= ${instr.arg2}`;
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

    if (instr.op === 'LABEL') {
      knownConstants.clear();
      modified.push(instr);
      continue;
    }

    // Propagation
    if (knownConstants.has(instr.arg1)) {
      instr.arg1 = String(knownConstants.get(instr.arg1));
    }
    if (knownConstants.has(instr.arg2)) {
      instr.arg2 = String(knownConstants.get(instr.arg2));
    }

    // Unary BNOT folding
    if (instr.op === 'BNOT' && isNumeric(instr.arg1)) {
      const v = parseInt(instr.arg1, 10);
      const res = (~v) | 0;
      instr.op = 'ASSIGN';
      instr.arg1 = String(res);
      instr.arg2 = '';
      knownConstants.set(instr.result, res);
    }

    // Binary folding (all 16 opcodes)
    const foldOps = [
      'ADD', 'SUB', 'MUL', 'DIV', 'MOD',
      'BAND', 'BOR', 'BXOR', 'SHL', 'SHR',
      'AND', 'OR', 'EQ', 'NE', 'LT', 'GT', 'LE', 'GE'
    ];
    if (foldOps.includes(instr.op) && isNumeric(instr.arg1) && isNumeric(instr.arg2)) {
      const v1 = parseInt(instr.arg1, 10);
      const v2 = parseInt(instr.arg2, 10);
      let res = 0;
      let safe = true;

      switch (instr.op) {
        case 'ADD': res = (v1 + v2) | 0; break;
        case 'SUB': res = (v1 - v2) | 0; break;
        case 'MUL': res = Math.imul(v1, v2); break;
        case 'DIV':
          if (v2 === 0) safe = false;
          else res = Math.trunc(v1 / v2);
          break;
        case 'MOD':
          if (v2 === 0) safe = false;
          else res = v1 % v2;
          break;
        case 'BAND': res = (v1 & v2) | 0; break;
        case 'BOR':  res = (v1 | v2) | 0; break;
        case 'BXOR': res = (v1 ^ v2) | 0; break;
        case 'SHL':
          if (v2 >= 0 && v2 < 31) res = (v1 << v2) | 0;
          else safe = false;
          break;
        case 'SHR':
          if (v2 >= 0 && v2 < 32) res = (v1 >> v2) | 0;
          else safe = false;
          break;
        case 'AND': res = (v1 !== 0 && v2 !== 0) ? 1 : 0; break;
        case 'OR':  res = (v1 !== 0 || v2 !== 0) ? 1 : 0; break;
        case 'EQ':  res = v1 === v2 ? 1 : 0; break;
        case 'NE':  res = v1 !== v2 ? 1 : 0; break;
        case 'LT':  res = v1 < v2 ? 1 : 0; break;
        case 'GT':  res = v1 > v2 ? 1 : 0; break;
        case 'LE':  res = v1 <= v2 ? 1 : 0; break;
        case 'GE':  res = v1 >= v2 ? 1 : 0; break;
        default: safe = false; break;
      }

      if (safe) {
        instr.op = 'ASSIGN';
        instr.arg1 = String(res);
        instr.arg2 = '';
        knownConstants.set(instr.result, res);
      }
    } else if (instr.op === 'ASSIGN' && isNumeric(instr.arg1) && !instr.arg2) {
      knownConstants.set(instr.result, parseInt(instr.arg1, 10));
    } else if (instr.result) {
      knownConstants.delete(instr.result);
    }

    if (instructionToString(instr) !== original) {
      changes++;
    }
    modified.push(instr);
  }

  return { instructions: modified, changes };
}

// 2. Common Subexpression Elimination Pass (with Commutativity & Full Op Support)
function getExpressionSignature(op, arg1, arg2) {
  let a1 = arg1;
  let a2 = arg2;
  const isCommutative = ['ADD', 'MUL', 'AND', 'OR', 'BAND', 'BOR', 'BXOR', 'EQ', 'NE'].includes(op);
  if (isCommutative && a1.localeCompare(a2) > 0) {
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

    if (instr.op === 'LABEL') {
      availableExprs.clear();
      modified.push(instr);
      continue;
    }

    const cseOps = [
      'ADD', 'SUB', 'MUL', 'DIV', 'MOD',
      'BAND', 'BOR', 'BXOR', 'EQ', 'NE',
      'LT', 'GT', 'LE', 'GE'
    ];
    if (cseOps.includes(instr.op)) {
      const sig = getExpressionSignature(instr.op, instr.arg1, instr.arg2);
      if (availableExprs.has(sig)) {
        instr.op = 'ASSIGN';
        instr.arg1 = availableExprs.get(sig);
        instr.arg2 = '';
      } else {
        availableExprs.set(sig, instr.result);
      }
    }

    if (instr.result) {
      for (const [sig, res] of Array.from(availableExprs.entries())) {
        const parts = sig.split(' ');
        if (parts[1] === instr.result || parts[2] === instr.result || res === instr.result) {
          availableExprs.delete(sig);
        }
      }
    }

    if (instructionToString(instr) !== original) {
      changes++;
    }
    modified.push(instr);
  }

  return { instructions: modified, changes };
}

// 3. Dead Code Elimination Pass (Safe Backward Liveness)
function runDeadCodeEliminationPass(instructions) {
  const hasControlFlow = instructions.some(i =>
    i.op === 'LABEL' || i.op === 'GOTO' || i.op === 'IF_ZERO' || i.op === 'IF_NOT_ZERO'
  );

  let changes = 0;

  if (hasControlFlow) {
    const allReadVars = new Set();
    instructions.forEach(i => {
      if (i.arg1 && !isNumeric(i.arg1)) allReadVars.add(i.arg1);
      if (i.arg2 && !isNumeric(i.arg2)) allReadVars.add(i.arg2);
    });

    const valueOps = [
      'ASSIGN', 'ADD', 'SUB', 'MUL', 'DIV', 'MOD',
      'AND', 'OR', 'BAND', 'BOR', 'BXOR', 'BNOT',
      'SHL', 'SHR', 'EQ', 'NE', 'LT', 'GT', 'LE', 'GE'
    ];

    const active = [];
    instructions.forEach(i => {
      if (valueOps.includes(i.op) && i.result && !allReadVars.has(i.result)) {
        changes++;
      } else {
        active.push(i);
      }
    });
    return { instructions: active, changes };
  }

  // Straight-line backward liveness analysis
  const liveVars = new Set();
  const hasPrint = instructions.some(i => i.op === 'PRINT');
  if (!hasPrint) {
    instructions.forEach(i => {
      if (i.result && !/^t\d+$/.test(i.result)) {
        liveVars.add(i.result);
      }
    });
  }

  const active = [];
  for (let i = instructions.length - 1; i >= 0; i--) {
    const instr = instructions[i];
    if (instr.op === 'PRINT') {
      active.push(instr);
      if (instr.arg1 && !isNumeric(instr.arg1)) {
        liveVars.add(instr.arg1);
      }
    } else if (instr.result) {
      if (liveVars.has(instr.result)) {
        active.push(instr);
        liveVars.delete(instr.result);
        if (instr.arg1 && !isNumeric(instr.arg1)) liveVars.add(instr.arg1);
        if (instr.arg2 && !isNumeric(instr.arg2)) liveVars.add(instr.arg2);
      } else {
        changes++;
      }
    } else {
      active.push(instr);
    }
  }

  active.reverse();
  return { instructions: active, changes };
}

// 4. Algebraic Simplification & Strength Reduction
function runAlgebraicSimplificationPass(instructions) {
  let changes = 0;
  const modified = instructions.map(raw => {
    const instr = { ...raw };
    const original = instructionToString(instr);

    // Helper: same non-numeric variable in both operands
    const sameVar = () => instr.arg1 && !isNumeric(instr.arg1) && instr.arg1 === instr.arg2;

    if (instr.op === 'MUL') {
      if (instr.arg1 === '0' || instr.arg2 === '0') { instr.op = 'ASSIGN'; instr.arg1 = '0'; instr.arg2 = ''; }
      else if (instr.arg2 === '1') { instr.op = 'ASSIGN'; instr.arg2 = ''; }
      else if (instr.arg1 === '1') { instr.op = 'ASSIGN'; instr.arg1 = instr.arg2; instr.arg2 = ''; }
      else if (instr.arg2 === '2' && !isNumeric(instr.arg1)) { instr.op = 'SHL'; instr.arg2 = '1'; }
      else if (instr.arg1 === '2' && !isNumeric(instr.arg2)) { instr.op = 'SHL'; instr.arg1 = instr.arg2; instr.arg2 = '1'; }
      else if (instr.arg2 === '4' && !isNumeric(instr.arg1)) { instr.op = 'SHL'; instr.arg2 = '2'; }
      else if (instr.arg1 === '4' && !isNumeric(instr.arg2)) { instr.op = 'SHL'; instr.arg1 = instr.arg2; instr.arg2 = '2'; }
      else if (instr.arg2 === '8' && !isNumeric(instr.arg1)) { instr.op = 'SHL'; instr.arg2 = '3'; }
      else if (instr.arg1 === '8' && !isNumeric(instr.arg2)) { instr.op = 'SHL'; instr.arg1 = instr.arg2; instr.arg2 = '3'; }
    } else if (instr.op === 'ADD') {
      if (instr.arg2 === '0') { instr.op = 'ASSIGN'; instr.arg2 = ''; }
      else if (instr.arg1 === '0') { instr.op = 'ASSIGN'; instr.arg1 = instr.arg2; instr.arg2 = ''; }
    } else if (instr.op === 'SUB') {
      if (instr.arg2 === '0') { instr.op = 'ASSIGN'; instr.arg2 = ''; }
      else if (sameVar()) { instr.op = 'ASSIGN'; instr.arg1 = '0'; instr.arg2 = ''; }
    } else if (instr.op === 'DIV') {
      if (instr.arg2 === '1') { instr.op = 'ASSIGN'; instr.arg2 = ''; }
      else if (sameVar()) { instr.op = 'ASSIGN'; instr.arg1 = '1'; instr.arg2 = ''; }
    } else if (instr.op === 'MOD') {
      if (instr.arg2 === '1' || sameVar()) { instr.op = 'ASSIGN'; instr.arg1 = '0'; instr.arg2 = ''; }
    } else if (instr.op === 'BAND') {
      if (instr.arg1 === '0' || instr.arg2 === '0') { instr.op = 'ASSIGN'; instr.arg1 = '0'; instr.arg2 = ''; }
      else if (sameVar()) { instr.op = 'ASSIGN'; instr.arg2 = ''; }
    } else if (instr.op === 'BOR') {
      if (instr.arg2 === '0') { instr.op = 'ASSIGN'; instr.arg2 = ''; }
      else if (instr.arg1 === '0') { instr.op = 'ASSIGN'; instr.arg1 = instr.arg2; instr.arg2 = ''; }
      else if (sameVar()) { instr.op = 'ASSIGN'; instr.arg2 = ''; }
    } else if (instr.op === 'BXOR') {
      if (sameVar()) { instr.op = 'ASSIGN'; instr.arg1 = '0'; instr.arg2 = ''; }
      else if (instr.arg2 === '0') { instr.op = 'ASSIGN'; instr.arg2 = ''; }
      else if (instr.arg1 === '0') { instr.op = 'ASSIGN'; instr.arg1 = instr.arg2; instr.arg2 = ''; }
    } else if (instr.op === 'SHL' || instr.op === 'SHR') {
      if (instr.arg2 === '0') { instr.op = 'ASSIGN'; instr.arg2 = ''; }
    } else if (instr.op === 'AND') {
      if (instr.arg1 === '0' || instr.arg2 === '0') { instr.op = 'ASSIGN'; instr.arg1 = '0'; instr.arg2 = ''; }
    } else if (instr.op === 'OR') {
      if (instr.arg1 === '1' || instr.arg2 === '1') { instr.op = 'ASSIGN'; instr.arg1 = '1'; instr.arg2 = ''; }
    } else if (instr.op === 'EQ' && sameVar()) {
      instr.op = 'ASSIGN'; instr.arg1 = '1'; instr.arg2 = '';
    } else if (instr.op === 'NE' && sameVar()) {
      instr.op = 'ASSIGN'; instr.arg1 = '0'; instr.arg2 = '';
    } else if ((instr.op === 'LT' || instr.op === 'GT') && sameVar()) {
      instr.op = 'ASSIGN'; instr.arg1 = '0'; instr.arg2 = '';
    } else if ((instr.op === 'LE' || instr.op === 'GE') && sameVar()) {
      instr.op = 'ASSIGN'; instr.arg1 = '1'; instr.arg2 = '';
    }

    if (instructionToString(instr) !== original) changes++;
    return instr;
  });
  return { instructions: modified, changes };
}

// 5. Copy Propagation Pass
function runCopyPropagationPass(instructions) {
  const copyMap = new Map(); // var -> source variable
  let changes = 0;
  const modified = [];

  for (const raw of instructions) {
    const instr = { ...raw };
    const original = instructionToString(instr);

    if (instr.op === 'LABEL') { copyMap.clear(); modified.push(instr); continue; }

    // Propagate
    if (!isNumeric(instr.arg1) && instr.arg1 && copyMap.has(instr.arg1)) instr.arg1 = copyMap.get(instr.arg1);
    if (!isNumeric(instr.arg2) && instr.arg2 && copyMap.has(instr.arg2)) instr.arg2 = copyMap.get(instr.arg2);

    // Invalidate copies involving this result
    if (instr.result) {
      for (const [k, v] of Array.from(copyMap.entries())) {
        if (k === instr.result || v === instr.result) copyMap.delete(k);
      }
      // Record new copy: result = plainVar
      if (instr.op === 'ASSIGN' && instr.arg1 && !isNumeric(instr.arg1) && !instr.arg2) {
        copyMap.set(instr.result, instr.arg1);
      }
    }

    if (instructionToString(instr) !== original) changes++;
    modified.push(instr);
  }
  return { instructions: modified, changes };
}

// 6. Redundant Assignment Elimination
function runRedundantAssignmentPass(instructions) {
  let changes = 0;
  const modified = instructions.filter(raw => {
    if (raw.op === 'ASSIGN' && raw.result && !raw.arg2 && raw.arg1 === raw.result) {
      changes++;
      return false;
    }
    return true;
  });
  return { instructions: modified, changes };
}

// 7. Constant Propagation Pass (cross-block forward)
function runConstantPropagationPass(instructions) {
  const constMap = new Map(); // var -> numeric string
  let changes = 0;
  const modified = [];

  for (const raw of instructions) {
    const instr = { ...raw };
    const original = instructionToString(instr);

    if (instr.op === 'LABEL') { constMap.clear(); modified.push(instr); continue; }

    // Substitute known constants
    if (!isNumeric(instr.arg1) && instr.arg1 && constMap.has(instr.arg1)) instr.arg1 = constMap.get(instr.arg1);
    if (!isNumeric(instr.arg2) && instr.arg2 && constMap.has(instr.arg2)) instr.arg2 = constMap.get(instr.arg2);

    // Update map
    if (instr.result) {
      if (instr.op === 'ASSIGN' && isNumeric(instr.arg1) && !instr.arg2) {
        constMap.set(instr.result, instr.arg1);
      } else {
        constMap.delete(instr.result);
      }
    }

    if (instructionToString(instr) !== original) changes++;
    modified.push(instr);
  }
  return { instructions: modified, changes };
}

// 8. Peephole Optimization Pass
function runPeepholePass(instructions) {
  let changes = 0;
  const result = [...instructions.map(i => ({ ...i }))];

  // Count uses of each variable
  const useCount = new Map();
  for (const instr of result) {
    if (instr.arg1 && !isNumeric(instr.arg1)) useCount.set(instr.arg1, (useCount.get(instr.arg1) || 0) + 1);
    if (instr.arg2 && !isNumeric(instr.arg2)) useCount.set(instr.arg2, (useCount.get(instr.arg2) || 0) + 1);
  }

  // Pattern A: tN = expr; dst = tN  -->  dst = expr  (when tN used exactly once)
  for (let i = 0; i + 1 < result.length; i++) {
    const curr = result[i];
    const next = result[i + 1];
    const tempPattern = /^t\d+$/.test(curr.result);
    if (tempPattern && next.op === 'ASSIGN' && next.arg1 === curr.result && !next.arg2 && useCount.get(curr.result) === 1) {
      curr.result = next.result;
      next.op = 'NOP'; next.arg1 = ''; next.arg2 = ''; next.result = '';
      changes++;
    }
  }

  // Pattern C: consecutive identical assignments x=v; x=v
  for (let i = 0; i + 1 < result.length; i++) {
    const curr = result[i];
    const next = result[i + 1];
    if (curr.op === 'ASSIGN' && next.op === 'ASSIGN' &&
        curr.result === next.result && curr.arg1 === next.arg1 && curr.arg2 === next.arg2) {
      curr.op = 'NOP'; curr.arg1 = ''; curr.arg2 = ''; curr.result = '';
      changes++;
    }
  }

  return { instructions: result.filter(i => i.op !== 'NOP'), changes };
}

// Pass Manager Fixed-Point Loop with Intermediate Snapshots (8 passes, mirrors C++ backend)
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

    // Pass 7: Constant Propagation (run first to expose folding opportunities)
    const cp = runConstantPropagationPass(instructions);
    instructions = cp.instructions;
    history.push({
      step: history.length, iteration,
      name: `Loop ${iteration}: Constant Propagation`,
      passName: 'ConstantPropagationPass',
      changes: cp.changes,
      instructions: instructions.map(i => ({ ...i })),
      summary: cp.changes > 0
        ? `Forward-propagated ${cp.changes} compile-time constants across basic blocks.`
        : 'No new constant propagation opportunities.'
    });

    // Pass 1: Constant Folding
    const cf = runConstantFoldingPass(instructions);
    instructions = cf.instructions;
    history.push({
      step: history.length, iteration,
      name: `Loop ${iteration}: Constant Folding`,
      passName: 'ConstantFoldingPass',
      changes: cf.changes,
      instructions: instructions.map(i => ({ ...i })),
      summary: cf.changes > 0
        ? `Propagated known constants and folded arithmetic operations (${cf.changes} instructions modified).`
        : 'No further constant folding opportunities detected.'
    });

    // Pass 4: Algebraic Simplification & Strength Reduction
    const alg = runAlgebraicSimplificationPass(instructions);
    instructions = alg.instructions;
    history.push({
      step: history.length, iteration,
      name: `Loop ${iteration}: Algebraic Simplification`,
      passName: 'AlgebraicSimplificationPass',
      changes: alg.changes,
      instructions: instructions.map(i => ({ ...i })),
      summary: alg.changes > 0
        ? `Applied ${alg.changes} algebraic identities and strength reductions (e.g. x*2→x<<1, x-x→0).`
        : 'No algebraic simplification opportunities.'
    });

    // Pass 5: Copy Propagation
    const copy = runCopyPropagationPass(instructions);
    instructions = copy.instructions;
    history.push({
      step: history.length, iteration,
      name: `Loop ${iteration}: Copy Propagation`,
      passName: 'CopyPropagationPass',
      changes: copy.changes,
      instructions: instructions.map(i => ({ ...i })),
      summary: copy.changes > 0
        ? `Propagated ${copy.changes} variable copies, eliminating unnecessary temporaries.`
        : 'No copy propagation opportunities.'
    });

    // Pass 6: Redundant Assignment Elimination
    const red = runRedundantAssignmentPass(instructions);
    instructions = red.instructions;
    history.push({
      step: history.length, iteration,
      name: `Loop ${iteration}: Redundant Assignments`,
      passName: 'RedundantAssignmentPass',
      changes: red.changes,
      instructions: instructions.map(i => ({ ...i })),
      summary: red.changes > 0
        ? `Removed ${red.changes} self-assignment no-ops (x = x).`
        : 'No redundant self-assignments found.'
    });

    // Pass 3: Common Subexpression Elimination
    const cse = runCommonSubexpressionPass(instructions);
    instructions = cse.instructions;
    history.push({
      step: history.length, iteration,
      name: `Loop ${iteration}: Common Subexpressions`,
      passName: 'CommonSubexpressionPass',
      changes: cse.changes,
      instructions: instructions.map(i => ({ ...i })),
      summary: cse.changes > 0
        ? `Identified and substituted canonical duplicate subexpressions (${cse.changes} redundant operations removed).`
        : 'No duplicate signatures available.'
    });

    // Pass 2: Dead Code Elimination
    const dce = runDeadCodeEliminationPass(instructions);
    instructions = dce.instructions;
    history.push({
      step: history.length, iteration,
      name: `Loop ${iteration}: Dead Code Elimination`,
      passName: 'DeadCodeEliminationPass',
      changes: dce.changes,
      instructions: instructions.map(i => ({ ...i })),
      summary: dce.changes > 0
        ? `Eliminated orphaned intermediate assignments (${dce.changes} dead instructions pruned).`
        : 'All surviving variables are active.'
    });

    // Pass 8: Peephole
    const ph = runPeepholePass(instructions);
    instructions = ph.instructions;
    history.push({
      step: history.length, iteration,
      name: `Loop ${iteration}: Peephole`,
      passName: 'PeepholePass',
      changes: ph.changes,
      instructions: instructions.map(i => ({ ...i })),
      summary: ph.changes > 0
        ? `Applied ${ph.changes} local 2-instruction window micro-optimizations.`
        : 'No peephole opportunities found.'
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
    const a = () => getValue(instr.arg1);
    const b = () => getValue(instr.arg2);
    switch (instr.op) {
      case 'ASSIGN': memory.set(instr.result, a()); break;
      case 'ADD':    memory.set(instr.result, a() + b()); break;
      case 'SUB':    memory.set(instr.result, a() - b()); break;
      case 'MUL':    memory.set(instr.result, a() * b()); break;
      case 'DIV':    { const d = b(); memory.set(instr.result, d !== 0 ? Math.trunc(a() / d) : 0); break; }
      case 'MOD':    { const d = b(); memory.set(instr.result, d !== 0 ? a() % d : 0); break; }
      case 'BAND':   memory.set(instr.result, (a() & b()) | 0); break;
      case 'BOR':    memory.set(instr.result, (a() | b()) | 0); break;
      case 'BXOR':   memory.set(instr.result, (a() ^ b()) | 0); break;
      case 'BNOT':   memory.set(instr.result, (~a()) | 0); break;
      case 'SHL':    memory.set(instr.result, (a() << b()) | 0); break;
      case 'SHR':    memory.set(instr.result, (a() >> b()) | 0); break;
      case 'AND':    memory.set(instr.result, (a() !== 0 && b() !== 0) ? 1 : 0); break;
      case 'OR':     memory.set(instr.result, (a() !== 0 || b() !== 0) ? 1 : 0); break;
      case 'EQ':     memory.set(instr.result, a() === b() ? 1 : 0); break;
      case 'NE':     memory.set(instr.result, a() !== b() ? 1 : 0); break;
      case 'LT':     memory.set(instr.result, a() < b() ? 1 : 0); break;
      case 'GT':     memory.set(instr.result, a() > b() ? 1 : 0); break;
      case 'LE':     memory.set(instr.result, a() <= b() ? 1 : 0); break;
      case 'GE':     memory.set(instr.result, a() >= b() ? 1 : 0); break;
      case 'PRINT':  output.push(a()); break;
      case 'NOP':
      default:       break;
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

// ==============================================================================
// Control Flow Graph (CFG) Construction & Dominator Analysis
// ==============================================================================
export function buildCFG(instructions) {
  if (!instructions || instructions.length === 0) return { blocks: [], loops: [] };

  const isLeader = new Array(instructions.length).fill(false);
  isLeader[0] = true;

  instructions.forEach((instr, idx) => {
    if (instr.op === 'LABEL') {
      isLeader[idx] = true;
    }
    if (instr.op === 'GOTO' || instr.op === 'IF_ZERO' || instr.op === 'IF_NOT_ZERO') {
      if (idx + 1 < instructions.length) {
        isLeader[idx + 1] = true;
      }
    }
  });

  const blocks = [];
  const labelToBlock = new Map();
  let currentBlock = { id: 0, label: '', instructions: [], preds: [], succs: [], dominators: [] };

  instructions.forEach((instr, idx) => {
    if (isLeader[idx] && idx > 0) {
      if (currentBlock.instructions.length > 0) {
        blocks.push(currentBlock);
        currentBlock = { id: blocks.length, label: '', instructions: [], preds: [], succs: [], dominators: [] };
      }
    }
    if (instr.op === 'LABEL') {
      currentBlock.label = instr.arg1;
      labelToBlock.set(instr.arg1, currentBlock.id);
    }
    currentBlock.instructions.push(instr);
  });

  if (currentBlock.instructions.length > 0) {
    blocks.push(currentBlock);
  }

  // Connect edges
  blocks.forEach((blk, bIdx) => {
    if (blk.instructions.length === 0) return;
    const last = blk.instructions[blk.instructions.length - 1];

    if (last.op === 'GOTO') {
      const target = labelToBlock.get(last.arg1);
      if (target !== undefined) {
        blk.succs.push(target);
        blocks[target].preds.push(bIdx);
      }
    } else if (last.op === 'IF_ZERO' || last.op === 'IF_NOT_ZERO') {
      const target = labelToBlock.get(last.arg2);
      if (target !== undefined) {
        blk.succs.push(target);
        blocks[target].preds.push(bIdx);
      }
      if (bIdx + 1 < blocks.length) {
        blk.succs.push(bIdx + 1);
        blocks[bIdx + 1].preds.push(bIdx);
      }
    } else {
      if (bIdx + 1 < blocks.length) {
        blk.succs.push(bIdx + 1);
        blocks[bIdx + 1].preds.push(bIdx);
      }
    }
  });

  // Dominator Analysis
  if (blocks.length > 0) {
    const all = blocks.map(b => b.id);
    blocks[0].dominators = [0];
    for (let i = 1; i < blocks.length; i++) {
      blocks[i].dominators = [...all];
    }

    let changed = true;
    while (changed) {
      changed = false;
      for (let i = 1; i < blocks.length; i++) {
        const blk = blocks[i];
        if (blk.preds.length === 0) continue;
        let intersect = [...blocks[blk.preds[0]].dominators];
        for (let p = 1; p < blk.preds.length; p++) {
          const pDom = new Set(blocks[blk.preds[p]].dominators);
          intersect = intersect.filter(d => pDom.has(d));
        }
        intersect.push(i);
        const unique = Array.from(new Set(intersect)).sort((a,b) => a-b);
        if (JSON.stringify(unique) !== JSON.stringify(blk.dominators)) {
          blk.dominators = unique;
          changed = true;
        }
      }
    }
  }

  // Natural Loops
  const loops = [];
  blocks.forEach(blk => {
    blk.succs.forEach(succ => {
      if (blk.dominators.includes(succ)) {
        loops.push({ header: succ, latch: blk.id });
      }
    });
  });

  return { blocks, loops };
}

// ==============================================================================
// Chaitin-Briggs Graph Coloring Register Allocation
// ==============================================================================
export function allocateRegistersChaitin(instructions, k = 6) {
  const physicalRegs = ['rbx', 'r10', 'r11', 'r12', 'r13', 'r14', 'r15', 'rsi', 'rdi'].slice(0, k);

  const isVar = (str) => {
    if (!str || typeof str !== 'string') return false;
    if (/^-?\d+$/.test(str)) return false;
    return true;
  };

  const allVars = new Set();
  const uses = instructions.map(ins => {
    const s = new Set();
    if (ins.op !== 'LABEL' && ins.op !== 'GOTO') {
      if (isVar(ins.arg1)) { s.add(ins.arg1); allVars.add(ins.arg1); }
      if (isVar(ins.arg2)) { s.add(ins.arg2); allVars.add(ins.arg2); }
    }
    return s;
  });

  const defs = instructions.map(ins => {
    const s = new Set();
    if (ins.result && isVar(ins.result)) { s.add(ins.result); allVars.add(ins.result); }
    return s;
  });

  // Backward liveness
  const liveOut = instructions.map(() => new Set());
  const liveIn = instructions.map(() => new Set());
  let changed = true;
  while (changed) {
    changed = false;
    for (let i = instructions.length - 1; i >= 0; i--) {
      const succs = (i + 1 < instructions.length) ? [i + 1] : [];
      const newLiveOut = new Set();
      succs.forEach(s => liveIn[s].forEach(v => newLiveOut.add(v)));

      const newLiveIn = new Set(uses[i]);
      newLiveOut.forEach(v => {
        if (!defs[i].has(v)) newLiveIn.add(v);
      });

      if (newLiveIn.size !== liveIn[i].size || newLiveOut.size !== liveOut[i].size) {
        liveIn[i] = newLiveIn;
        liveOut[i] = newLiveOut;
        changed = true;
      }
    }
  }

  // Interference Graph
  const interference = new Map();
  allVars.forEach(v => interference.set(v, new Set()));

  instructions.forEach((ins, idx) => {
    if (ins.result && isVar(ins.result)) {
      liveOut[idx].forEach(live => {
        if (live !== ins.result) {
          interference.get(ins.result)?.add(live);
          interference.get(live)?.add(ins.result);
        }
      });
    }
  });

  // Simplify & Spill
  const graphCopy = new Map();
  interference.forEach((neighbors, v) => graphCopy.set(v, new Set(neighbors)));

  const selectStack = [];
  while (graphCopy.size > 0) {
    let candidate = null;
    for (const [node, neighbors] of graphCopy.entries()) {
      if (neighbors.size < k) {
        candidate = node;
        break;
      }
    }

    if (candidate) {
      selectStack.push({ varName: candidate, isSpill: false });
      const nbs = graphCopy.get(candidate);
      nbs.forEach(n => graphCopy.get(n)?.delete(candidate));
      graphCopy.delete(candidate);
    } else {
      // Pick highest degree as spill candidate
      let maxDeg = -1;
      let spillNode = null;
      for (const [node, neighbors] of graphCopy.entries()) {
        if (neighbors.size > maxDeg) {
          maxDeg = neighbors.size;
          spillNode = node;
        }
      }
      selectStack.push({ varName: spillNode, isSpill: true });
      const nbs = graphCopy.get(spillNode);
      nbs.forEach(n => graphCopy.get(n)?.delete(spillNode));
      graphCopy.delete(spillNode);
    }
  }

  // Select Colors
  const registerMap = {};
  const stackSpills = {};
  let spillOffset = -8;
  let spillCount = 0;

  while (selectStack.length > 0) {
    const { varName } = selectStack.pop();
    const neighborColors = new Set();
    const neighbors = interference.get(varName) || new Set();
    neighbors.forEach(n => {
      if (registerMap[n]) neighborColors.add(registerMap[n]);
    });

    const freeReg = physicalRegs.find(r => !neighborColors.has(r));
    if (freeReg) {
      registerMap[varName] = freeReg;
    } else {
      stackSpills[varName] = spillOffset;
      spillOffset -= 8;
      spillCount++;
    }
  }

  const stackBytes = Math.ceil((Math.abs(spillOffset) + 32) / 16) * 16;
  return {
    registerMap,
    stackSpills,
    spillCount,
    stackFrameBytes: stackBytes,
    kRegisters: k,
    interferenceList: Array.from(interference.entries()).map(([v, n]) => ({ var: v, neighbors: Array.from(n) }))
  };
}

// ==============================================================================
// Target x86-64 Machine Code Generator (Intel Syntax)
// ==============================================================================
export function generateX86Assembly(instructions, regAlloc) {
  const lines = [];
  lines.push('.intel_syntax noprefix');
  lines.push('.globl main');
  lines.push('.def main; .scl 2; .type 32; .endef');
  lines.push('.text');
  lines.push('main:');
  lines.push('    push rbp');
  lines.push('    mov rbp, rsp');
  const frame = regAlloc?.stackFrameBytes || 48;
  lines.push(`    sub rsp, ${frame}`);

  const getOp = (arg, fallback = 'rax') => {
    if (!arg) return '0';
    if (/^-?\d+$/.test(arg)) return arg;
    if (regAlloc?.registerMap?.[arg]) return regAlloc.registerMap[arg];
    if (regAlloc?.stackSpills?.[arg]) {
      const off = regAlloc.stackSpills[arg];
      lines.push(`    mov ${fallback}, QWORD PTR [rbp - ${Math.abs(off)}]`);
      return fallback;
    }
    return fallback;
  };

  const emitStore = (res, srcReg) => {
    if (!res) return;
    if (regAlloc?.registerMap?.[res]) {
      if (regAlloc.registerMap[res] !== srcReg) {
        lines.push(`    mov ${regAlloc.registerMap[res]}, ${srcReg}`);
      }
      return;
    }
    if (regAlloc?.stackSpills?.[res]) {
      const off = regAlloc.stackSpills[res];
      lines.push(`    mov QWORD PTR [rbp - ${Math.abs(off)}], ${srcReg}`);
    }
  };

  instructions.forEach(ins => {
    switch (ins.op) {
      case 'ASSIGN': {
        const op1 = getOp(ins.arg1, 'rax');
        emitStore(ins.result, op1);
        break;
      }
      case 'ADD': {
        const op1 = getOp(ins.arg1, 'rax');
        const op2 = getOp(ins.arg2, 'rcx');
        lines.push(`    mov rax, ${op1}`);
        lines.push(`    add rax, ${op2}`);
        emitStore(ins.result, 'rax');
        break;
      }
      case 'SUB': {
        const op1 = getOp(ins.arg1, 'rax');
        const op2 = getOp(ins.arg2, 'rcx');
        lines.push(`    mov rax, ${op1}`);
        lines.push(`    sub rax, ${op2}`);
        emitStore(ins.result, 'rax');
        break;
      }
      case 'MUL': {
        const op1 = getOp(ins.arg1, 'rax');
        const op2 = getOp(ins.arg2, 'rcx');
        lines.push(`    mov rax, ${op1}`);
        lines.push(`    imul rax, ${op2}`);
        emitStore(ins.result, 'rax');
        break;
      }
      case 'DIV': {
        const op1 = getOp(ins.arg1, 'rax');
        const op2 = getOp(ins.arg2, 'rcx');
        lines.push(`    mov rax, ${op1}`);
        lines.push('    cqo');
        lines.push(`    mov r11, ${op2}`);
        lines.push('    idiv r11');
        emitStore(ins.result, 'rax');
        break;
      }
      case 'MOD': {
        const op1 = getOp(ins.arg1, 'rax');
        const op2 = getOp(ins.arg2, 'rcx');
        lines.push(`    mov rax, ${op1}`);
        lines.push('    cqo');
        lines.push(`    mov r11, ${op2}`);
        lines.push('    idiv r11');
        emitStore(ins.result, 'rdx');
        break;
      }
      case 'PRINT': {
        const op1 = getOp(ins.arg1, 'rdx');
        lines.push('    sub rsp, 32');
        lines.push('    lea rcx, [rip + fmt_int]');
        lines.push(`    mov rdx, ${op1}`);
        lines.push('    call printf');
        lines.push('    add rsp, 32');
        break;
      }
      case 'LABEL':
        lines.push(`${ins.arg1}:`);
        break;
      case 'GOTO':
        lines.push(`    jmp ${ins.arg1}`);
        break;
      case 'IF_ZERO': {
        const op1 = getOp(ins.arg1, 'rax');
        lines.push(`    mov rax, ${op1}`);
        lines.push('    cmp rax, 0');
        lines.push(`    je ${ins.arg2}`);
        break;
      }
      default:
        break;
    }
  });

  lines.push('    mov eax, 0');
  lines.push('    mov rsp, rbp');
  lines.push('    pop rbp');
  lines.push('    ret');
  lines.push('');
  lines.push('.section .rdata,"dr"');
  lines.push('fmt_int:');
  lines.push('    .asciz "%lld\\n"');

  return lines.join('\n');
}

// Complete Full Pipeline Runner (100% Phase 3)
export async function runFullPipeline(sourceCode) {
  const startTime = performance.now();

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
      cfg: null,
      regAlloc: null,
      x86Assembly: null,
      astImage: nativeResult?.astImage || null,
      nativeStdout: nativeResult?.stdout || null
    };
  }

  const astGraph = buildASTGraph(statements);
  const initialTAC = generateTAC(statements);
  const { finalInstructions, totalIterations, history: passHistory } = runPassManager(initialTAC);
  const diff = computeDiff(initialTAC, finalInstructions);

  // Advanced Phase 3 Backend Passes
  const cfg = buildCFG(finalInstructions);
  const regAlloc = allocateRegistersChaitin(finalInstructions, 6);
  const x86Assembly = generateX86Assembly(finalInstructions, regAlloc);

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
    cfg,
    regAlloc,
    x86Assembly,
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
