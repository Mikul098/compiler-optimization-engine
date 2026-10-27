/**
 * ==============================================================================
 * File: src/main.cpp
 * Purpose: Main driver and pipeline orchestrator for the Toy Compiler.
 *
 * Role in Compiler:
 *  - Serves as the primary executable entry point:
 *      1. Command-Line Processing: Opens the input .toy file provided as an argument.
 *      2. Terminal Setup: Enables UTF-8 and ANSI escape sequences on Windows consoles.
 *      3. Parsing (Flex/Bison): Calls yyparse() to construct the Abstract Syntax Tree.
 *      4. AST Printing: Displays the hierarchical tree with Unicode branches in terminal.
 *      5. Web & Image Export: Writes ast.html and ast.json, invoking render_ast.py.
 *      6. TAC Generation: Runs post-order traversal on the AST to emit and print
 *         unoptimized Three-Address Code (TAC) intermediate representation.
 * ==============================================================================
 */

#include <iostream>
#include <memory>
#include <cstdio>
#include "ast.h"
#include "tac.h"
#include "optimizer.h"
#include "diff.h"
#include "interpreter.h"

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

            TACBuilder builder;
            root->generateTAC(builder);

            std::vector<TACInstruction> initialTAC = builder.getInstructions();

            PassManager manager;
            // Pass 7: Constant Propagation - forward-propagate consts to expose more folding
            manager.addPass(std::make_unique<ConstantPropagationPass>());
            // Pass 1: Constant Folding - fold fully-constant expressions
            manager.addPass(std::make_unique<ConstantFoldingPass>());
            // Pass 4: Algebraic Simplification & Strength Reduction
            manager.addPass(std::make_unique<AlgebraicSimplificationPass>());
            // Pass 5: Copy Propagation - eliminate unnecessary temporaries
            manager.addPass(std::make_unique<CopyPropagationPass>());
            // Pass 6: Redundant Assignment Elimination - remove x=x
            manager.addPass(std::make_unique<RedundantAssignmentPass>());
            // Pass 3: Common Subexpression Elimination
            manager.addPass(std::make_unique<CommonSubexpressionPass>());
            // Pass 2: Dead Code Elimination - backward liveness analysis
            manager.addPass(std::make_unique<DeadCodeEliminationPass>());
            // Pass 8: Peephole - local 2-instruction window micro-optimizations
            manager.addPass(std::make_unique<PeepholePass>());

            manager.optimize(builder.getInstructions());

            DiffEngine diff;
            diff.setBefore(initialTAC);
            diff.setAfter(builder.getInstructions());

            diff.printSideBySide();
            diff.printMetrics();

            std::cout << "\n=== SEMANTIC VERIFICATION ===\n";
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
