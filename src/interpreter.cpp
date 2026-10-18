/**
 * ==============================================================================
 * File: src/interpreter.cpp
 * Purpose: Implementation of the Interpreter class for TAC instruction execution.
 *
 * Role in Compiler:
 *  - Evaluates TAC instructions sequentially in an emulation memory environment.
 *  - Handles constants and variable lookups via memory map.
 *  - Returns captured print output buffer.
 * ==============================================================================
 */

#if __has_include("src/interpreter.h")
#include "src/interpreter.h"
#else
#include "interpreter.h"
#endif

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

std::vector<int> Interpreter::execute(const std::vector<TACInstruction>& instructions) {
    std::unordered_map<std::string, int> memory;
    std::vector<int> output;

    auto getValue = [&](const std::string& arg) -> int {
        if (arg.empty()) {
            return 0;
        }
        try {
            size_t idx = 0;
            int val = std::stoi(arg, &idx);
            if (idx == arg.length()) {
                return val;
            }
        } catch (...) {
        }
        return memory[arg];
    };

    for (const auto& instr : instructions) {
        switch (instr.op) {
            case OpCode::ASSIGN:
                memory[instr.result] = getValue(instr.arg1);
                break;
            case OpCode::ADD:
                memory[instr.result] = getValue(instr.arg1) + getValue(instr.arg2);
                break;
            case OpCode::SUB:
                memory[instr.result] = getValue(instr.arg1) - getValue(instr.arg2);
                break;
            case OpCode::MUL:
                memory[instr.result] = getValue(instr.arg1) * getValue(instr.arg2);
                break;
            case OpCode::DIV: {
                int divisor = getValue(instr.arg2);
                if (divisor != 0) {
                    memory[instr.result] = getValue(instr.arg1) / divisor;
                } else {
                    std::cerr << "Warning: Division by zero for instruction: " << instr.toString() << "\n";
                    memory[instr.result] = 0;
                }
                break;
            }
            case OpCode::PRINT:
                output.push_back(getValue(instr.arg1));
                break;
            case OpCode::NOP:
                // Do nothing
                break;
            default:
                break;
        }
    }

    return output;
}
