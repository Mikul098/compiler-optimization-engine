/**
 * ==============================================================================
 * File: src/ast.cpp
 * Purpose: Implementation of AST methods, visualizations, and TAC generation.
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

void BlockNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;35mBlock { }\033[0m" << std::endl;
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    for (size_t i = 0; i < statements.size(); ++i) {
        bool last = (i == statements.size() - 1);
        statements[i]->print(childPrefix, last, "");
    }
}

void IfNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;36mIf Statement\033[0m" << std::endl;
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (condition) condition->print(childPrefix, false, "Cond:");
    if (thenBranch) thenBranch->print(childPrefix, elseBranch == nullptr, "Then:");
    if (elseBranch) elseBranch->print(childPrefix, true, "Else:");
}

void WhileNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;36mWhile Loop\033[0m" << std::endl;
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (condition) condition->print(childPrefix, false, "Cond:");
    if (body) body->print(childPrefix, true, "Body:");
}

void ForNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;36mFor Loop\033[0m" << std::endl;
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (init) init->print(childPrefix, false, "Init:");
    if (condition) condition->print(childPrefix, false, "Cond:");
    if (increment) increment->print(childPrefix, false, "Inc:");
    if (body) body->print(childPrefix, true, "Body:");
}

void DoWhileNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;36mDo-While Loop\033[0m" << std::endl;
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (body) body->print(childPrefix, false, "Body:");
    if (condition) condition->print(childPrefix, true, "Cond:");
}

void BreakNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;31mBreak\033[0m" << std::endl;
}

void ContinueNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;33mContinue\033[0m" << std::endl;
}

void CaseNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;35mCase:\033[0m " << value << std::endl;
}

void DefaultNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;35mDefault\033[0m" << std::endl;
}

void SwitchNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;36mSwitch Statement\033[0m" << std::endl;
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (expression) expression->print(childPrefix, body == nullptr, "Expr:");
    if (body) body->print(childPrefix, true, "Body:");
}

void GotoNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;31mGoto:\033[0m " << labelName << std::endl;
}

void LabelNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;32mLabel:\033[0m " << labelName << ":" << std::endl;
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
        case BinaryOp::MOD: opStr = "% (Modulo)"; break;
        case BinaryOp::AND: opStr = "&& (Logical AND)"; break;
        case BinaryOp::OR:  opStr = "|| (Logical OR)"; break;
        case BinaryOp::BAND: opStr = "& (Bitwise AND)"; break;
        case BinaryOp::BOR:  opStr = "| (Bitwise OR)"; break;
        case BinaryOp::BXOR: opStr = "^ (Bitwise XOR)"; break;
        case BinaryOp::SHL:  opStr = "<< (Shift Left)"; break;
        case BinaryOp::SHR:  opStr = ">> (Shift Right)"; break;
        case BinaryOp::EQ:  opStr = "== (Equal)"; break;
        case BinaryOp::NE:  opStr = "!= (Not Equal)"; break;
        case BinaryOp::LT:  opStr = "< (Less)"; break;
        case BinaryOp::GT:  opStr = "> (Greater)"; break;
        case BinaryOp::LE:  opStr = "<= (Less Equal)"; break;
        case BinaryOp::GE:  opStr = ">= (Greater Equal)"; break;
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

void TernaryNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;33mTernary (?:)\033[0m" << std::endl;
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (condition) condition->print(childPrefix, false, "Cond:");
    if (thenExpr) thenExpr->print(childPrefix, false, "Then:");
    if (elseExpr) elseExpr->print(childPrefix, true, "Else:");
}

void FunctionCallNode::print(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? "└── " : "├── ");
    if (!label.empty()) std::cout << label << " ";
    std::cout << "\033[1;35mCall:\033[0m " << funcName << "()" << std::endl;
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    for (size_t i = 0; i < args.size(); ++i) {
        bool last = (i == args.size() - 1);
        if (args[i]) args[i]->print(childPrefix, last, "Arg:");
    }
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

void BlockNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-root\">BLOCK</span><div class=\"val\">{ }</div></div>\n";
    if (!statements.empty()) {
        out << "<ul>\n";
        for (const auto& stmt : statements) {
            stmt->toHtml(out);
        }
        out << "</ul>\n";
    }
    out << "</li>\n";
}

void IfNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">IF</span><div class=\"val\">if</div></div>\n<ul>\n";
    if (condition) condition->toHtml(out);
    if (thenBranch) thenBranch->toHtml(out);
    if (elseBranch) elseBranch->toHtml(out);
    out << "</ul></li>\n";
}

void WhileNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">WHILE</span><div class=\"val\">while</div></div>\n<ul>\n";
    if (condition) condition->toHtml(out);
    if (body) body->toHtml(out);
    out << "</ul></li>\n";
}

void ForNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">FOR</span><div class=\"val\">for</div></div>\n<ul>\n";
    if (init) init->toHtml(out);
    if (condition) condition->toHtml(out);
    if (increment) increment->toHtml(out);
    if (body) body->toHtml(out);
    out << "</ul></li>\n";
}

void DoWhileNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">DO-WHILE</span><div class=\"val\">do-while</div></div>\n<ul>\n";
    if (body) body->toHtml(out);
    if (condition) condition->toHtml(out);
    out << "</ul></li>\n";
}

void BreakNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">BREAK</span><div class=\"val\">break</div></div></li>\n";
}

void ContinueNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">CONTINUE</span><div class=\"val\">continue</div></div></li>\n";
}

void CaseNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">CASE</span><div class=\"val\">case " << value << "</div></div></li>\n";
}

void DefaultNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">DEFAULT</span><div class=\"val\">default</div></div></li>\n";
}

void SwitchNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">SWITCH</span><div class=\"val\">switch</div></div>\n<ul>\n";
    if (expression) expression->toHtml(out);
    if (body) body->toHtml(out);
    out << "</ul></li>\n";
}

void GotoNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">GOTO</span><div class=\"val\">goto " << labelName << "</div></div></li>\n";
}

void LabelNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">LABEL</span><div class=\"val\">" << labelName << ":</div></div></li>\n";
}

void AssignmentNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">ASSIGN</span><div class=\"val\">" << varName << " =</div></div>\n";
    if (expression) {
        out << "<ul>\n";
        expression->toHtml(out);
        out << "</ul>\n";
    }
    out << "</li>\n";
}

void PrintNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-print\">PRINT</span><div class=\"val\">print</div></div>\n";
    if (expression) {
        out << "<ul>\n";
        expression->toHtml(out);
        out << "</ul>\n";
    }
    out << "</li>\n";
}

void BinaryOpNode::toHtml(std::ostream& out) const {
    std::string opStr;
    switch (op) {
        case BinaryOp::ADD: opStr = "+"; break;
        case BinaryOp::SUB: opStr = "-"; break;
        case BinaryOp::MUL: opStr = "*"; break;
        case BinaryOp::DIV: opStr = "/"; break;
        case BinaryOp::MOD: opStr = "%"; break;
        case BinaryOp::AND: opStr = "&&"; break;
        case BinaryOp::OR:  opStr = "||"; break;
        case BinaryOp::BAND: opStr = "&"; break;
        case BinaryOp::BOR:  opStr = "|"; break;
        case BinaryOp::BXOR: opStr = "^"; break;
        case BinaryOp::SHL:  opStr = "<<"; break;
        case BinaryOp::SHR:  opStr = ">>"; break;
        case BinaryOp::EQ:  opStr = "=="; break;
        case BinaryOp::NE:  opStr = "!="; break;
        case BinaryOp::LT:  opStr = "<"; break;
        case BinaryOp::GT:  opStr = ">"; break;
        case BinaryOp::LE:  opStr = "<="; break;
        case BinaryOp::GE:  opStr = ">="; break;
    }
    out << "<li><div class=\"card\"><span class=\"badge badge-op\">OP</span><div class=\"val\">" << opStr << "</div></div>\n";
    out << "<ul>\n";
    if (left) left->toHtml(out);
    if (right) right->toHtml(out);
    out << "</ul></li>\n";
}

void VariableNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-var\">VAR</span><div class=\"val\">" << name << "</div></div></li>\n";
}

void NumberNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-num\">NUM</span><div class=\"val\">" << value << "</div></div></li>\n";
}

void TernaryNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-op\">?:</span><div class=\"val\">ternary</div></div>\n<ul>\n";
    if (condition) condition->toHtml(out);
    if (thenExpr) thenExpr->toHtml(out);
    if (elseExpr) elseExpr->toHtml(out);
    out << "</ul></li>\n";
}

void FunctionCallNode::toHtml(std::ostream& out) const {
    out << "<li><div class=\"card\"><span class=\"badge badge-stmt\">CALL</span><div class=\"val\">" << funcName << "()</div></div>\n";
    if (!args.empty()) {
        out << "<ul>\n";
        for (const auto& a : args) if (a) a->toHtml(out);
        out << "</ul>\n";
    }
    out << "</li>\n";
}

// ==========================================
// JSON Tree Serialization Implementation
// ==========================================

void ProgramNode::toJson(std::ostream& out, const std::string& edge) const {
    (void)edge;
    out << "{\"name\": \"Program\", \"type\": \"root\", \"children\": [";
    for (size_t i = 0; i < statements.size(); ++i) {
        statements[i]->toJson(out);
        if (i + 1 < statements.size()) out << ", ";
    }
    out << "]}";
}

void BlockNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Block\", \"type\": \"root\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << ", \"children\": [";
    for (size_t i = 0; i < statements.size(); ++i) {
        statements[i]->toJson(out);
        if (i + 1 < statements.size()) out << ", ";
    }
    out << "]}";
}

void IfNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"If\", \"type\": \"stmt\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << ", \"children\": [";
    if (condition) condition->toJson(out, "Cond");
    if (thenBranch) {
        out << ", ";
        thenBranch->toJson(out, "Then");
    }
    if (elseBranch) {
        out << ", ";
        elseBranch->toJson(out, "Else");
    }
    out << "]}";
}

void WhileNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"While\", \"type\": \"stmt\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << ", \"children\": [";
    if (condition) condition->toJson(out, "Cond");
    if (body) {
        out << ", ";
        body->toJson(out, "Body");
    }
    out << "]}";
}

void ForNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"For\", \"type\": \"stmt\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << ", \"children\": [";
    bool first = true;
    if (init) { init->toJson(out, "Init"); first = false; }
    if (condition) { if (!first) out << ", "; condition->toJson(out, "Cond"); first = false; }
    if (increment) { if (!first) out << ", "; increment->toJson(out, "Inc"); first = false; }
    if (body) { if (!first) out << ", "; body->toJson(out, "Body"); }
    out << "]}";
}

void DoWhileNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"DoWhile\", \"type\": \"stmt\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << ", \"children\": [";
    if (body) body->toJson(out, "Body");
    if (body && condition) out << ", ";
    if (condition) condition->toJson(out, "Cond");
    out << "]}";
}

void BreakNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Break\", \"type\": \"stmt\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << "}";
}

void ContinueNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Continue\", \"type\": \"stmt\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << "}";
}

void CaseNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Case " << value << "\", \"type\": \"stmt\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << "}";
}

void DefaultNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Default\", \"type\": \"stmt\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << "}";
}

void SwitchNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Switch\", \"type\": \"stmt\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << ", \"children\": [";
    if (expression) expression->toJson(out, "Expr");
    if (expression && body) out << ", ";
    if (body) body->toJson(out, "Body");
    out << "]}";
}

void GotoNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Goto " << labelName << "\", \"type\": \"stmt\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << "}";
}

void LabelNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Label " << labelName << "\", \"type\": \"stmt\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << "}";
}

void AssignmentNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Assign: " << varName << "\", \"type\": \"assign\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    if (expression) {
        out << ", \"children\": [";
        expression->toJson(out, "Value");
        out << "]";
    }
    out << "}";
}

void PrintNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Print\", \"type\": \"print\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    if (expression) {
        out << ", \"children\": [";
        expression->toJson(out, "Arg");
        out << "]";
    }
    out << "}";
}

void BinaryOpNode::toJson(std::ostream& out, const std::string& edge) const {
    std::string opStr;
    switch (op) {
        case BinaryOp::ADD: opStr = "+"; break;
        case BinaryOp::SUB: opStr = "-"; break;
        case BinaryOp::MUL: opStr = "*"; break;
        case BinaryOp::DIV: opStr = "/"; break;
        case BinaryOp::MOD: opStr = "%"; break;
        case BinaryOp::AND: opStr = "&&"; break;
        case BinaryOp::OR:  opStr = "||"; break;
        case BinaryOp::BAND: opStr = "&"; break;
        case BinaryOp::BOR:  opStr = "|"; break;
        case BinaryOp::BXOR: opStr = "^"; break;
        case BinaryOp::SHL:  opStr = "<<"; break;
        case BinaryOp::SHR:  opStr = ">>"; break;
        case BinaryOp::EQ:  opStr = "=="; break;
        case BinaryOp::NE:  opStr = "!="; break;
        case BinaryOp::LT:  opStr = "<"; break;
        case BinaryOp::GT:  opStr = ">"; break;
        case BinaryOp::LE:  opStr = "<="; break;
        case BinaryOp::GE:  opStr = ">="; break;
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

void TernaryNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Ternary\", \"type\": \"op\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    out << ", \"children\": [";
    if (condition) condition->toJson(out, "Cond");
    if (condition && thenExpr) out << ", ";
    if (thenExpr) thenExpr->toJson(out, "Then");
    if ((condition || thenExpr) && elseExpr) out << ", ";
    if (elseExpr) elseExpr->toJson(out, "Else");
    out << "]}";
}

void FunctionCallNode::toJson(std::ostream& out, const std::string& edge) const {
    out << "{\"name\": \"Call " << funcName << "\", \"type\": \"call\"";
    if (!edge.empty()) out << ", \"edge\": \"" << edge << "\"";
    if (!args.empty()) {
        out << ", \"children\": [";
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i]) args[i]->toJson(out, "Arg");
            if (i + 1 < args.size()) out << ", ";
        }
        out << "]";
    }
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
    std::string cmd = "\"D:\\anacondaa3\\python.exe\" scripts/render_ast.py ast.json " + filename + " || python scripts/render_ast.py ast.json " + filename;
    system(cmd.c_str());
    std::remove("ast.json");
}

// ==========================================
// Three-Address Code (TAC) Generation
// ==========================================

std::string ProgramNode::generateTAC(TACBuilder& builder) const {
    for (const auto& stmt : statements) {
        if (stmt) stmt->generateTAC(builder);
    }
    return "";
}

std::string BlockNode::generateTAC(TACBuilder& builder) const {
    for (const auto& stmt : statements) {
        if (stmt) stmt->generateTAC(builder);
    }
    return "";
}

std::string IfNode::generateTAC(TACBuilder& builder) const {
    std::string condTemp = condition->generateTAC(builder);
    std::string elseLabel = builder.newLabel();
    std::string endLabel = builder.newLabel();

    builder.emit(OpCode::IF_ZERO, condTemp, elseBranch ? elseLabel : endLabel, "");
    if (thenBranch) thenBranch->generateTAC(builder);

    if (elseBranch) {
        builder.emit(OpCode::GOTO, endLabel, "", "");
        builder.emit(OpCode::LABEL, elseLabel, "", "");
        elseBranch->generateTAC(builder);
    }
    builder.emit(OpCode::LABEL, endLabel, "", "");
    return "";
}

std::string WhileNode::generateTAC(TACBuilder& builder) const {
    std::string startLabel = builder.newLabel();
    std::string endLabel = builder.newLabel();

    builder.pushContinueLabel(startLabel);
    builder.pushBreakLabel(endLabel);

    builder.emit(OpCode::LABEL, startLabel, "", "");
    std::string condTemp = condition->generateTAC(builder);
    builder.emit(OpCode::IF_ZERO, condTemp, endLabel, "");

    if (body) body->generateTAC(builder);

    builder.emit(OpCode::GOTO, startLabel, "", "");
    builder.emit(OpCode::LABEL, endLabel, "", "");

    builder.popBreakLabel();
    builder.popContinueLabel();
    return "";
}

std::string ForNode::generateTAC(TACBuilder& builder) const {
    if (init) init->generateTAC(builder);
    std::string startLabel = builder.newLabel();
    std::string incLabel = builder.newLabel();
    std::string endLabel = builder.newLabel();

    builder.pushContinueLabel(incLabel);
    builder.pushBreakLabel(endLabel);

    builder.emit(OpCode::LABEL, startLabel, "", "");
    if (condition) {
        std::string condTemp = condition->generateTAC(builder);
        builder.emit(OpCode::IF_ZERO, condTemp, endLabel, "");
    }
    if (body) body->generateTAC(builder);
    builder.emit(OpCode::LABEL, incLabel, "", "");
    if (increment) increment->generateTAC(builder);

    builder.emit(OpCode::GOTO, startLabel, "", "");
    builder.emit(OpCode::LABEL, endLabel, "", "");

    builder.popBreakLabel();
    builder.popContinueLabel();
    return "";
}

std::string DoWhileNode::generateTAC(TACBuilder& builder) const {
    std::string startLabel = builder.newLabel();
    std::string condLabel = builder.newLabel();
    std::string endLabel = builder.newLabel();

    builder.pushContinueLabel(condLabel);
    builder.pushBreakLabel(endLabel);

    builder.emit(OpCode::LABEL, startLabel, "", "");
    if (body) body->generateTAC(builder);
    builder.emit(OpCode::LABEL, condLabel, "", "");
    std::string condTemp = condition->generateTAC(builder);
    builder.emit(OpCode::IF_NOT_ZERO, condTemp, startLabel, "");
    builder.emit(OpCode::LABEL, endLabel, "", "");

    builder.popBreakLabel();
    builder.popContinueLabel();
    return "";
}

std::string BreakNode::generateTAC(TACBuilder& builder) const {
    std::string bLabel = builder.currentBreakLabel();
    if (!bLabel.empty()) {
        builder.emit(OpCode::GOTO, bLabel, "", "");
    }
    return "";
}

std::string ContinueNode::generateTAC(TACBuilder& builder) const {
    std::string cLabel = builder.currentContinueLabel();
    if (!cLabel.empty()) {
        builder.emit(OpCode::GOTO, cLabel, "", "");
    }
    return "";
}

std::string CaseNode::generateTAC(TACBuilder& builder) const {
    if (!label.empty()) {
        builder.emit(OpCode::LABEL, label, "", "");
    }
    return "";
}

std::string DefaultNode::generateTAC(TACBuilder& builder) const {
    if (!label.empty()) {
        builder.emit(OpCode::LABEL, label, "", "");
    }
    return "";
}

static void collectSwitchLabels(StatementNode* stmt, TACBuilder& builder,
                                std::vector<std::pair<int, std::string>>& cases,
                                std::string& defaultLabel) {
    if (!stmt) return;
    if (auto cn = dynamic_cast<CaseNode*>(stmt)) {
        cn->label = builder.newLabel();
        cases.push_back({cn->value, cn->label});
    } else if (auto dn = dynamic_cast<DefaultNode*>(stmt)) {
        dn->label = builder.newLabel();
        defaultLabel = dn->label;
    } else if (auto bn = dynamic_cast<BlockNode*>(stmt)) {
        for (const auto& s : bn->statements) {
            collectSwitchLabels(s.get(), builder, cases, defaultLabel);
        }
    }
}

std::string SwitchNode::generateTAC(TACBuilder& builder) const {
    std::string valTemp = expression->generateTAC(builder);
    std::string endLabel = builder.newLabel();

    std::vector<std::pair<int, std::string>> caseLabels;
    std::string defaultLabel = "";
    collectSwitchLabels(body.get(), builder, caseLabels, defaultLabel);

    for (const auto& cp : caseLabels) {
        std::string cmpTemp = builder.newTemp();
        builder.emit(OpCode::EQ, valTemp, std::to_string(cp.first), cmpTemp);
        builder.emit(OpCode::IF_NOT_ZERO, cmpTemp, cp.second, "");
    }

    if (!defaultLabel.empty()) {
        builder.emit(OpCode::GOTO, defaultLabel, "", "");
    } else {
        builder.emit(OpCode::GOTO, endLabel, "", "");
    }

    builder.pushBreakLabel(endLabel);
    if (body) body->generateTAC(builder);
    builder.emit(OpCode::LABEL, endLabel, "", "");
    builder.popBreakLabel();
    return "";
}

std::string GotoNode::generateTAC(TACBuilder& builder) const {
    builder.emit(OpCode::GOTO, labelName, "", "");
    return "";
}

std::string LabelNode::generateTAC(TACBuilder& builder) const {
    builder.emit(OpCode::LABEL, labelName, "", "");
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
        case BinaryOp::MOD: opCode = OpCode::MOD; break;
        case BinaryOp::BAND: opCode = OpCode::BAND; break;
        case BinaryOp::BOR:  opCode = OpCode::BOR;  break;
        case BinaryOp::BXOR: opCode = OpCode::BXOR; break;
        case BinaryOp::SHL:  opCode = OpCode::SHL;  break;
        case BinaryOp::SHR:  opCode = OpCode::SHR;  break;
        case BinaryOp::AND: opCode = OpCode::AND; break;
        case BinaryOp::OR:  opCode = OpCode::OR;  break;
        case BinaryOp::EQ:  opCode = OpCode::EQ;  break;
        case BinaryOp::NE:  opCode = OpCode::NE;  break;
        case BinaryOp::LT:  opCode = OpCode::LT;  break;
        case BinaryOp::GT:  opCode = OpCode::GT;  break;
        case BinaryOp::LE:  opCode = OpCode::LE;  break;
        case BinaryOp::GE:  opCode = OpCode::GE;  break;
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

std::string TernaryNode::generateTAC(TACBuilder& builder) const {
    std::string condTemp = condition ? condition->generateTAC(builder) : "1";
    std::string resTemp = builder.newTemp();
    std::string elseLabel = builder.newLabel();
    std::string endLabel = builder.newLabel();

    builder.emit(OpCode::IF_ZERO, condTemp, elseLabel, "");
    std::string thenVal = thenExpr ? thenExpr->generateTAC(builder) : "0";
    builder.emit(OpCode::ASSIGN, thenVal, "", resTemp);
    builder.emit(OpCode::GOTO, endLabel, "", "");
    builder.emit(OpCode::LABEL, elseLabel, "", "");
    std::string elseVal = elseExpr ? elseExpr->generateTAC(builder) : "0";
    builder.emit(OpCode::ASSIGN, elseVal, "", resTemp);
    builder.emit(OpCode::LABEL, endLabel, "", "");
    return resTemp;
}

std::string FunctionCallNode::generateTAC(TACBuilder& builder) const {
    std::vector<std::string> evaluatedArgs;
    for (const auto& a : args) {
        if (a) evaluatedArgs.push_back(a->generateTAC(builder));
    }
    std::string res = builder.newTemp();
    if ((funcName == "abs" || funcName == "std::abs" || funcName == "labs") && evaluatedArgs.size() == 1) {
        std::string zero = "0";
        std::string isNeg = builder.newTemp();
        builder.emit(OpCode::LT, evaluatedArgs[0], zero, isNeg);
        std::string posLabel = builder.newLabel();
        std::string endLabel = builder.newLabel();
        builder.emit(OpCode::IF_ZERO, isNeg, posLabel, "");
        std::string negTemp = builder.newTemp();
        builder.emit(OpCode::SUB, zero, evaluatedArgs[0], negTemp);
        builder.emit(OpCode::ASSIGN, negTemp, "", res);
        builder.emit(OpCode::GOTO, endLabel, "", "");
        builder.emit(OpCode::LABEL, posLabel, "", "");
        builder.emit(OpCode::ASSIGN, evaluatedArgs[0], "", res);
        builder.emit(OpCode::LABEL, endLabel, "", "");
        return res;
    } else if ((funcName == "max" || funcName == "std::max") && evaluatedArgs.size() == 2) {
        std::string isGt = builder.newTemp();
        builder.emit(OpCode::GT, evaluatedArgs[0], evaluatedArgs[1], isGt);
        std::string elseLabel = builder.newLabel();
        std::string endLabel = builder.newLabel();
        builder.emit(OpCode::IF_ZERO, isGt, elseLabel, "");
        builder.emit(OpCode::ASSIGN, evaluatedArgs[0], "", res);
        builder.emit(OpCode::GOTO, endLabel, "", "");
        builder.emit(OpCode::LABEL, elseLabel, "", "");
        builder.emit(OpCode::ASSIGN, evaluatedArgs[1], "", res);
        builder.emit(OpCode::LABEL, endLabel, "", "");
        return res;
    } else if ((funcName == "min" || funcName == "std::min") && evaluatedArgs.size() == 2) {
        std::string isLt = builder.newTemp();
        builder.emit(OpCode::LT, evaluatedArgs[0], evaluatedArgs[1], isLt);
        std::string elseLabel = builder.newLabel();
        std::string endLabel = builder.newLabel();
        builder.emit(OpCode::IF_ZERO, isLt, elseLabel, "");
        builder.emit(OpCode::ASSIGN, evaluatedArgs[0], "", res);
        builder.emit(OpCode::GOTO, endLabel, "", "");
        builder.emit(OpCode::LABEL, elseLabel, "", "");
        builder.emit(OpCode::ASSIGN, evaluatedArgs[1], "", res);
        builder.emit(OpCode::LABEL, endLabel, "", "");
        return res;
    }
    if (!evaluatedArgs.empty()) {
        builder.emit(OpCode::ASSIGN, evaluatedArgs[0], "", res);
    } else {
        builder.emit(OpCode::ASSIGN, "0", "", res);
    }
    return res;
}
