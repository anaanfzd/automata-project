#ifndef AUTOMATAVIZ_SUBSETCONSTRUCTION_H
#define AUTOMATAVIZ_SUBSETCONSTRUCTION_H

#include "Automata/NFA.h"
#include "Automata/DFA.h"
#include <vector>
#include <set>
#include <map>
#include <string>

namespace AutomataViz {

struct SubsetStep {
    int stepNumber;
    std::string actionName;
    int sourceDFAStateId;
    std::string sourceDFAStateLabel;
    std::set<int> sourceNFASubset;
    char symbol;
    std::set<int> moveResult;
    std::set<int> closureResult;
    int targetDFAStateId;
    std::string targetDFAStateLabel;
    bool isTargetNew;
    std::string explanation;
    DFA dfaSnapshot;
};

struct SubsetResult {
    bool success;
    std::string errorMessage;
    DFA finalDFA;
    std::vector<SubsetStep> steps;
    std::map<int, std::set<int>> dfaToNfaMap;
};

class SubsetConstruction {
public:
    static SubsetResult convert(const NFA& nfa);

private:
    static std::string formatSet(const std::set<int>& states);
    static std::string generateLabel(int index);
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_SUBSETCONSTRUCTION_H
