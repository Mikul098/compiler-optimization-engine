/**
 * ==============================================================================
 * File: src/optimizer.h
 * Purpose: Optimization framework interfaces and pass declarations.
 *
 * Role in Compiler:
 *  - Defines the OptimizerPass base class for AST/TAC transformation passes.
 *  - Optimization passes (8 total):
 *      1. ConstantFoldingPass       - Fold compile-time-known constant expressions
 *      2. DeadCodeEliminationPass   - Remove assignments whose result is never used
 *      3. CommonSubexpressionPass   - Replace duplicate computations with a copy
 *      4. AlgebraicSimplificationPass - Apply algebraic identities & strength reduction
 *      5. CopyPropagationPass       - Substitute copies (t=a) to eliminate temporaries
 *      6. RedundantAssignmentPass   - Delete x=x no-op assignments
 *      7. ConstantPropagationPass   - Forward-propagate known constants across blocks
 *      8. PeepholePass              - Local 2-instruction window micro-optimizations
 * ==============================================================================
 */

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include "tac.h"

class OptimizerPass {
public:
    virtual ~OptimizerPass() = default;
    virtual void optimize(std::vector<TACInstruction>& instructions) = 0;
};

// Pass 1: Folds arithmetic/logical/bitwise/relational expressions with constant operands.
class ConstantFoldingPass : public OptimizerPass {
public:
    void optimize(std::vector<TACInstruction>& instructions) override;
};

// Pass 2: Backward liveness analysis to eliminate dead assignments.
class DeadCodeEliminationPass : public OptimizerPass {
public:
    void optimize(std::vector<TACInstruction>& instructions) override;
};

// Pass 3: Detects repeated computations and replaces with a single copy.
class CommonSubexpressionPass : public OptimizerPass {
private:
    std::string getExpressionSignature(OpCode op, const std::string& arg1, const std::string& arg2);
public:
    void optimize(std::vector<TACInstruction>& instructions) override;
};

// Pass 4: Algebraic identities and strength reduction.
//   e.g. x*0->0, x*1->x, x+0->x, x-0->x, x/1->x, x*2->x<<1,
//        x^x->0, x-x->0, x|0->x, x&0->0, x|x->x, x&x->x
class AlgebraicSimplificationPass : public OptimizerPass {
public:
    void optimize(std::vector<TACInstruction>& instructions) override;
};

// Pass 5: Substitutes copy-propagated variables to reduce temporary usage.
//   After  t1 = a,  replace all downstream uses of t1 with a.
class CopyPropagationPass : public OptimizerPass {
public:
    void optimize(std::vector<TACInstruction>& instructions) override;
};

// Pass 6: Removes redundant self-assignments of the form  x = x.
class RedundantAssignmentPass : public OptimizerPass {
public:
    void optimize(std::vector<TACInstruction>& instructions) override;
};

// Pass 7: Forward constant propagation across basic block boundaries.
//   Tracks variables set to a constant and replaces their uses globally.
class ConstantPropagationPass : public OptimizerPass {
public:
    void optimize(std::vector<TACInstruction>& instructions) override;
};

// Pass 8: Local peephole optimizations on a sliding 2-instruction window.
//   e.g. collapses t2 = t1 when t1 is used exactly once immediately after.
class PeepholePass : public OptimizerPass {
public:
    void optimize(std::vector<TACInstruction>& instructions) override;
};

class PassManager {
private:
    std::vector<std::unique_ptr<OptimizerPass>> passes;

public:
    void addPass(std::unique_ptr<OptimizerPass> pass);
    void optimize(std::vector<TACInstruction>& instructions);
};

bool isNumber(const std::string& str);

