/**
 * ==============================================================================
 * File: src/ast.cpp
 * Purpose: Implementation of AST methods, visualizations, and TAC generation.
 *
 * Role in Compiler:
 *  - Implements polymorphic operations for AST nodes:
 *      1. Terminal Tree Visualization (print):
 *         Recursively prints the syntax tree in the terminal with ANSI colors
 *         and branch box-drawing characters.
 *      2. HTML Tree Serialization (toHtml, exportToHtml):
 *         Renders an HTML-based interactive visual representation of the AST.
 *      3. JSON Tree Serialization (toJson, exportToJson):
 *         Dumps the node hierarchy to JSON for graphical rendering.
 *      4. Intermediate Code Generation (generateTAC):
 *         Executes post-order traversal to flatten complex nested arithmetic
 *         expressions into linear Three-Address Code (TAC) instructions.
 * ==============================================================================
 */

#include "ast.h"
#include <fstream>
#include <cstdlib>

// ==========================================
// Terminal Tree Visualization Implementation
// ==========================================

void ProgramNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    (void)prefix; (void)isLast; (void)label;
    std::cout << "\033[1;35mProgram (Root)\033[0m" << std::endl;
    for (size_t i = 0; i < statements.size(); ++i) {
        bool last = (i == statements.size() - 1);
        statements[i]->print("", last, "");
    }
}

void AssignmentNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;34mAssignment:\033[0m " << varName << " =" << std::endl;
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (expression) {
        expression->print(childPrefix, true, "Value:");
    }
}

void PrintNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;32mPrint\033[0m" << std::endl;
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (expression) {
        expression->print(childPrefix, true, "Arg:");
    }
}

void BinaryOpNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::string opStr;
    switch (op) {
        case BinaryOp::ADD: opStr = "+ (Add)"; break;
        case BinaryOp::SUB: opStr = "- (Subtract)"; break;
        case BinaryOp::MUL: opStr = "* (Multiply)"; break;
        case BinaryOp::DIV: opStr = "/ (Divide)"; break;
    }
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;33mBinaryOp:\033[0m " << opStr << std::endl;
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (left && right) {
        left->print(childPrefix, false, "[Left]");
        right->print(childPrefix, true, "[Right]");
    } else if (left) {
        left->print(childPrefix, true, "[Left]");
    } else if (right) {
        right->print(childPrefix, true, "[Right]");
    }
}

void VariableNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;36mVariable:\033[0m " << name << std::endl;
}

void NumberNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;31mNumber:\033[0m " << value << std::endl;
}

// ==========================================
// HTML Visualizer Implementation
// ==========================================

void ProgramNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-root\">ROOT</span><div class=\"val\">Program</div></div>\n";
    if (!statements.empty()) {
        out << "<ul>\n";
        for (const auto& stmt : statements) {
            stmt->toHtml(out);
        }
        out << "</ul>\n";
    }
    out << "</li>\n";
}

void AssignmentNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-assign\">ASSIGNMENT</span><div class=\"val\">" << varName << " =</div></div>\n";
    if (expression) {
        out << "<ul>\n";
        expression->toHtml(out);
        out << "</ul>\n";
    }
    out << "</li>\n";
}

void PrintNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-print\">PRINT</span><div class=\"val\">print()</div></div>\n";
    if (expression) {
        out << "<ul>\n";
        expression->toHtml(out);
        out << "</ul>\n";
    }
    out << "</li>\n";
}

void BinaryOpNode::toHtml(std::ostream& out) const {
    std::string opSymbol;
    switch (op) {
        case BinaryOp::ADD: opSymbol = "+"; break;
        case BinaryOp::SUB: opSymbol = "-"; break;
        case BinaryOp::MUL: opSymbol = "×"; break;
        case BinaryOp::DIV: opSymbol = "÷"; break;
    }
    out << "<li><div class=\"card\"><span class=\"badge badge-op\">OPERATOR</span><div class=\"val\">" << opSymbol << "</div></div>\n";
    if (left || right) {
        out << "<ul>\n";
        if (left) left->toHtml(out);
        if (right) right->toHtml(out);
        out << "</ul>\n";
    }
    out << "</li>\n";
}

void VariableNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-var\">VARIABLE</span><div class=\"val\">" << name << "</div></div></li>\n";
}

void NumberNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-num\">NUMBER</span><div class=\"val\">" << value << "</div></div></li>\n";
}

void ProgramNode::exportHTML(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    file << "<!DOCTYPE html>\n"
         << "<html lang=\"en\">\n<head>\n"
         << "<meta charset=\"UTF-8\">\n"
         << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
         << "<title>AST Visualizer</title>\n"
         << "<style>\n"
         << "  * { box-sizing: border-box; margin: 0; padding: 0; }\n"
         << "  body { background: #0f172a; color: #f8fafc; font-family: system-ui, -apple-system, sans-serif; padding: 2rem; overflow-x: auto; }\n"
         << "  header { text-align: center; margin-bottom: 2.5rem; }\n"
         << "  h1 { font-size: 2rem; color: #38bdf8; margin-bottom: 0.5rem; font-weight: 700; letter-spacing: -0.5px; }\n"
         << "  p.sub { color: #94a3b8; font-size: 0.95rem; }\n"
         << "  .tree { display: flex; justify-content: center; min-width: fit-content; padding-bottom: 4rem; }\n"
         << "  .tree ul { padding-top: 24px; position: relative; display: flex; justify-content: center; list-style: none; }\n"
         << "  .tree li { position: relative; padding: 24px 12px 0 12px; display: flex; flex-direction: column; align-items: center; }\n"
         << "  .tree li::before, .tree li::after { content: ''; position: absolute; top: 0; right: 50%; border-top: 2px solid #334155; width: 50%; height: 24px; }\n"
         << "  .tree li::after { right: auto; left: 50%; border-left: 2px solid #334155; }\n"
         << "  .tree li:only-child::after, .tree li:only-child::before { display: none; }\n"
         << "  .tree li:only-child { padding-top: 0; }\n"
         << "  .tree li:first-child::before, .tree li:last-child::after { border: 0 none; }\n"
         << "  .tree li:last-child::before { border-right: 2px solid #334155; border-radius: 0 6px 0 0; }\n"
         << "  .tree li:first-child::after { border-radius: 6px 0 0 0; }\n"
         << "  .tree ul ul::before { content: ''; position: absolute; top: 0; left: 50%; border-left: 2px solid #334155; width: 0; height: 24px; transform: translateX(-50%); }\n"
         << "  .card { background: #1e293b; border: 1.5px solid #334155; padding: 10px 18px; border-radius: 12px; text-align: center; box-shadow: 0 10px 20px -5px rgba(0,0,0,0.5); min-width: 90px; transition: all 0.2s ease; }\n"
         << "  .card:hover { transform: translateY(-4px) scale(1.03); border-color: #38bdf8; box-shadow: 0 16px 28px -5px rgba(56,189,248,0.25); }\n"
         << "  .badge { display: block; font-size: 0.65rem; font-weight: 700; text-transform: uppercase; padding: 2px 6px; border-radius: 6px; margin-bottom: 6px; letter-spacing: 0.5px; }\n"
         << "  .badge-root { background: #4f46e5; color: #fff; }\n"
         << "  .badge-assign { background: #0284c7; color: #fff; }\n"
         << "  .badge-print { background: #059669; color: #fff; }\n"
         << "  .badge-op { background: #d97706; color: #fff; }\n"
         << "  .badge-var { background: #0891b2; color: #fff; }\n"
         << "  .badge-num { background: #e11d48; color: #fff; }\n"
         << "  .val { font-size: 1.05rem; font-weight: 700; color: #f1f5f9; }\n"
         << "</style>\n</head>\n<body>\n"
         << "<header>\n"
         << "  <h1>Abstract Syntax Tree Visualizer</h1>\n"
         << "  <p class=\"sub\">Hierarchical visualization generated by compiler frontend</p>\n"
         << "</header>\n"
         << "<div class=\"tree\">\n<ul>\n";

    toHtml(file);

    file << "</ul>\n</div>\n</body>\n</html>\n";
    file.close();
}

// ==========================================
// PNG / JSON Visualizer Implementation
// ==========================================

void ProgramNode::toJson(std::ostream& out, const std::string& edge) const {
    (void)edge;
    out << "{\"name\": \"Program\", \"type\": \"root\", \"children\": [";
    for (size_t i = 0; i < statements.size(); ++i) {
        if (i > 0) out << ", ";
        statements[i]->toJson(out);
    }
    out << "]}";
}

void AssignmentNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Assign: " << varName << "\", \"type\": \"assign\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    if (expression) {
        out << ", \"children\": [";
        expression->toJson(out);
        out << "]";
    }
    out << "}";
}

void PrintNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Print\", \"type\": \"print\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    if (expression) {
        out << ", \"children\": [";
        expression->toJson(out);
        out << "]";
    }
    out << "}";
}

void BinaryOpNode::toJson(std::ostream& out, const std::string& edge) const {
    std::string opStr;
    switch (op) {
        case BinaryOp::ADD: opStr = "+ (Add)"; break;
        case BinaryOp::SUB: opStr = "- (Sub)"; break;
        case BinaryOp::MUL: opStr = "* (Mul)"; break;
        case BinaryOp::DIV: opStr = "/ (Div)"; break;
    }
    out << "{\"name\": \"Op: " << opStr << "\", \"type\": \"op\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << ", \"children\": [";
    if (left) left->toJson(out, "Left");
    if (left && right) out << ", ";
    if (right) right->toJson(out, "Right");
    out << "]}";
}

void VariableNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Var: " << name << "\", \"type\": \"var\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << "}";
}

void NumberNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Num: " << value << "\", \"type\": \"num\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << "}";
}

void ProgramNode::exportJSON(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    toJson(file);
    file.close();
}

void ProgramNode::exportPNG(const std::string& filename) const {
    exportJSON("ast.json");
    // Call python script to render PNG, trying direct anaconda path then python
    std::string cmd = "\"D:\\anacondaa3\\python.exe\" scripts/render_ast.py ast.json " + filename + " || python scripts/render_ast.py ast.json " + filename;
    system(cmd.c_str());
    // Clean up temporary ast.json file
    std::remove("ast.json");
}

// ==========================================
// Three-Address Code (TAC) Generation
// ==========================================

std::string ProgramNode::generateTAC(TACBuilder& builder) const {
    for (const auto& stmt : statements) {
        stmt->generateTAC(builder);
    }
    return "";
}

std::string AssignmentNode::generateTAC(TACBuilder& builder) const {
    std::string exprResult = expression->generateTAC(builder);
    builder.emit(OpCode::ASSIGN, exprResult, "", varName);
    return "";
}

std::string PrintNode::generateTAC(TACBuilder& builder) const {
    std::string exprResult = expression->generateTAC(builder);
    builder.emit(OpCode::PRINT, exprResult, "", "");
    return "";
}

std::string BinaryOpNode::generateTAC(TACBuilder& builder) const {
    std::string arg1 = left->generateTAC(builder);
    std::string arg2 = right->generateTAC(builder);
    std::string temp = builder.newTemp();
    OpCode opCode;
    switch (op) {
        case BinaryOp::ADD: opCode = OpCode::ADD; break;
        case BinaryOp::SUB: opCode = OpCode::SUB; break;
        case BinaryOp::MUL: opCode = OpCode::MUL; break;
        case BinaryOp::DIV: opCode = OpCode::DIV; break;
    }
    builder.emit(opCode, arg1, arg2, temp);
    return temp;
}

std::string VariableNode::generateTAC(TACBuilder& builder) const {
    (void)builder;
    return name;
}

std::string NumberNode::generateTAC(TACBuilder& builder) const {
    (void)builder;
    return std::to_string(value);
}
