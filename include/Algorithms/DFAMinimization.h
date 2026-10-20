#ifndef AUTOMATAVIZ_DFAMINIMIZATION_H
#define AUTOMATAVIZ_DFAMINIMIZATION_H

#include "Automata/DFA.h"
#include <vector>
#include <set>
#include <string>

namespace AutomataViz {

struct MinimizationStep {
    int roundNumber;
    std::string phaseName;
    std::vector<std::set<int>> partitionsBefore;
    std::vector<std::set<int>> partitionsAfter;
    std::string splitReason;
    std::string explanation;
    bool hasSplitOccurred;
};

struct MinimizationStats {
    size_t originalStateCount;
    size_t minimizedStateCount;
    size_t originalTransitionCount;
    size_t minimizedTransitionCount;
    std::vector<std::pair<std::string, std::vector<std::string>>> mergedGroups; // Minimized label -> list of original state labels
};

struct MinimizationResult {
    bool success;
    std::string errorMessage;
    DFA minimizedDFA;
    std::vector<MinimizationStep> steps;
    MinimizationStats stats;
};

class DFAMinimization {
public:
    static MinimizationResult minimize(const DFA& originalDfa);

private:
    static DFA removeUnreachableStates(const DFA& dfa, std::set<int>& reachableOut);
    static std::string formatPartitionList(const std::vector<std::set<int>>& partitions, const DFA& dfa);
    static std::string formatStateSet(const std::set<int>& states, const DFA& dfa);
    static int findPartitionIndex(int stateId, const std::vector<std::set<int>>& partitions);
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_DFAMINIMIZATION_H
