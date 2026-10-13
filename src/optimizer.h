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

bool isNumber(const std::string& str);

