/**
 * ==============================================================================
 * File: src/main.cpp
 * Purpose: Complete End-to-End Compiler Pipeline Orchestrator (100% Phase 3).
 *
 * Pipeline Flow:
 *  1. Frontend: Flex Tokenizer -> Bison LALR(1) Parsing -> AST Hierarchy
 *  2. AST Visualization: Terminal tree & Graphviz high-resolution PNG export
 *  3. Mid-End IR: Three-Address Code (TAC) Quadruple Lowering
 *  4. Mid-End Optimization: 8-Pass Fixed-Point Optimization Loop (PassManager)
 *  5. Control Flow Graph: Basic Block Partitioning & Dominator Tree Analysis
 *  6. Loop Optimization: Loop Invariant Code Motion (LICM) & Natural Loop Detection
 *  7. Backend Analysis: Liveness Dataflow Analysis & Interference Graph
 *  8. Register Allocation: Chaitin-Briggs Graph Coloring Allocator (K Registers + Spilling)
 *  9. Target Codegen: Native x86-64 Assembly Generation (System V & Microsoft x64 ABIs)
 * 10. Native Assembly & Execution: GCC Assembler/Linker invocation & Bare-Metal Benchmarking
 * 11. Dual-State Semantic Verification: Bytecode Virtual Machine asserting identical outputs
 * ==============================================================================
 */

#include <iostream>
#include <memory>
#include <cstdio>
#include <chrono>

#include "ast.h"
#include "tac.h"
#include "optimizer.h"
#include "diff.h"
#include "interpreter.h"
#include "cfg.h"
#include "loop_opt.h"
#include "liveness.h"
#include "regalloc.h"
#include "codegen_x86.h"

extern int yyparse();
extern FILE* yyin;

// Accessible from parser.y
extern std::unique_ptr<ProgramNode> root;

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    // Enable UTF-8 encoding and ANSI escape sequences in Windows console
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
#endif

    if (argc > 1) {
        FILE* file = std::fopen(argv[1], "r");
        if (!file) {
            std::cerr << "Error: Could not open file " << argv[1] << std::endl;
            return 1;
        }
        yyin = file;
        std::cout << "Starting compiler... Reading from " << argv[1] << std::endl;
    } else {
        std::cout << "Starting compiler... Reading from stdin." << std::endl;
    }
    
    if (yyparse() == 0) {
        std::cout << "Parse successful!" << std::endl;
        if (root) {
            std::cout << "\nAbstract Syntax Tree:" << std::endl;
            root->print();
            
            root->exportPNG("ast.png");
            std::cout << "\n\033[1;32m[Visualization]\033[0m High-res AST image saved as \033[1;36mast.png\033[0m!\n" << std::endl;

            // Step 1: Generate Initial Unoptimized TAC
            TACBuilder builder;
            root->generateTAC(builder);
            std::vector<TACInstruction> initialTAC = builder.getInstructions();

            // Step 2: Mid-End 8-Pass Fixed-Point Optimization Pipeline
            PassManager manager;
            manager.addPass(std::make_unique<ConstantPropagationPass>());
            manager.addPass(std::make_unique<ConstantFoldingPass>());
            manager.addPass(std::make_unique<AlgebraicSimplificationPass>());
            manager.addPass(std::make_unique<CopyPropagationPass>());
            manager.addPass(std::make_unique<RedundantAssignmentPass>());
            manager.addPass(std::make_unique<CommonSubexpressionPass>());
            manager.addPass(std::make_unique<DeadCodeEliminationPass>());
            manager.addPass(std::make_unique<PeepholePass>());

            manager.optimize(builder.getInstructions());

            // Step 3: Differential Telemetry
            DiffEngine diff;
            diff.setBefore(initialTAC);
            diff.setAfter(builder.getInstructions());
            diff.printSideBySide();
            diff.printMetrics();

            // Step 4: Control Flow Graph & Dominator Analysis
            std::cout << "\n=== ADVANCED CONTROL FLOW GRAPH & DOMINANCE ===\n";
            ControlFlowGraph cfg;
            cfg.buildFromTAC(builder.getInstructions());
            cfg.printCFG();

            auto naturalLoops = cfg.detectNaturalLoops();
            std::cout << "[CFG] Detected " << naturalLoops.size() << " natural loop(s).\n";

            // Step 5: Loop-Level Optimizations (LICM)
            LoopOptimizer loopOpt;
            if (loopOpt.optimize(cfg)) {
                builder.getInstructions() = cfg.linearize();
                std::cout << "[Optimization] Loop Invariant Code Motion updated the instruction stream.\n";
            }

            // Step 6: Liveness Dataflow Analysis
            std::cout << "\n=== BACKEND LIVENESS & INTERFERENCE GRAPH ===\n";
            LivenessAnalysis liveness;
            liveness.analyze(builder.getInstructions());
            liveness.printLiveness(builder.getInstructions());

            InterferenceGraph ifGraph;
            ifGraph.build(liveness, builder.getInstructions());
            ifGraph.printGraph();

            // Step 7: Chaitin-Briggs Graph Coloring Register Allocation
            ChaitinRegisterAllocator allocator(6); // K = 6 physical registers
            RegAllocResult regAlloc = allocator.allocate(ifGraph);
            allocator.printAllocation(regAlloc);

            // Step 8: Target x86-64 Machine Code Generation
            std::cout << "=== TARGET MACHINE CODE GENERATION (x86-64) ===\n";
            X86CodeGenerator codegen(builder.getInstructions(), regAlloc);
            codegen.generate();
            codegen.printAssembly();

            std::string asmFile = "output.s";
            std::string exeFile = "output.exe";
            codegen.emitToFile(asmFile);
            std::cout << "\033[1;32m[Codegen]\033[0m Wrote native assembly to \033[1;36m" << asmFile << "\033[0m\n";

            // Step 9: Native Compilation & Bare-Metal Execution
            if (codegen.assembleAndLink(asmFile, exeFile)) {
                std::cout << "\033[1;32m[Build]\033[0m Successfully assembled and linked \033[1;36m" << exeFile << "\033[0m via GCC.\n";

                std::vector<long long> nativeOut;
                double execTimeMs = 0.0;
                if (codegen.runNative(exeFile, nativeOut, execTimeMs)) {
                    std::cout << "\033[1;32m[Execution]\033[0m Native bare-metal execution completed in " << execTimeMs << " ms.\n";
                    std::cout << "Native Program Output: ";
                    for (auto val : nativeOut) {
                        std::cout << val << " ";
                    }
                    std::cout << "\n";
                }
            } else {
                std::cout << "\033[1;33m[Notice]\033[0m Native GCC assembly step skipped or compiler in cross-mode.\n";
            }

            // Step 10: Virtual Machine Semantic Verification
            std::cout << "\n=== SEMANTIC VERIFICATION (VM & BARE-METAL) ===\n";
            Interpreter vm;
            std::vector<int> unoptimizedOutput = vm.execute(initialTAC);
            std::vector<int> optimizedOutput = vm.execute(builder.getInstructions());

            if (unoptimizedOutput == optimizedOutput) {
                std::cout << "\033[32m[PASS] Optimized code produces identical output.\033[0m\n";
                for (int val : optimizedOutput) {
                    std::cout << "Program Output: " << val << "\n";
                }
            } else {
                std::cout << "\033[31m[FAIL] Semantic mismatch detected! Optimization broke the logic.\033[0m\n";
            }
        }
    } else {
        std::cerr << "Parse failed!" << std::endl;
        return 1;
    }
    
    return 0;
}
