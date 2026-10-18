/**
 * ==============================================================================
 * File: src/diff.h
 * Purpose: Declaration of the DiffEngine class for TAC optimization metrics.
 *
 * Role in Compiler:
 *  - Stores pre-optimization and post-optimization TAC instruction states.
 *  - Calculates and reports optimization effectiveness metrics.
 * ==============================================================================
 */

#pragma once

#include <vector>
#include <string>
#include <iostream>
#if __has_include("src/tac.h")
#include "src/tac.h"
#else
#include "tac.h"
#endif

class DiffEngine {
private:
    std::vector<TACInstruction> beforeState;
    std::vector<TACInstruction> afterState;

public:
    DiffEngine() = default;

    void setBefore(const std::vector<TACInstruction>& before);
    void setAfter(const std::vector<TACInstruction>& after);
    void printMetrics() const;
    void printSideBySide() const;

    const std::vector<TACInstruction>& getBefore() const { return beforeState; }
    const std::vector<TACInstruction>& getAfter() const { return afterState; }
};
