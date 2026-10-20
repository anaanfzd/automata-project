#include "Algorithms/DFAMinimization.h"
#include <queue>
#include <map>
#include <algorithm>

namespace AutomataViz {

DFA DFAMinimization::removeUnreachableStates(const DFA& dfa, std::set<int>& reachableOut) {
    reachableOut.clear();
    int start = dfa.getStartState();
    if (start == -1) return dfa;

    std::queue<int> q;
    reachableOut.insert(start);
    q.push(start);

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        for (char c : dfa.getAlphabet()) {
            int nxt = dfa.getTransition(curr, c);
            if (nxt != -1 && reachableOut.find(nxt) == reachableOut.end()) {
                reachableOut.insert(nxt);
                q.push(nxt);
            }
        }
    }

    DFA cleanDfa;
    cleanDfa.setAlphabet(dfa.getAlphabet());

    std::map<int, int> oldToNew;
    for (int oldId : reachableOut) {
        const State* st = dfa.getState(oldId);
        if (st) {
            int newId = cleanDfa.addState(st->label, st->isStart, st->isFinal, st->nfaSubsetState);
            oldToNew[oldId] = newId;
        }
    }

    for (int oldId : reachableOut) {
        for (char c : dfa.getAlphabet()) {
            int oldNxt = dfa.getTransition(oldId, c);
            if (oldNxt != -1 && reachableOut.find(oldNxt) != reachableOut.end()) {
                cleanDfa.setTransition(oldToNew[oldId], c, oldToNew[oldNxt]);
            }
        }
    }

    return cleanDfa;
}

int DFAMinimization::findPartitionIndex(int stateId, const std::vector<std::set<int>>& partitions) {
    if (stateId == -1) return -1;
    for (size_t i = 0; i < partitions.size(); ++i) {
        if (partitions[i].find(stateId) != partitions[i].end()) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::string DFAMinimization::formatStateSet(const std::set<int>& states, const DFA& dfa) {
    if (states.empty()) return "∅";
    std::string s = "{";
    bool first = true;
    for (int id : states) {
        if (!first) s += ", ";
        const State* st = dfa.getState(id);
        s += st ? st->label : std::to_string(id);
        first = false;
    }
    s += "}";
    return s;
}

std::string DFAMinimization::formatPartitionList(const std::vector<std::set<int>>& partitions, const DFA& dfa) {
    std::string s;
    for (size_t i = 0; i < partitions.size(); ++i) {
        if (i > 0) s += ", ";
        s += formatStateSet(partitions[i], dfa);
    }
    return s;
}

MinimizationResult DFAMinimization::minimize(const DFA& originalDfa) {
    MinimizationResult result;
    result.success = true;

    if (originalDfa.getStartState() == -1 || originalDfa.stateCount() == 0) {
        result.success = false;
        result.errorMessage = "Input DFA is empty.";
        return result;
    }

    // Reachability filter
    std::set<int> reachableStates;
    DFA dfa = removeUnreachableStates(originalDfa, reachableStates);

    result.stats.originalStateCount = originalDfa.stateCount();
    result.stats.originalTransitionCount = originalDfa.transitionCount();

    // Step 0: Initial partition into Non-Final and Final states
    std::set<int> nonFinals;
    std::set<int> finals;

    for (const auto& st : dfa.getStates()) {
        if (st.isFinal) {
            finals.insert(st.id);
        } else {
            nonFinals.insert(st.id);
        }
    }

    std::vector<std::set<int>> currentPartitions;
    if (!nonFinals.empty()) currentPartitions.push_back(nonFinals);
    if (!finals.empty()) currentPartitions.push_back(finals);

    MinimizationStep initialStep;
    initialStep.roundNumber = 0;
    initialStep.phaseName = "Initial Partition P0";
    initialStep.partitionsBefore = {};
    initialStep.partitionsAfter = currentPartitions;
    initialStep.hasSplitOccurred = true;
    initialStep.splitReason = "Initial partition separating accepting and non-accepting states.";
    initialStep.explanation = "Initial partition P0: Separated states into non-accepting " +
                             formatStateSet(nonFinals, dfa) + " and accepting " +
                             formatStateSet(finals, dfa) + ".";
    result.steps.push_back(initialStep);

    // Iterative partition refinement
    std::vector<char> sortedAlphabet(dfa.getAlphabet().begin(), dfa.getAlphabet().end());
    int round = 1;
    bool changed = true;

    while (changed) {
        changed = false;
        std::vector<std::set<int>> nextPartitions;
        std::string roundSplitReasons;

        for (const auto& group : currentPartitions) {
            if (group.size() <= 1) {
                nextPartitions.push_back(group);
                continue;
            }

            // Group states by signature: vector of partition indices for each alphabet symbol
            std::map<std::vector<int>, std::set<int>> signatureGroups;
            for (int sid : group) {
                std::vector<int> sig;
                for (char sym : sortedAlphabet) {
                    int target = dfa.getTransition(sid, sym);
                    int targetPart = findPartitionIndex(target, currentPartitions);
                    sig.push_back(targetPart);
                }
                signatureGroups[sig].insert(sid);
            }

            if (signatureGroups.size() > 1) {
                changed = true;
                std::string groupDesc = formatStateSet(group, dfa);
                roundSplitReasons += "Group " + groupDesc + " split into " +
                                     std::to_string(signatureGroups.size()) + " subsets. ";
            }

            for (const auto& kv : signatureGroups) {
                nextPartitions.push_back(kv.second);
            }
        }

        MinimizationStep step;
        step.roundNumber = round++;
        step.phaseName = "Refinement Round " + std::to_string(step.roundNumber);
        step.partitionsBefore = currentPartitions;
        step.partitionsAfter = nextPartitions;
        step.hasSplitOccurred = changed;

        if (changed) {
            step.splitReason = roundSplitReasons;
            step.explanation = "Partitions refined based on transition divergence: " +
                              formatPartitionList(nextPartitions, dfa);
        } else {
            step.splitReason = "No partition splits occurred. Fixed point achieved.";
            step.explanation = "Equivalence fixed point reached: All states in each group exhibit identical transition behaviors. Final partition: " +
                              formatPartitionList(nextPartitions, dfa);
        }

        result.steps.push_back(step);
        currentPartitions = nextPartitions;
    }

    // Construct the Minimized DFA
    DFA minDfa;
    minDfa.setAlphabet(dfa.getAlphabet());

    // Map each group to a new state in minDfa
    std::map<int, int> oldStateToNewMinState;
    std::vector<std::string> groupLabels;

    for (size_t i = 0; i < currentPartitions.size(); ++i) {
        const auto& group = currentPartitions[i];
        bool isStart = false;
        bool isFinal = false;
        std::set<int> combinedNfaSubset;

        std::vector<std::string> origLabels;
        for (int sid : group) {
            const State* origSt = dfa.getState(sid);
            if (origSt) {
                if (origSt->isStart) isStart = true;
                if (origSt->isFinal) isFinal = true;
                origLabels.push_back(origSt->label);
                combinedNfaSubset.insert(origSt->nfaSubsetState.begin(), origSt->nfaSubsetState.end());
            }
        }

        // Form combined label, e.g. "A" if single, or "A,C" if merged
        std::string newLabel;
        for (size_t l = 0; l < origLabels.size(); ++l) {
            if (l > 0) newLabel += ",";
            newLabel += origLabels[l];
        }

        int newId = minDfa.addState(newLabel, isStart, isFinal, combinedNfaSubset);
        for (int sid : group) {
            oldStateToNewMinState[sid] = newId;
        }

        result.stats.mergedGroups.emplace_back(newLabel, origLabels);
    }

    // Connect transitions between partition groups
    for (size_t i = 0; i < currentPartitions.size(); ++i) {
        int repStateId = *currentPartitions[i].begin();
        int minFrom = oldStateToNewMinState[repStateId];

        for (char sym : dfa.getAlphabet()) {
            int oldTarget = dfa.getTransition(repStateId, sym);
            if (oldTarget != -1) {
                int minTo = oldStateToNewMinState[oldTarget];
                minDfa.setTransition(minFrom, sym, minTo);
            }
        }
    }

    result.stats.minimizedStateCount = minDfa.stateCount();
    result.stats.minimizedTransitionCount = minDfa.transitionCount();
    result.minimizedDFA = minDfa;

    return result;
}

} // namespace AutomataViz
