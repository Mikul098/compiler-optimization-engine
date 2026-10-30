/**
 * ==============================================================================
 * File: src/liveness.cpp
 * Purpose: Implementation of Live Variable Analysis & Register Interference Graph.
 * ==============================================================================
 */

#include "liveness.h"
#include <algorithm>
#include <cctype>
#include <iomanip>

static bool isVariable(const std::string& str) {
    if (str.empty()) return false;
    if (std::isdigit(static_cast<unsigned char>(str[0]))) return false;
    if (str[0] == '-' && str.length() > 1 && std::isdigit(static_cast<unsigned char>(str[1]))) return false;
    return true;
}

void LivenessAnalysis::analyze(const std::vector<TACInstruction>& instructions) {
    size_t n = instructions.size();
    liveIn.assign(n, std::unordered_set<std::string>());
    liveOut.assign(n, std::unordered_set<std::string>());
    ranges.clear();
    allVariables.clear();

    if (n == 0) return;

    // 1. Identify def and use sets for each instruction
    std::vector<std::unordered_set<std::string>> defs(n);
    std::vector<std::unordered_set<std::string>> uses(n);

    // Label lookup map
    std::unordered_map<std::string, size_t> labelMap;
    for (size_t i = 0; i < n; ++i) {
        if (instructions[i].op == OpCode::LABEL) {
            labelMap[instructions[i].arg1] = i;
        }
    }

    // Successor list for each instruction
    std::vector<std::vector<size_t>> succs(n);
    for (size_t i = 0; i < n; ++i) {
        const auto& instr = instructions[i];

        // Track uses
        if (instr.op != OpCode::LABEL && instr.op != OpCode::GOTO) {
            if (isVariable(instr.arg1)) {
                uses[i].insert(instr.arg1);
                allVariables.insert(instr.arg1);
            }
            if (isVariable(instr.arg2)) {
                uses[i].insert(instr.arg2);
                allVariables.insert(instr.arg2);
            }
        }

        // Track defs
        if (!instr.result.empty() && isVariable(instr.result)) {
            defs[i].insert(instr.result);
            allVariables.insert(instr.result);
        }

        // Successor links
        if (instr.op == OpCode::GOTO) {
            auto it = labelMap.find(instr.arg1);
            if (it != labelMap.end()) {
                succs[i].push_back(it->second);
            }
        } else if (instr.op == OpCode::IF_ZERO || instr.op == OpCode::IF_NOT_ZERO) {
            auto it = labelMap.find(instr.arg2);
            if (it != labelMap.end()) {
                succs[i].push_back(it->second);
            }
            if (i + 1 < n) {
                succs[i].push_back(i + 1);
            }
        } else {
            if (i + 1 < n) {
                succs[i].push_back(i + 1);
            }
        }
    }

    // 2. Backward Dataflow Iteration
    bool changed = true;
    while (changed) {
        changed = false;

        for (int i = static_cast<int>(n) - 1; i >= 0; --i) {
            // LiveOut[i] = U (LiveIn[s]) for s in succs[i]
            std::unordered_set<std::string> newLiveOut;
            for (size_t s : succs[i]) {
                newLiveOut.insert(liveIn[s].begin(), liveIn[s].end());
            }

            // LiveIn[i] = Use[i] U (LiveOut[i] \ Def[i])
            std::unordered_set<std::string> newLiveIn = uses[i];
            for (const auto& var : newLiveOut) {
                if (defs[i].find(var) == defs[i].end()) {
                    newLiveIn.insert(var);
                }
            }

            if (newLiveIn != liveIn[i] || newLiveOut != liveOut[i]) {
                liveIn[i] = std::move(newLiveIn);
                liveOut[i] = std::move(newLiveOut);
                changed = true;
            }
        }
    }

    // 3. Compute live ranges
    for (const auto& var : allVariables) {
        LiveRange lr;
        lr.var = var;
        for (size_t i = 0; i < n; ++i) {
            if (defs[i].count(var) || uses[i].count(var) || liveIn[i].count(var) || liveOut[i].count(var)) {
                if (lr.start == -1) lr.start = static_cast<int>(i);
                lr.end = static_cast<int>(i);
            }
        }
        if (lr.start != -1) {
            ranges[var] = lr;
        }
    }
}

void LivenessAnalysis::printLiveness(const std::vector<TACInstruction>& instructions) const {
    std::cout << "=== LIVENESS DATAFLOW MATRIX ===\n";
    std::cout << std::left << std::setw(6) << "Line" 
              << std::setw(30) << "Instruction" 
              << std::setw(25) << "LiveIn" 
              << std::setw(25) << "LiveOut" << "\n";
    std::cout << std::string(86, '-') << "\n";

    for (size_t i = 0; i < instructions.size(); ++i) {
        std::string inStr = "{";
        for (const auto& v : liveIn[i]) inStr += v + " ";
        inStr += "}";

        std::string outStr = "{";
        for (const auto& v : liveOut[i]) outStr += v + " ";
        outStr += "}";

        std::cout << std::left << std::setw(6) << i
                  << std::setw(30) << instructions[i].toString()
                  << std::setw(25) << inStr
                  << std::setw(25) << outStr << "\n";
    }
}

void InterferenceGraph::build(const LivenessAnalysis& liveness, const std::vector<TACInstruction>& instructions) {
    nodes.clear();
    adjList.clear();

    // Add all active variables as nodes
    for (const auto& var : liveness.allVariables) {
        nodes.insert(var);
        adjList[var] = std::unordered_set<std::string>();
    }

    // Build interference edges
    for (size_t i = 0; i < instructions.size(); ++i) {
        const auto& instr = instructions[i];

        if (!instr.result.empty() && isVariable(instr.result)) {
            const std::string& defVar = instr.result;

            for (const auto& liveVar : liveness.liveOut[i]) {
                if (liveVar != defVar) {
                    addEdge(defVar, liveVar);
                }
            }
        }
    }
}

void InterferenceGraph::addEdge(const std::string& u, const std::string& v) {
    if (u == v) return;
    nodes.insert(u);
    nodes.insert(v);
    adjList[u].insert(v);
    adjList[v].insert(u);
}

bool InterferenceGraph::hasEdge(const std::string& u, const std::string& v) const {
    auto it = adjList.find(u);
    if (it != adjList.end()) {
        return it->second.find(v) != it->second.end();
    }
    return false;
}

const std::unordered_set<std::string>& InterferenceGraph::getNeighbors(const std::string& u) const {
    static const std::unordered_set<std::string> emptySet;
    auto it = adjList.find(u);
    if (it != adjList.end()) {
        return it->second;
    }
    return emptySet;
}

int InterferenceGraph::getDegree(const std::string& u) const {
    auto it = adjList.find(u);
    if (it != adjList.end()) {
        return static_cast<int>(it->second.size());
    }
    return 0;
}

void InterferenceGraph::removeNode(const std::string& u) {
    nodes.erase(u);
    for (auto& pair : adjList) {
        pair.second.erase(u);
    }
    adjList.erase(u);
}

void InterferenceGraph::printGraph() const {
    std::cout << "=== REGISTER INTERFERENCE GRAPH (" << nodes.size() << " Variables) ===\n";
    for (const auto& node : nodes) {
        std::cout << "  Node [" << node << "] (deg=" << getDegree(node) << ") -> ";
        for (const auto& neighbor : getNeighbors(node)) {
            std::cout << neighbor << " ";
        }
        std::cout << "\n";
    }
}
