/**
 * ==============================================================================
 * File: src/codegen_x86.h
 * Purpose: Target Machine Code Generation for x86-64 Native Assembly.
 *
 * Role in Compiler:
 *  - Translates intermediate Three-Address Code (TAC) into valid, compilable x86-64
 *    assembly conforming to System V AMD64 and Microsoft x64 ABIs.
 *  - Incorporates Chaitin-Briggs register allocations and stack spill locations.
 *  - Links with standard C runtime library (libc / MSVCRT) for I/O operations.
 *  - Supports automated assembly & linking into standalone native .exe binaries.
 * ==============================================================================
 */

#pragma once

#include <vector>
#include <string>
#include "tac.h"
#include "regalloc.h"

class X86CodeGenerator {
private:
    std::vector<TACInstruction> instructions;
    RegAllocResult regAlloc;
    std::vector<std::string> asmLines;
    bool isWindows = true;

    std::string getOperand(const std::string& arg, const std::string& fallbackReg);
    void emitStore(const std::string& res, const std::string& srcReg);

public:
    X86CodeGenerator(const std::vector<TACInstruction>& instrs, const RegAllocResult& alloc);

    // Generates complete x86-64 assembly in Intel syntax
    void generate();

    // Writes generated assembly to file (.s)
    bool emitToFile(const std::string& filename) const;

    // Assembles and links assembly into a native executable (.exe) using GCC
    bool assembleAndLink(const std::string& asmFile, const std::string& exeFile) const;

    // Executes the compiled native executable and captures printed integer outputs
    bool runNative(const std::string& exeFile, std::vector<long long>& outputs, double& executionTimeMs) const;

    // Getters
    const std::vector<std::string>& getAssembly() const { return asmLines; }
    void printAssembly() const;
};
