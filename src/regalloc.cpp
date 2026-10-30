/**
 * ==============================================================================
 * File: src/regalloc.cpp
 * Purpose: Implementation of Chaitin-Briggs Graph Coloring Register Allocator.
 * ==============================================================================
 */

#include "regalloc.h"
#include <iostream>
#include <algorithm>
#include <stack>

ChaitinRegisterAllocator::ChaitinRegisterAllocator(int k) : K(k) {
    // Standard allocatable 64-bit general purpose registers
    // (Reserving rax/rcx/rdx as dedicated scratch and ABI parameter registers)
    physicalRegisters = {"rbx", "r10", "r11", "r12", "r13", "r14", "r15", "rsi", "rdi"};
    if (K > static_cast<int>(physicalRegisters.size())) {
        K = static_cast<int>(physicalRegisters.size());
    }
}

void ChaitinRegisterAllocator::setRegisterPool(const std::vector<std::string>& regs) {
    physicalRegisters = regs;
    K = static_cast<int>(regs.size());
}

RegAllocResult ChaitinRegisterAllocator::allocate(const InterferenceGraph& originalGraph) {
    RegAllocResult result;
    result.kRegisters = K;

    InterferenceGraph graph = originalGraph;
    std::stack<std::pair<std::string, bool>> selectStack; // (variable, wasSpillCandidate)

    // Phase 1: Simplify & Potential Spill
    while (!graph.getNodes().empty()) {
        std::string simplifyNode = "";

        // Find node with degree < K
        for (const auto& node : graph.getNodes()) {
            if (graph.getDegree(node) < K) {
                simplifyNode = node;
                break;
            }
        }

        if (!simplifyNode.empty()) {
            selectStack.push({simplifyNode, false});
            graph.removeNode(simplifyNode);
        } else {
            // No node with degree < K: select spill candidate with maximum degree
            std::string maxDegreeNode = "";
            int maxDeg = -1;

            for (const auto& node : graph.getNodes()) {
                int deg = graph.getDegree(node);
                if (deg > maxDeg) {
                    maxDeg = deg;
                    maxDegreeNode = node;
                }
            }

            selectStack.push({maxDegreeNode, true});
            graph.removeNode(maxDegreeNode);
        }
    }

    // Phase 2: Select & Color Assignment
    int nextSpillOffset = -8;

    while (!selectStack.empty()) {
        auto [var, wasSpillCandidate] = selectStack.top();
        selectStack.pop();

        // Check colors of interfering neighbors in original graph
        std::unordered_set<std::string> neighborColors;
        for (const auto& neighbor : originalGraph.getNeighbors(var)) {
            auto it = result.registerMap.find(neighbor);
            if (it != result.registerMap.end()) {
                neighborColors.insert(it->second);
            }
        }

        // Find first available physical register
        std::string assignedColor = "";
        for (int i = 0; i < K && i < static_cast<int>(physicalRegisters.size()); ++i) {
            const std::string& reg = physicalRegisters[i];
            if (neighborColors.find(reg) == neighborColors.end()) {
                assignedColor = reg;
                break;
            }
        }

        if (!assignedColor.empty()) {
            result.registerMap[var] = assignedColor;
        } else {
            // Actual spill to stack frame
            result.stackSpills[var] = nextSpillOffset;
            nextSpillOffset -= 8;
            result.spillCount++;
            result.successWithoutSpills = false;
        }
    }

    // Stack alignment to 16 bytes for System V / Microsoft ABI compliance
    int rawBytes = std::abs(nextSpillOffset) + 32; // Include 32-byte shadow store
    result.stackFrameBytes = ((rawBytes + 15) / 16) * 16;

    return result;
}

void ChaitinRegisterAllocator::printAllocation(const RegAllocResult& result) const {
    std::cout << "=== CHAITIN-BRIGGS REGISTER ALLOCATION (K=" << result.kRegisters << ") ===\n";
    std::cout << "  Physical Register Map (" << result.registerMap.size() << " variables allocated):\n";
    for (const auto& [var, reg] : result.registerMap) {
        std::cout << "    Var [" << var << "] -> %" << reg << "\n";
    }

    if (result.spillCount > 0) {
        std::cout << "  Stack Frame Spills (" << result.spillCount << " variables spilled):\n";
        for (const auto& [var, offset] : result.stackSpills) {
            std::cout << "    Var [" << var << "] -> [rbp " << offset << "]\n";
        }
    } else {
        std::cout << "  Optimal: Zero stack spills required! All variables fit into physical registers.\n";
    }
    std::cout << "  Total Stack Frame Allocation: " << result.stackFrameBytes << " bytes (16-byte aligned)\n\n";
}
