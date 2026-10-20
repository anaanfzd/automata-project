#include "Automata/NFA.h"
#include <queue>
#include <algorithm>

namespace AutomataViz {

NFA::NFA() : startStateId(-1) {}

int NFA::addState(const std::string& label, bool isStart, bool isFinal) {
    int newId = static_cast<int>(states.size());
    std::string finalLabel = label.empty() ? ("q" + std::to_string(newId)) : label;
    states.emplace_back(newId, finalLabel, isStart, isFinal);
    stateIdToIndex[newId] = states.size() - 1;

    if (isStart) {
        startStateId = newId;
    }
    if (isFinal) {
        finalStateIds.insert(newId);
    }
    return newId;
}

void NFA::addTransition(int from, int to, char symbol) {
    transitions.emplace_back(from, to, symbol);
    if (symbol != EPSILON_CHAR && symbol != '#') {
        alphabet.insert(symbol);
    }
}

void NFA::setStartState(int id) {
    startStateId = id;
    for (auto& st : states) {
        st.isStart = (st.id == id);
    }
}

void NFA::setFinalState(int id, bool isFinal) {
    if (isFinal) {
        finalStateIds.insert(id);
    } else {
        finalStateIds.erase(id);
    }
    for (auto& st : states) {
        if (st.id == id) {
            st.isFinal = isFinal;
            break;
        }
    }
}

const State* NFA::getState(int id) const {
    auto it = stateIdToIndex.find(id);
    if (it != stateIdToIndex.end() && it->second < states.size()) {
        return &states[it->second];
    }
    return nullptr;
}

State* NFA::getState(int id) {
    auto it = stateIdToIndex.find(id);
    if (it != stateIdToIndex.end() && it->second < states.size()) {
        return &states[it->second];
    }
    return nullptr;
}

bool NFA::hasState(int id) const {
    return stateIdToIndex.find(id) != stateIdToIndex.end();
}

std::set<int> NFA::epsilonClosure(int stateId) const {
    std::set<int> closure;
    std::queue<int> q;

    closure.insert(stateId);
    q.push(stateId);

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        for (const auto& trans : transitions) {
            if (trans.from == curr && trans.isEpsilon()) {
                if (closure.find(trans.to) == closure.end()) {
                    closure.insert(trans.to);
                    q.push(trans.to);
                }
            }
        }
    }

    return closure;
}

std::set<int> NFA::epsilonClosure(const std::set<int>& stateIds) const {
    std::set<int> totalClosure;
    for (int sid : stateIds) {
        std::set<int> single = epsilonClosure(sid);
        totalClosure.insert(single.begin(), single.end());
    }
    return totalClosure;
}

std::set<int> NFA::move(const std::set<int>& stateIds, char symbol) const {
    std::set<int> result;
    for (int sid : stateIds) {
        for (const auto& trans : transitions) {
            if (trans.from == sid && trans.symbol == symbol) {
                result.insert(trans.to);
            }
        }
    }
    return result;
}

std::vector<Transition> NFA::getTransitionsFrom(int stateId) const {
    std::vector<Transition> list;
    for (const auto& trans : transitions) {
        if (trans.from == stateId) {
            list.push_back(trans);
        }
    }
    return list;
}

void NFA::clear() {
    states.clear();
    stateIdToIndex.clear();
    alphabet.clear();
    transitions.clear();
    startStateId = -1;
    finalStateIds.clear();
}

} // namespace AutomataViz
