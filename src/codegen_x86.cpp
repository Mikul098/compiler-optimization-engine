/**
 * ==============================================================================
 * File: src/codegen_x86.cpp
 * Purpose: Implementation of x86-64 Target Machine Code Generator.
 * ==============================================================================
 */

#include "codegen_x86.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <chrono>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#endif

static bool isNumericLiteral(const std::string& str) {
    if (str.empty()) return false;
    size_t i = 0;
    if (str[0] == '-') {
        if (str.length() == 1) return false;
        i = 1;
    }
    for (; i < str.length(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(str[i]))) return false;
    }
    return true;
}

X86CodeGenerator::X86CodeGenerator(const std::vector<TACInstruction>& instrs, const RegAllocResult& alloc)
    : instructions(instrs), regAlloc(alloc) {
#ifdef _WIN32
    isWindows = true;
#else
    isWindows = false;
#endif
}

std::string X86CodeGenerator::getOperand(const std::string& arg, const std::string& fallbackReg) {
    if (arg.empty()) return "0";

    // Immediate constant
    if (isNumericLiteral(arg)) {
        return arg;
    }

    // Allocated to physical register
    auto itReg = regAlloc.registerMap.find(arg);
    if (itReg != regAlloc.registerMap.end()) {
        return itReg->second;
    }

    // Spilled to stack frame
    auto itSpill = regAlloc.stackSpills.find(arg);
    if (itSpill != regAlloc.stackSpills.end()) {
        int offset = itSpill->second;
        std::stringstream ss;
        if (offset < 0) {
            ss << "QWORD PTR [rbp - " << (-offset) << "]";
        } else {
            ss << "QWORD PTR [rbp + " << offset << "]";
        }
        asmLines.push_back("    mov " + fallbackReg + ", " + ss.str());
        return fallbackReg;
    }

    return fallbackReg;
}

void X86CodeGenerator::emitStore(const std::string& res, const std::string& srcReg) {
    if (res.empty()) return;

    auto itReg = regAlloc.registerMap.find(res);
    if (itReg != regAlloc.registerMap.end()) {
        if (itReg->second != srcReg) {
            asmLines.push_back("    mov " + itReg->second + ", " + srcReg);
        }
        return;
    }

    auto itSpill = regAlloc.stackSpills.find(res);
    if (itSpill != regAlloc.stackSpills.end()) {
        int offset = itSpill->second;
        std::stringstream ss;
        if (offset < 0) {
            ss << "QWORD PTR [rbp - " << (-offset) << "]";
        } else {
            ss << "QWORD PTR [rbp + " << offset << "]";
        }
        asmLines.push_back("    mov " + ss.str() + ", " + srcReg);
    }
}

void X86CodeGenerator::generate() {
    asmLines.clear();

    // Assembly Header
    asmLines.push_back(".intel_syntax noprefix");
    asmLines.push_back(".globl main");
    if (isWindows) {
        asmLines.push_back(".def main; .scl 2; .type 32; .endef");
    }
    asmLines.push_back(".text");
    asmLines.push_back("main:");

    // Function Prologue
    asmLines.push_back("    push rbp");
    asmLines.push_back("    mov rbp, rsp");

    int frameSize = regAlloc.stackFrameBytes > 0 ? regAlloc.stackFrameBytes : 32;
    // Ensure 16-byte stack frame alignment
    frameSize = ((frameSize + 15) / 16) * 16;
    asmLines.push_back("    sub rsp, " + std::to_string(frameSize));

    // Save callee-saved registers that were allocated
    std::vector<std::string> calleeSaved = {"rbx", "r12", "r13", "r14", "r15", "rsi", "rdi"};
    for (const auto& reg : calleeSaved) {
        bool used = false;
        for (const auto& [var, r] : regAlloc.registerMap) {
            if (r == reg) { used = true; break; }
        }
        if (used) {
            asmLines.push_back("    push " + reg);
        }
    }

    asmLines.push_back("");

    // Instruction Body Generation
    for (const auto& instr : instructions) {
        switch (instr.op) {
            case OpCode::ASSIGN: {
                std::string op1 = getOperand(instr.arg1, "rax");
                emitStore(instr.result, op1);
                break;
            }

            case OpCode::ADD: {
                std::string op1 = getOperand(instr.arg1, "rax");
                std::string op2 = getOperand(instr.arg2, "rcx");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    add rax, " + op2);
                emitStore(instr.result, "rax");
                break;
            }

            case OpCode::SUB: {
                std::string op1 = getOperand(instr.arg1, "rax");
                std::string op2 = getOperand(instr.arg2, "rcx");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    sub rax, " + op2);
                emitStore(instr.result, "rax");
                break;
            }

            case OpCode::MUL: {
                std::string op1 = getOperand(instr.arg1, "rax");
                std::string op2 = getOperand(instr.arg2, "rcx");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    imul rax, " + op2);
                emitStore(instr.result, "rax");
                break;
            }

            case OpCode::DIV: {
                std::string op1 = getOperand(instr.arg1, "rax");
                std::string op2 = getOperand(instr.arg2, "rcx");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    cqo");
                asmLines.push_back("    mov r11, " + op2);
                asmLines.push_back("    idiv r11");
                emitStore(instr.result, "rax");
                break;
            }

            case OpCode::MOD: {
                std::string op1 = getOperand(instr.arg1, "rax");
                std::string op2 = getOperand(instr.arg2, "rcx");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    cqo");
                asmLines.push_back("    mov r11, " + op2);
                asmLines.push_back("    idiv r11");
                emitStore(instr.result, "rdx");
                break;
            }

            case OpCode::BAND: {
                std::string op1 = getOperand(instr.arg1, "rax");
                std::string op2 = getOperand(instr.arg2, "rcx");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    and rax, " + op2);
                emitStore(instr.result, "rax");
                break;
            }

            case OpCode::BOR: {
                std::string op1 = getOperand(instr.arg1, "rax");
                std::string op2 = getOperand(instr.arg2, "rcx");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    or rax, " + op2);
                emitStore(instr.result, "rax");
                break;
            }

            case OpCode::BXOR: {
                std::string op1 = getOperand(instr.arg1, "rax");
                std::string op2 = getOperand(instr.arg2, "rcx");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    xor rax, " + op2);
                emitStore(instr.result, "rax");
                break;
            }

            case OpCode::SHL: {
                std::string op1 = getOperand(instr.arg1, "rax");
                std::string op2 = getOperand(instr.arg2, "rcx");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    mov rcx, " + op2);
                asmLines.push_back("    shl rax, cl");
                emitStore(instr.result, "rax");
                break;
            }

            case OpCode::SHR: {
                std::string op1 = getOperand(instr.arg1, "rax");
                std::string op2 = getOperand(instr.arg2, "rcx");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    mov rcx, " + op2);
                asmLines.push_back("    sar rax, cl");
                emitStore(instr.result, "rax");
                break;
            }

            case OpCode::BNOT: {
                std::string op1 = getOperand(instr.arg1, "rax");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    not rax");
                emitStore(instr.result, "rax");
                break;
            }

            case OpCode::EQ:
            case OpCode::NE:
            case OpCode::LT:
            case OpCode::GT:
            case OpCode::LE:
            case OpCode::GE: {
                std::string op1 = getOperand(instr.arg1, "rax");
                std::string op2 = getOperand(instr.arg2, "rcx");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    cmp rax, " + op2);
                std::string setcc = "sete";
                if (instr.op == OpCode::NE) setcc = "setne";
                else if (instr.op == OpCode::LT) setcc = "setl";
                else if (instr.op == OpCode::GT) setcc = "setg";
                else if (instr.op == OpCode::LE) setcc = "setle";
                else if (instr.op == OpCode::GE) setcc = "setge";

                asmLines.push_back("    " + setcc + " al");
                asmLines.push_back("    movzx rax, al");
                emitStore(instr.result, "rax");
                break;
            }

            case OpCode::LABEL: {
                asmLines.push_back(instr.arg1 + ":");
                break;
            }

            case OpCode::GOTO: {
                asmLines.push_back("    jmp " + instr.arg1);
                break;
            }

            case OpCode::IF_ZERO: {
                std::string op1 = getOperand(instr.arg1, "rax");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    cmp rax, 0");
                asmLines.push_back("    je " + instr.arg2);
                break;
            }

            case OpCode::IF_NOT_ZERO: {
                std::string op1 = getOperand(instr.arg1, "rax");
                asmLines.push_back("    mov rax, " + op1);
                asmLines.push_back("    cmp rax, 0");
                asmLines.push_back("    jne " + instr.arg2);
                break;
            }

            case OpCode::PRINT: {
                std::string op1 = getOperand(instr.arg1, "rdx");
                if (isWindows) {
                    asmLines.push_back("    sub rsp, 32"); // Shadow store
                    asmLines.push_back("    lea rcx, [rip + fmt_int]");
                    asmLines.push_back("    mov rdx, " + op1);
                    asmLines.push_back("    call printf");
                    asmLines.push_back("    add rsp, 32");
                } else {
                    asmLines.push_back("    lea rdi, [rip + fmt_int]");
                    asmLines.push_back("    mov rsi, " + op1);
                    asmLines.push_back("    xor al, al");
                    asmLines.push_back("    call printf");
                }
                break;
            }

            default:
                break;
        }
    }

    asmLines.push_back("");

    // Restore callee-saved registers in reverse order
    for (auto it = calleeSaved.rbegin(); it != calleeSaved.rend(); ++it) {
        bool used = false;
        for (const auto& [var, r] : regAlloc.registerMap) {
            if (r == *it) { used = true; break; }
        }
        if (used) {
            asmLines.push_back("    pop " + *it);
        }
    }

    // Function Epilogue
    asmLines.push_back("    mov eax, 0");
    asmLines.push_back("    mov rsp, rbp");
    asmLines.push_back("    pop rbp");
    asmLines.push_back("    ret");

    // Read-only Data Section
    asmLines.push_back("");
    asmLines.push_back(".section .rdata,\"dr\"");
    asmLines.push_back("fmt_int:");
    asmLines.push_back("    .asciz \"%lld\\n\"");
}

bool X86CodeGenerator::emitToFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    for (const auto& line : asmLines) {
        out << line << "\n";
    }
    return true;
}

bool X86CodeGenerator::assembleAndLink(const std::string& asmFile, const std::string& exeFile) const {
    std::string cmd = "gcc " + asmFile + " -o " + exeFile + " -no-pie 2>&1";
    int ret = std::system(cmd.c_str());
    return (ret == 0);
}

bool X86CodeGenerator::runNative(const std::string& exeFile, std::vector<long long>& outputs, double& executionTimeMs) const {
    outputs.clear();

    auto startTime = std::chrono::high_resolution_clock::now();

#ifdef _WIN32
    FILE* pipe = _popen(exeFile.c_str(), "r");
#else
    FILE* pipe = popen(exeFile.c_str(), "r");
#endif

    if (!pipe) return false;

    char buffer[256];
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        try {
            long long val = std::stoll(buffer);
            outputs.push_back(val);
        } catch (...) {}
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = endTime - startTime;
    executionTimeMs = duration.count();

    return true;
}

void X86CodeGenerator::printAssembly() const {
    std::cout << "=== GENERATED x86-64 NATIVE ASSEMBLY ===\n";
    for (const auto& line : asmLines) {
        std::cout << line << "\n";
    }
    std::cout << "========================================\n";
}
