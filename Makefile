# ==============================================================================
# File: Makefile
# Purpose: Build automation and test orchestration for the Toy Compiler project.
#
# Key Responsibilities:
#  - Compiles Bison parser grammar (src/parser.y) into C++ source and header.
#  - Compiles Flex lexer rules (src/lexer.l) into C++ scanner source.
#  - Compiles all C++17 source files (main, ast, tac, parser, lexer).
#  - Links the final executable into bin/compiler (or bin/compiler.exe on Windows).
#  - Provides targets:
#      * 'all'   : Default target, builds the compiler executable.
#      * 'clean' : Removes object files, generated lexer/parser files, and binaries.
#      * 'test'  : Runs the compiler on all test programs in the tests/ directory.
# ==============================================================================

CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -Isrc
BISON = bison
FLEX = flex

SRC_DIR = src
BIN_DIR = bin

PARSER_CPP = $(SRC_DIR)/parser.tab.cpp
PARSER_HPP = $(SRC_DIR)/parser.tab.hpp
LEXER_CPP = $(SRC_DIR)/lexer.yy.cpp

SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/ast.cpp $(SRC_DIR)/tac.cpp $(SRC_DIR)/optimizer.cpp $(PARSER_CPP) $(LEXER_CPP)
OBJS = $(SRCS:.cpp=.o)

TARGET = $(BIN_DIR)/compiler

ifeq ($(OS),Windows_NT)
    TARGET_BIN = $(TARGET).exe
else
    TARGET_BIN = $(TARGET)
endif

.PHONY: all clean test

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJS)
	-mkdir $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PARSER_CPP) $(PARSER_HPP) &: $(SRC_DIR)/parser.y
	$(BISON) -d -o $(PARSER_CPP) $<

$(LEXER_CPP): $(SRC_DIR)/lexer.l $(PARSER_HPP)
	$(FLEX) -o $(LEXER_CPP) $<

clean:
	rm -f $(SRC_DIR)/*.o $(PARSER_CPP) $(PARSER_HPP) $(LEXER_CPP) $(TARGET_BIN)

test: all
	@echo "=== Running Test 1: Basic ==="
	-$(TARGET_BIN) tests/test1_basic.toy
	@echo ""
	@echo "=== Running Test 2: Precedence ==="
	-$(TARGET_BIN) tests/test2_precedence.toy
	@echo ""
	@echo "=== Running Test 3: Syntax Error (Expected to fail) ==="
	-$(TARGET_BIN) tests/test3_syntax_error.toy
	@echo ""
	@echo "=== Running Test 4: Complex TAC ==="
	-$(TARGET_BIN) tests/test4_complex_tac.toy
	@echo ""
	@echo "=== Running Test 5: Subexpressions ==="
	-$(TARGET_BIN) tests/test5_subexpressions.toy
	@echo ""
	@echo "=== Running Test 6: Constant Folding ==="
	-$(TARGET_BIN) tests/test6_const_fold.toy
	@echo ""
	@echo "=== Running Test 7: Dead Code Elimination ==="
	-$(TARGET_BIN) tests/test7_dce.toy


