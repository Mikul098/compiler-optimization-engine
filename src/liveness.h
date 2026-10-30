/**
 * ==============================================================================
 * File: src/liveness.h
 * Purpose: Liveness Analysis and Register Interference Graph Construction.
 *
 * Role in Compiler:
 *  - Calculates exact live ranges and LiveIn/LiveOut sets for every TAC instruction.
 *  - Constructs the Register Interference Graph G = (V, E) where edge (u, v) denotes
 *    simultaneous liveness (variables competing for physical machine registers).
 * ==============================================================================
 */

#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include "tac.h"

struct LiveRange {
    std::string var;
    int start = -1;
    int end = -1;
};

class LivenessAnalysis {
public:
    std::vector<std::unordered_set<std::string>> liveIn;
    std::vector<std::unordered_set<std::string>> liveOut;
    std::unordered_map<std::string, LiveRange> ranges;
    std::unordered_set<std::string> allVariables;

    void analyze(const std::vector<TACInstruction>& instructions);
    void printLiveness(const std::vector<TACInstruction>& instructions) const;
};

class InterferenceGraph {
private:
    std::unordered_set<std::string> nodes;
    std::unordered_map<std::string, std::unordered_set<std::string>> adjList;

public:
    InterferenceGraph() = default;

    void build(const LivenessAnalysis& liveness, const std::vector<TACInstruction>& instructions);

    void addEdge(const std::string& u, const std::string& v);
    bool hasEdge(const std::string& u, const std::string& v) const;

    const std::unordered_set<std::string>& getNodes() const { return nodes; }
    const std::unordered_set<std::string>& getNeighbors(const std::string& u) const;
    int getDegree(const std::string& u) const;

    void removeNode(const std::string& u);

    void printGraph() const;
};
