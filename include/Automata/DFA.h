#ifndef AUTOMATAVIZ_DFA_H
#define AUTOMATAVIZ_DFA_H

#include "Automata/State.h"
#include "Automata/Transition.h"
#include <vector>
#include <set>
#include <map>
#include <string>

namespace AutomataViz {

struct DFASimulationStep {
    int stepNumber;
    int fromState;
    char symbolRead;
    int toState;
    size_t remainingInputIndex;
    std::string explanation;
    bool isValidTransition;
};

struct DFASimulationResult {
    bool accepted;
    std::string inputString;
    std::string statusMessage;
    std::vector<DFASimulationStep> steps;
    int finalStateId;
};

struct DFATableRow {
    int stateId;
    std::string label;
    bool isStart;
    bool isFinal;
    std::string subsetString; // e.g. "{q0, q1, q3}"
    std::map<char, int> transitions; // maps symbol -> target state ID (-1 if no transition / dead)
};

class DFA {
public:
    DFA();

    int addState(const std::string& label = "", bool isStart = false, bool isFinal = false,
                 const std::set<int>& nfaSubset = {});
    void setTransition(int from, char symbol, int to);
    int getTransition(int from, char symbol) const;
    bool hasTransition(int from, char symbol) const;

    void setStartState(int id);
    void setFinalState(int id, bool isFinal = true);

    int getStartState() const { return startStateId; }
    const std::set<int>& getFinalStates() const { return finalStateIds; }
    const std::vector<State>& getStates() const { return states; }
    const std::set<char>& getAlphabet() const { return alphabet; }
    void setAlphabet(const std::set<char>& alpha) { alphabet = alpha; }
    
    std::vector<Transition> getAllTransitions() const;

    const State* getState(int id) const;
    State* getState(int id);
    bool hasState(int id) const;

    size_t stateCount() const { return states.size(); }
    size_t transitionCount() const { return transitions.size(); }

    bool isAccepting(int stateId) const;

    // Simulation
    DFASimulationResult simulate(const std::string& inputString) const;

    // Transition Table representation
    std::vector<DFATableRow> getTransitionTable() const;

    void clear();

private:
    std::vector<State> states;
    std::map<int, size_t> stateIdToIndex;
    std::set<char> alphabet;
    std::map<std::pair<int, char>, int> transitions; // (from, symbol) -> to
    int startStateId;
    std::set<int> finalStateIds;
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_DFA_H
