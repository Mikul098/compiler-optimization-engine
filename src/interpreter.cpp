/**
 * ==============================================================================
 * File: src/interpreter.cpp
 * Purpose: Implementation of the Interpreter class for TAC instruction execution.
 *
 * Capabilities:
 *  - Supports straight-line arithmetic, assignments, prints.
 *  - Supports relational comparisons (==, !=, <, >, <=, >=).
 *  - Supports branch control flow (LABEL, GOTO, IF_ZERO, IF_NOT_ZERO).
 *  - Safeguarded against infinite loops via max execution cycle count.
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

    // 1. Build label lookup map
    std::unordered_map<std::string, size_t> labelMap;
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (instructions[i].op == OpCode::LABEL) {
            labelMap[instructions[i].arg1] = i;
        }
    }

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

    size_t pc = 0;
    int stepCount = 0;
    const int MAX_STEPS = 2000000; // 2 million step ceiling

    while (pc < instructions.size() && stepCount++ < MAX_STEPS) {
        const auto& instr = instructions[pc];

        switch (instr.op) {
            case OpCode::LABEL:
                pc++;
                break;

            case OpCode::GOTO:
                if (labelMap.find(instr.arg1) != labelMap.end()) {
                    pc = labelMap[instr.arg1];
                } else {
                    pc++;
                }
                break;

            case OpCode::IF_ZERO:
                if (getValue(instr.arg1) == 0) {
                    if (labelMap.find(instr.arg2) != labelMap.end()) {
                        pc = labelMap[instr.arg2];
                    } else {
                        pc++;
                    }
                } else {
                    pc++;
                }
                break;

            case OpCode::IF_NOT_ZERO:
                if (getValue(instr.arg1) != 0) {
                    if (labelMap.find(instr.arg2) != labelMap.end()) {
                        pc = labelMap[instr.arg2];
                    } else {
                        pc++;
                    }
                } else {
                    pc++;
                }
                break;

            case OpCode::EQ:
                memory[instr.result] = (getValue(instr.arg1) == getValue(instr.arg2)) ? 1 : 0;
                pc++;
                break;

            case OpCode::NE:
                memory[instr.result] = (getValue(instr.arg1) != getValue(instr.arg2)) ? 1 : 0;
                pc++;
                break;

            case OpCode::LT:
                memory[instr.result] = (getValue(instr.arg1) < getValue(instr.arg2)) ? 1 : 0;
                pc++;
                break;

            case OpCode::GT:
                memory[instr.result] = (getValue(instr.arg1) > getValue(instr.arg2)) ? 1 : 0;
                pc++;
                break;

            case OpCode::LE:
                memory[instr.result] = (getValue(instr.arg1) <= getValue(instr.arg2)) ? 1 : 0;
                pc++;
                break;

            case OpCode::GE:
                memory[instr.result] = (getValue(instr.arg1) >= getValue(instr.arg2)) ? 1 : 0;
                pc++;
                break;

            case OpCode::ASSIGN:
                memory[instr.result] = getValue(instr.arg1);
                pc++;
                break;

            case OpCode::ADD:
                memory[instr.result] = getValue(instr.arg1) + getValue(instr.arg2);
                pc++;
                break;

            case OpCode::SUB:
                memory[instr.result] = getValue(instr.arg1) - getValue(instr.arg2);
                pc++;
                break;

            case OpCode::MUL:
                memory[instr.result] = getValue(instr.arg1) * getValue(instr.arg2);
                pc++;
                break;

            case OpCode::DIV: {
                int divisor = getValue(instr.arg2);
                if (divisor != 0) {
                    memory[instr.result] = getValue(instr.arg1) / divisor;
                } else {
                    memory[instr.result] = 0;
                }
                pc++;
                break;
            }

            case OpCode::MOD: {
                int divisor = getValue(instr.arg2);
                if (divisor != 0) {
                    memory[instr.result] = getValue(instr.arg1) % divisor;
                } else {
                    memory[instr.result] = 0;
                }
                pc++;
                break;
            }

            case OpCode::AND:
                memory[instr.result] = (getValue(instr.arg1) != 0 && getValue(instr.arg2) != 0) ? 1 : 0;
                pc++;
                break;

            case OpCode::OR:
                memory[instr.result] = (getValue(instr.arg1) != 0 || getValue(instr.arg2) != 0) ? 1 : 0;
                pc++;
                break;

            case OpCode::BAND:
                memory[instr.result] = getValue(instr.arg1) & getValue(instr.arg2);
                pc++;
                break;

            case OpCode::BOR:
                memory[instr.result] = getValue(instr.arg1) | getValue(instr.arg2);
                pc++;
                break;

            case OpCode::BXOR:
                memory[instr.result] = getValue(instr.arg1) ^ getValue(instr.arg2);
                pc++;
                break;

            case OpCode::BNOT:
                memory[instr.result] = ~getValue(instr.arg1);
                pc++;
                break;

            case OpCode::SHL:
                memory[instr.result] = getValue(instr.arg1) << getValue(instr.arg2);
                pc++;
                break;

            case OpCode::SHR:
                memory[instr.result] = getValue(instr.arg1) >> getValue(instr.arg2);
                pc++;
                break;

            case OpCode::PRINT:
                output.push_back(getValue(instr.arg1));
                pc++;
                break;

            case OpCode::NOP:
                pc++;
                break;

            default:
                pc++;
                break;
        }
    }

    if (stepCount >= MAX_STEPS) {
        std::cerr << "Warning: Interpreter hit execution step ceiling (" << MAX_STEPS << " steps).\n";
    }

    return output;
}
