/**
 * ==============================================================================
 * File: src/interpreter.h
 * Purpose: Declaration of the Interpreter class for TAC instruction execution.
 *
 * Role in Compiler:
 *  - Virtual execution environment for Three-Address Code (TAC).
 *  - Maintains runtime variable memory and captures print outputs.
 *  - Allows validation of semantic equivalence between unoptimized and
 *    optimized TAC instruction streams.
 * ==============================================================================
 */

#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#if __has_include("src/tac.h")
#include "src/tac.h"
#else
#include "tac.h"
#endif

class Interpreter {
public:
    Interpreter() = default;

    std::vector<int> execute(const std::vector<TACInstruction>& instructions);
};
