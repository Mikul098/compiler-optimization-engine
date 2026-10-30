/**
 * ==============================================================================
 * File: src/loop_opt.h
 * Purpose: Loop-Level Optimizations (Loop Invariant Code Motion & Loop Unrolling).
 *
 * Role in Compiler:
 *  - Analyzes natural loops detected by the CFG.
 *  - Hoists computations invariant across loop iterations into pre-header blocks (LICM).
 *  - Unrolls small bounded loops to eliminate branch penalties and pipeline stalls.
 * ==============================================================================
 */

#pragma once

#include "cfg.h"
#include <vector>
#include <string>
#include <unordered_set>

class LoopOptimizer {
public:
    LoopOptimizer() = default;

    // Executes Loop Invariant Code Motion (LICM) across all natural loops
    bool runLICM(ControlFlowGraph& cfg);

    // Executes Loop Unrolling on small constant-bounded loops
    bool runLoopUnrolling(ControlFlowGraph& cfg, int factor = 2);

    // Master loop optimization orchestrator
    bool optimize(ControlFlowGraph& cfg);
};
