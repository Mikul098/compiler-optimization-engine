/**
 * ==============================================================================
 * File: src/diff.cpp
 * Purpose: Implementation of the DiffEngine class.
 *
 * Role in Compiler:
 *  - Stores pre- and post-optimization TAC instruction vectors.
 *  - Implements printMetrics() to calculate instruction counts, absolute
 *    reductions, and percentage reduction achieved across optimization passes.
 * ==============================================================================
 */

#if __has_include("src/diff.h")
#include "src/diff.h"
#else
#include "diff.h"
#endif

#include <iostream>
#include <iomanip>
#include <algorithm>

const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string RESET = "\033[0m";

void DiffEngine::setBefore(const std::vector<TACInstruction>& before) {
    beforeState = before;
}

void DiffEngine::setAfter(const std::vector<TACInstruction>& after) {
    afterState = after;
}

void DiffEngine::printMetrics() const {
    int before = static_cast<int>(beforeState.size());
    int after = static_cast<int>(afterState.size());
    int reduced = before - after;
    float percentage = 0.0f;
    if (before > 0) {
        percentage = ((before - after) / (float)before) * 100.0f;
    }

    std::cout << "=== OPTIMIZATION METRICS ===\n";
    std::cout << "Initial instruction count: " << before << "\n";
    std::cout << "Final instruction count: " << after << "\n";
    std::cout << "Total instructions reduced: " << reduced << " (" << percentage << "%)\n";
}

void DiffEngine::printSideBySide() const {
    std::cout << std::left << std::setw(40) << "UNOPTIMIZED TAC (Before)" << "OPTIMIZED TAC (After)" << "\n";

    size_t maxSize = std::max(beforeState.size(), afterState.size());
    for (size_t i = 0; i < maxSize; ++i) {
        std::string beforeStr = (i < beforeState.size()) ? beforeState[i].toString() : "";
        std::string afterStr = (i < afterState.size()) ? afterState[i].toString() : "";

        std::cout << RED << std::left << std::setw(40) << beforeStr
                  << GREEN << afterStr
                  << RESET << "\n";
    }
}

