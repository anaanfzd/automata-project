#ifndef AUTOMATAVIZ_NFA_H
#define AUTOMATAVIZ_NFA_H

#include "Automata/State.h"
#include "Automata/Transition.h"
#include <vector>
#include <set>
#include <map>
#include <string>

namespace AutomataViz {

class NFA {
public:
    NFA();

    int addState(const std::string& label = "", bool isStart = false, bool isFinal = false);
    void addTransition(int from, int to, char symbol);

    void setStartState(int id);
    void setFinalState(int id, bool isFinal = true);

    int getStartState() const { return startStateId; }
    const std::set<int>& getFinalStates() const { return finalStateIds; }
    const std::vector<State>& getStates() const { return states; }
    const std::set<char>& getAlphabet() const { return alphabet; }
    const std::vector<Transition>& getTransitions() const { return transitions; }

    const State* getState(int id) const;
    State* getState(int id);

    bool hasState(int id) const;
    size_t stateCount() const { return states.size(); }
    size_t transitionCount() const { return transitions.size(); }

    // Core automata operations
    std::set<int> epsilonClosure(int stateId) const;
    std::set<int> epsilonClosure(const std::set<int>& stateIds) const;
    std::set<int> move(const std::set<int>& stateIds, char symbol) const;

    std::vector<Transition> getTransitionsFrom(int stateId) const;

    void clear();

private:
    std::vector<State> states;
    std::map<int, size_t> stateIdToIndex;
    std::set<char> alphabet;
    std::vector<Transition> transitions;
    int startStateId;
    std::set<int> finalStateIds;
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_NFA_H
