#ifndef AUTOMATAVIZ_THOMPSONCONSTRUCTION_H
#define AUTOMATAVIZ_THOMPSONCONSTRUCTION_H

#include "Automata/NFA.h"
#include "Regex/Token.h"
#include <vector>
#include <string>

namespace AutomataViz {

struct ThompsonStep {
    int stepNumber;
    std::string operation;
    std::string tokenStr;
    std::string description;
    std::vector<int> newStates;
    std::vector<Transition> newTransitions;
    NFA nfaSnapshot;
};

struct ThompsonResult {
    bool success;
    std::string errorMessage;
    NFA finalNFA;
    std::vector<ThompsonStep> steps;
};

class ThompsonConstruction {
public:
    static ThompsonResult build(const std::vector<Token>& postfixTokens);
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_THOMPSONCONSTRUCTION_H
