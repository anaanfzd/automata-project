#include "Automata/DFA.h"
#include <algorithm>

namespace AutomataViz {

DFA::DFA() : startStateId(-1) {}

int DFA::addState(const std::string& label, bool isStart, bool isFinal,
                 const std::set<int>& nfaSubset) {
    int newId = static_cast<int>(states.size());
    std::string finalLabel = label.empty() ? std::to_string(newId) : label;
    State newState(newId, finalLabel, isStart, isFinal);
    newState.nfaSubsetState = nfaSubset;
    states.push_back(newState);
    stateIdToIndex[newId] = states.size() - 1;

    if (isStart) {
        startStateId = newId;
    }
    if (isFinal) {
        finalStateIds.insert(newId);
    }
    return newId;
}

void DFA::setTransition(int from, char symbol, int to) {
    transitions[{from, symbol}] = to;
    alphabet.insert(symbol);
}

int DFA::getTransition(int from, char symbol) const {
    auto it = transitions.find({from, symbol});
    if (it != transitions.end()) {
        return it->second;
    }
    return -1;
}

bool DFA::hasTransition(int from, char symbol) const {
    return transitions.find({from, symbol}) != transitions.end();
}

void DFA::setStartState(int id) {
    startStateId = id;
    for (auto& st : states) {
        st.isStart = (st.id == id);
    }
}

void DFA::setFinalState(int id, bool isFinal) {
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

const State* DFA::getState(int id) const {
    auto it = stateIdToIndex.find(id);
    if (it != stateIdToIndex.end() && it->second < states.size()) {
        return &states[it->second];
    }
    return nullptr;
}

State* DFA::getState(int id) {
    auto it = stateIdToIndex.find(id);
    if (it != stateIdToIndex.end() && it->second < states.size()) {
        return &states[it->second];
    }
    return nullptr;
}

bool DFA::hasState(int id) const {
    return stateIdToIndex.find(id) != stateIdToIndex.end();
}

bool DFA::isAccepting(int stateId) const {
    return finalStateIds.find(stateId) != finalStateIds.end();
}

std::vector<Transition> DFA::getAllTransitions() const {
    std::vector<Transition> list;
    for (const auto& kv : transitions) {
        list.emplace_back(kv.first.first, kv.second, kv.first.second);
    }
    return list;
}

DFASimulationResult DFA::simulate(const std::string& inputString) const {
    DFASimulationResult res;
    res.inputString = inputString;
    res.accepted = false;
    res.finalStateId = -1;

    if (startStateId == -1 || states.empty()) {
        res.statusMessage = "DFA has no start state.";
        return res;
    }

    int currentState = startStateId;
    int stepNum = 1;

    // Check empty string acceptance
    if (inputString.empty()) {
        res.finalStateId = currentState;
        res.accepted = isAccepting(currentState);
        res.statusMessage = res.accepted ? "ACCEPTED (Empty string in start/accept state) ✓"
                                         : "REJECTED (Empty string in non-accept state) ✗";
        return res;
    }

    for (size_t i = 0; i < inputString.size(); ++i) {
        char symbol = inputString[i];
        int nextState = getTransition(currentState, symbol);

        DFASimulationStep step;
        step.stepNumber = stepNum++;
        step.fromState = currentState;
        step.symbolRead = symbol;
        step.remainingInputIndex = i;

        const State* curSt = getState(currentState);
        std::string curLabel = curSt ? curSt->label : std::to_string(currentState);

        if (nextState == -1) {
            step.toState = -1;
            step.isValidTransition = false;
            step.explanation = "State " + curLabel + " has no transition on symbol '" + symbol + "' (Dead/Trap).";
            res.steps.push_back(step);
            res.accepted = false;
            res.finalStateId = -1;
            res.statusMessage = "REJECTED (No valid transition on '" + std::string(1, symbol) + "') ✗";
            return res;
        }

        const State* nextSt = getState(nextState);
        std::string nextLabel = nextSt ? nextSt->label : std::to_string(nextState);

        step.toState = nextState;
        step.isValidTransition = true;
        step.explanation = "State " + curLabel + " --'" + symbol + "'--> State " + nextLabel;
        res.steps.push_back(step);

        currentState = nextState;
    }

    res.finalStateId = currentState;
    res.accepted = isAccepting(currentState);

    const State* finalSt = getState(currentState);
    std::string finalLabel = finalSt ? finalSt->label : std::to_string(currentState);

    if (res.accepted) {
        res.statusMessage = "ACCEPTED (Ended at accepting state " + finalLabel + ") ✓";
    } else {
        res.statusMessage = "REJECTED (Ended at non-accepting state " + finalLabel + ") ✗";
    }

    return res;
}

std::vector<DFATableRow> DFA::getTransitionTable() const {
    std::vector<DFATableRow> table;
    for (const auto& st : states) {
        DFATableRow row;
        row.stateId = st.id;
        row.label = st.label;
        row.isStart = st.isStart;
        row.isFinal = st.isFinal;

        // Build subset string
        if (!st.nfaSubsetState.empty()) {
            std::string sub = "{";
            bool first = true;
            for (int sid : st.nfaSubsetState) {
                if (!first) sub += ", ";
                sub += "q" + std::to_string(sid);
                first = false;
            }
            sub += "}";
            row.subsetString = sub;
        } else {
            row.subsetString = "{}";
        }

        for (char c : alphabet) {
            row.transitions[c] = getTransition(st.id, c);
        }
        table.push_back(row);
    }
    return table;
}

void DFA::clear() {
    states.clear();
    stateIdToIndex.clear();
    alphabet.clear();
    transitions.clear();
    startStateId = -1;
    finalStateIds.clear();
}

} // namespace AutomataViz
