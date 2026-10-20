#include "Algorithms/ThompsonConstruction.h"
#include <stack>

namespace AutomataViz {

struct Fragment {
    int startState;
    int finalState;
};

ThompsonResult ThompsonConstruction::build(const std::vector<Token>& postfixTokens) {
    ThompsonResult result;
    result.success = true;

    if (postfixTokens.empty()) {
        result.success = false;
        result.errorMessage = "Empty postfix expression provided.";
        return result;
    }

    NFA nfa;
    std::stack<Fragment> fragStack;
    int stepNum = 1;

    for (const auto& token : postfixTokens) {
        ThompsonStep step;
        step.stepNumber = stepNum++;
        step.tokenStr = token.toString();

        if (token.type == TokenType::SYMBOL) {
            int s = nfa.addState();
            int f = nfa.addState();
            nfa.addTransition(s, f, token.symbol);
            fragStack.push({s, f});

            step.operation = "Symbol Atom '" + token.toString() + "'";
            step.description = "Constructed base NFA fragment for symbol '" + token.toString() +
                               "': created state q" + std::to_string(s) + " with transition '" +
                               token.toString() + "' to state q" + std::to_string(f) + ".";
            step.newStates = {s, f};
            step.newTransitions.emplace_back(s, f, token.symbol);

        } else if (token.type == TokenType::EPSILON) {
            int s = nfa.addState();
            int f = nfa.addState();
            nfa.addTransition(s, f, EPSILON_CHAR);
            fragStack.push({s, f});

            step.operation = "Epsilon Atom (ε)";
            step.description = "Constructed base NFA fragment for epsilon (ε): created state q" +
                               std::to_string(s) + " with ε-transition to state q" + std::to_string(f) + ".";
            step.newStates = {s, f};
            step.newTransitions.emplace_back(s, f, EPSILON_CHAR);

        } else if (token.type == TokenType::CONCAT) {
            if (fragStack.size() < 2) {
                result.success = false;
                result.errorMessage = "Malformed postfix expression: insufficient operands for concatenation.";
                return result;
            }
            Fragment frag2 = fragStack.top(); fragStack.pop();
            Fragment frag1 = fragStack.top(); fragStack.pop();

            // Connect exit of frag1 to start of frag2 via epsilon
            nfa.addTransition(frag1.finalState, frag2.startState, EPSILON_CHAR);
            fragStack.push({frag1.startState, frag2.finalState});

            step.operation = "Concatenation (.)";
            step.description = "Concatenated fragments: linked exit state q" + std::to_string(frag1.finalState) +
                               " to entry state q" + std::to_string(frag2.startState) + " via ε-transition.";
            step.newTransitions.emplace_back(frag1.finalState, frag2.startState, EPSILON_CHAR);

        } else if (token.type == TokenType::UNION) {
            if (fragStack.size() < 2) {
                result.success = false;
                result.errorMessage = "Malformed postfix expression: insufficient operands for union.";
                return result;
            }
            Fragment frag2 = fragStack.top(); fragStack.pop();
            Fragment frag1 = fragStack.top(); fragStack.pop();

            int newStart = nfa.addState();
            int newFinal = nfa.addState();

            nfa.addTransition(newStart, frag1.startState, EPSILON_CHAR);
            nfa.addTransition(newStart, frag2.startState, EPSILON_CHAR);
            nfa.addTransition(frag1.finalState, newFinal, EPSILON_CHAR);
            nfa.addTransition(frag2.finalState, newFinal, EPSILON_CHAR);

            fragStack.push({newStart, newFinal});

            step.operation = "Union (|)";
            step.description = "Created branch start q" + std::to_string(newStart) +
                               " with ε-transitions to sub-automata entries (q" + std::to_string(frag1.startState) +
                               ", q" + std::to_string(frag2.startState) + "), and joined exits (q" +
                               std::to_string(frag1.finalState) + ", q" + std::to_string(frag2.finalState) +
                               ") to new exit q" + std::to_string(newFinal) + " via ε.";
            step.newStates = {newStart, newFinal};
            step.newTransitions.emplace_back(newStart, frag1.startState, EPSILON_CHAR);
            step.newTransitions.emplace_back(newStart, frag2.startState, EPSILON_CHAR);
            step.newTransitions.emplace_back(frag1.finalState, newFinal, EPSILON_CHAR);
            step.newTransitions.emplace_back(frag2.finalState, newFinal, EPSILON_CHAR);

        } else if (token.type == TokenType::STAR) {
            if (fragStack.empty()) {
                result.success = false;
                result.errorMessage = "Malformed postfix expression: missing operand for Kleene star.";
                return result;
            }
            Fragment frag = fragStack.top(); fragStack.pop();

            int newStart = nfa.addState();
            int newFinal = nfa.addState();

            nfa.addTransition(newStart, frag.startState, EPSILON_CHAR);
            nfa.addTransition(newStart, newFinal, EPSILON_CHAR);
            nfa.addTransition(frag.finalState, frag.startState, EPSILON_CHAR);
            nfa.addTransition(frag.finalState, newFinal, EPSILON_CHAR);

            fragStack.push({newStart, newFinal});

            step.operation = "Kleene Star (*)";
            step.description = "Applied Kleene star: created entry q" + std::to_string(newStart) +
                               " and exit q" + std::to_string(newFinal) + ". Added bypass ε (0 occurrences) and loopback ε from q" +
                               std::to_string(frag.finalState) + " to q" + std::to_string(frag.startState) + " (repeat occurrences).";
            step.newStates = {newStart, newFinal};
            step.newTransitions.emplace_back(newStart, frag.startState, EPSILON_CHAR);
            step.newTransitions.emplace_back(newStart, newFinal, EPSILON_CHAR);
            step.newTransitions.emplace_back(frag.finalState, frag.startState, EPSILON_CHAR);
            step.newTransitions.emplace_back(frag.finalState, newFinal, EPSILON_CHAR);

        } else if (token.type == TokenType::PLUS) {
            if (fragStack.empty()) {
                result.success = false;
                result.errorMessage = "Malformed postfix expression: missing operand for plus.";
                return result;
            }
            Fragment frag = fragStack.top(); fragStack.pop();

            int newStart = nfa.addState();
            int newFinal = nfa.addState();

            nfa.addTransition(newStart, frag.startState, EPSILON_CHAR);
            nfa.addTransition(frag.finalState, frag.startState, EPSILON_CHAR);
            nfa.addTransition(frag.finalState, newFinal, EPSILON_CHAR);

            fragStack.push({newStart, newFinal});

            step.operation = "Plus Operator (+)";
            step.description = "Applied Plus (1+ occurrences): created entry q" + std::to_string(newStart) +
                               " to q" + std::to_string(frag.startState) + ", loopback from q" +
                               std::to_string(frag.finalState) + " to q" + std::to_string(frag.startState) +
                               ", and exit to q" + std::to_string(newFinal) + ".";
            step.newStates = {newStart, newFinal};
            step.newTransitions.emplace_back(newStart, frag.startState, EPSILON_CHAR);
            step.newTransitions.emplace_back(frag.finalState, frag.startState, EPSILON_CHAR);
            step.newTransitions.emplace_back(frag.finalState, newFinal, EPSILON_CHAR);

        } else if (token.type == TokenType::QUESTION) {
            if (fragStack.empty()) {
                result.success = false;
                result.errorMessage = "Malformed postfix expression: missing operand for optional.";
                return result;
            }
            Fragment frag = fragStack.top(); fragStack.pop();

            int newStart = nfa.addState();
            int newFinal = nfa.addState();

            nfa.addTransition(newStart, frag.startState, EPSILON_CHAR);
            nfa.addTransition(newStart, newFinal, EPSILON_CHAR);
            nfa.addTransition(frag.finalState, newFinal, EPSILON_CHAR);

            fragStack.push({newStart, newFinal});

            step.operation = "Optional (?)";
            step.description = "Applied Optional (0 or 1 occurrences): created entry q" + std::to_string(newStart) +
                               ", direct bypass ε to exit q" + std::to_string(newFinal) + ", and path through fragment.";
            step.newStates = {newStart, newFinal};
            step.newTransitions.emplace_back(newStart, frag.startState, EPSILON_CHAR);
            step.newTransitions.emplace_back(newStart, newFinal, EPSILON_CHAR);
            step.newTransitions.emplace_back(frag.finalState, newFinal, EPSILON_CHAR);
        }

        step.nfaSnapshot = nfa;
        result.steps.push_back(step);
    }

    if (fragStack.size() != 1) {
        result.success = false;
        result.errorMessage = "Incomplete construction: postfix expression did not reduce to a single fragment.";
        return result;
    }

    Fragment finalFrag = fragStack.top();
    nfa.setStartState(finalFrag.startState);
    nfa.setFinalState(finalFrag.finalState, true);

    // Final completion step
    ThompsonStep finishStep;
    finishStep.stepNumber = stepNum++;
    finishStep.operation = "Thompson Construction Complete";
    finishStep.tokenStr = "Done";
    finishStep.description = "Designated initial state q" + std::to_string(finalFrag.startState) +
                             " (Start) and accepting state q" + std::to_string(finalFrag.finalState) +
                             " (Double circle). Total NFA states: " + std::to_string(nfa.stateCount()) +
                             ", Transitions: " + std::to_string(nfa.transitionCount()) + ".";
    finishStep.newStates = {finalFrag.startState, finalFrag.finalState};
    finishStep.nfaSnapshot = nfa;
    result.steps.push_back(finishStep);

    result.finalNFA = nfa;
    return result;
}

} // namespace AutomataViz
