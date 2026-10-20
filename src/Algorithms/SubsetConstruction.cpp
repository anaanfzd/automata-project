#include "Algorithms/SubsetConstruction.h"
#include <queue>
#include <sstream>

namespace AutomataViz {

std::string SubsetConstruction::generateLabel(int index) {
    if (index < 26) {
        return std::string(1, static_cast<char>('A' + index));
    }
    return std::string(1, static_cast<char>('A' + (index % 26))) + std::to_string(index / 26);
}

std::string SubsetConstruction::formatSet(const std::set<int>& states) {
    if (states.empty()) return "∅";
    std::string s = "{";
    bool first = true;
    for (int id : states) {
        if (!first) s += ", ";
        s += "q" + std::to_string(id);
        first = false;
    }
    s += "}";
    return s;
}

SubsetResult SubsetConstruction::convert(const NFA& nfa) {
    SubsetResult result;
    result.success = true;

    if (nfa.getStartState() == -1) {
        result.success = false;
        result.errorMessage = "Input NFA has no start state.";
        return result;
    }

    DFA dfa;
    dfa.setAlphabet(nfa.getAlphabet());

    std::map<std::set<int>, int> subsetToDfaState;
    std::queue<int> unmarkedQueue;
    int stepNum = 1;

    // Step 1: Initial state epsilon closure
    std::set<int> initialNfaSet = nfa.epsilonClosure(nfa.getStartState());
    bool isStartAccepting = false;
    for (int fid : nfa.getFinalStates()) {
        if (initialNfaSet.find(fid) != initialNfaSet.end()) {
            isStartAccepting = true;
            break;
        }
    }

    std::string startLabel = generateLabel(0);
    int startDfaId = dfa.addState(startLabel, true, isStartAccepting, initialNfaSet);
    subsetToDfaState[initialNfaSet] = startDfaId;
    unmarkedQueue.push(startDfaId);
    result.dfaToNfaMap[startDfaId] = initialNfaSet;

    SubsetStep initStep;
    initStep.stepNumber = stepNum++;
    initStep.actionName = "Initial State ε-Closure";
    initStep.sourceDFAStateId = -1;
    initStep.sourceDFAStateLabel = "Start";
    initStep.symbol = '\0';
    initStep.closureResult = initialNfaSet;
    initStep.targetDFAStateId = startDfaId;
    initStep.targetDFAStateLabel = startLabel;
    initStep.isTargetNew = true;
    initStep.explanation = "Computed ε-closure of NFA start state {q" + std::to_string(nfa.getStartState()) +
                          "} = " + formatSet(initialNfaSet) + ". Formed initial DFA state " + startLabel +
                          (isStartAccepting ? " (Accepting)." : ".");
    initStep.dfaSnapshot = dfa;
    result.steps.push_back(initStep);

    // Get sorted alphabet list for deterministic order
    std::vector<char> sortedAlphabet(nfa.getAlphabet().begin(), nfa.getAlphabet().end());

    while (!unmarkedQueue.empty()) {
        int uId = unmarkedQueue.front();
        unmarkedQueue.pop();

        const State* uState = dfa.getState(uId);
        if (!uState) continue;
        std::set<int> uSubset = uState->nfaSubsetState;
        std::string uLabel = uState->label;

        for (char symbol : sortedAlphabet) {
            std::set<int> moveSet = nfa.move(uSubset, symbol);
            std::set<int> closureSet = nfa.epsilonClosure(moveSet);

            SubsetStep step;
            step.stepNumber = stepNum++;
            step.sourceDFAStateId = uId;
            step.sourceDFAStateLabel = uLabel;
            step.sourceNFASubset = uSubset;
            step.symbol = symbol;
            step.moveResult = moveSet;
            step.closureResult = closureSet;

            if (closureSet.empty()) {
                step.actionName = "Empty Transition (Trap)";
                step.targetDFAStateId = -1;
                step.targetDFAStateLabel = "None";
                step.isTargetNew = false;
                step.explanation = "From DFA State " + uLabel + " on '" + symbol + "': move(" +
                                  formatSet(uSubset) + ", '" + symbol + "') = ∅. No transition added.";
                step.dfaSnapshot = dfa;
                result.steps.push_back(step);
                continue;
            }

            auto it = subsetToDfaState.find(closureSet);
            if (it != subsetToDfaState.end()) {
                // Existing DFA state
                int vId = it->second;
                dfa.setTransition(uId, symbol, vId);

                const State* vState = dfa.getState(vId);
                std::string vLabel = vState ? vState->label : std::to_string(vId);

                step.actionName = "Transition to Existing State";
                step.targetDFAStateId = vId;
                step.targetDFAStateLabel = vLabel;
                step.isTargetNew = false;
                step.explanation = "From DFA State " + uLabel + " on '" + symbol + "': move = " +
                                  formatSet(moveSet) + ", ε-closure = " + formatSet(closureSet) +
                                  ". Matches existing DFA state " + vLabel + ". Added transition " +
                                  uLabel + " --'" + symbol + "'--> " + vLabel + ".";
            } else {
                // New DFA state discovered
                int nextIndex = static_cast<int>(dfa.stateCount());
                std::string vLabel = generateLabel(nextIndex);

                bool isFinal = false;
                for (int fid : nfa.getFinalStates()) {
                    if (closureSet.find(fid) != closureSet.end()) {
                        isFinal = true;
                        break;
                    }
                }

                int vId = dfa.addState(vLabel, false, isFinal, closureSet);
                dfa.setTransition(uId, symbol, vId);
                subsetToDfaState[closureSet] = vId;
                result.dfaToNfaMap[vId] = closureSet;
                unmarkedQueue.push(vId);

                step.actionName = "Discovered New DFA State";
                step.targetDFAStateId = vId;
                step.targetDFAStateLabel = vLabel;
                step.isTargetNew = true;
                step.explanation = "From DFA State " + uLabel + " on '" + symbol + "': move = " +
                                  formatSet(moveSet) + ", ε-closure = " + formatSet(closureSet) +
                                  ". Discovered NEW DFA state " + vLabel + " = " + formatSet(closureSet) +
                                  (isFinal ? " [ACCEPTING]" : "") + ". Added transition " +
                                  uLabel + " --'" + symbol + "'--> " + vLabel + ".";
            }

            step.dfaSnapshot = dfa;
            result.steps.push_back(step);
        }
    }

    // Final summary step
    SubsetStep doneStep;
    doneStep.stepNumber = stepNum++;
    doneStep.actionName = "Subset Construction Complete";
    doneStep.sourceDFAStateId = -1;
    doneStep.sourceDFAStateLabel = "Complete";
    doneStep.symbol = '\0';
    doneStep.targetDFAStateId = -1;
    doneStep.targetDFAStateLabel = "";
    doneStep.isTargetNew = false;
    doneStep.explanation = "Subset construction complete. All reachable state subsets processed. Generated " +
                          std::to_string(dfa.stateCount()) + " DFA states and " +
                          std::to_string(dfa.transitionCount()) + " deterministic transitions.";
    doneStep.dfaSnapshot = dfa;
    result.steps.push_back(doneStep);

    result.finalDFA = dfa;
    return result;
}

} // namespace AutomataViz
