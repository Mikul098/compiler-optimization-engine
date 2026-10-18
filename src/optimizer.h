/**
 * ==============================================================================
 * File: src/optimizer.h
 * Purpose: Optimization framework interfaces and pass declarations.
 *
 * Role in Compiler:
 *  - Defines the OptimizerPass base class for AST/TAC transformation passes.
 *  - Defines optimization passes such as ConstantFoldingPass.
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

class ConstantFoldingPass : public OptimizerPass {
public:
    void optimize(std::vector<TACInstruction>& instructions) override;
};

class DeadCodeEliminationPass : public OptimizerPass {
public:
    void optimize(std::vector<TACInstruction>& instructions) override;
};

class CommonSubexpressionPass : public OptimizerPass {
private:
    std::string getExpressionSignature(OpCode op, const std::string& arg1, const std::string& arg2);

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

