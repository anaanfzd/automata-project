#include "Algorithms/StringSimulator.h"

namespace AutomataViz {

StringSimulator::StringSimulator()
    : currentStepIndex(-1), currentStateId(-1), status(SimulationState::NOT_STARTED) {}

void StringSimulator::setDFA(const DFA& newDfa) {
    dfa = newDfa;
    currentStateId = dfa.getStartState();
    reset();
}

void StringSimulator::setInputString(const std::string& input) {
    inputStr = input;
    cachedResult = dfa.simulate(input);
    reset();
}

void StringSimulator::reset() {
    currentStepIndex = -1;
    currentStateId = dfa.getStartState();
    status = SimulationState::NOT_STARTED;
}

bool StringSimulator::stepForward() {
    if (cachedResult.steps.empty()) {
        // Special case: empty string
        currentStepIndex = 0;
        updateStatus();
        return false;
    }

    if (currentStepIndex < static_cast<int>(cachedResult.steps.size()) - 1) {
        currentStepIndex++;
        currentStateId = cachedResult.steps[currentStepIndex].toState;
        updateStatus();
        return true;
    }

    updateStatus();
    return false;
}

bool StringSimulator::stepBackward() {
    if (currentStepIndex > -1) {
        currentStepIndex--;
        if (currentStepIndex == -1) {
            currentStateId = dfa.getStartState();
        } else {
            currentStateId = cachedResult.steps[currentStepIndex].toState;
        }
        updateStatus();
        return true;
    }
    return false;
}

void StringSimulator::runToEnd() {
    if (!cachedResult.steps.empty()) {
        currentStepIndex = static_cast<int>(cachedResult.steps.size()) - 1;
        currentStateId = cachedResult.steps[currentStepIndex].toState;
    } else {
        currentStepIndex = 0;
    }
    updateStatus();
}

void StringSimulator::updateStatus() {
    if (currentStepIndex == -1) {
        status = SimulationState::NOT_STARTED;
    } else if (inputStr.empty()) {
        status = cachedResult.accepted ? SimulationState::ACCEPTED : SimulationState::REJECTED;
    } else if (currentStepIndex < static_cast<int>(cachedResult.steps.size()) - 1) {
        // In the middle of execution
        const auto& step = cachedResult.steps[currentStepIndex];
        if (!step.isValidTransition) {
            status = SimulationState::REJECTED;
        } else {
            status = SimulationState::RUNNING;
        }
    } else {
        // At the last step
        status = cachedResult.accepted ? SimulationState::ACCEPTED : SimulationState::REJECTED;
    }
}

size_t StringSimulator::getCurrentCharIndex() const {
    if (currentStepIndex >= 0 && currentStepIndex < static_cast<int>(cachedResult.steps.size())) {
        return cachedResult.steps[currentStepIndex].remainingInputIndex;
    }
    return 0;
}

char StringSimulator::getCurrentChar() const {
    if (currentStepIndex >= 0 && currentStepIndex < static_cast<int>(cachedResult.steps.size())) {
        return cachedResult.steps[currentStepIndex].symbolRead;
    }
    return '\0';
}

const DFASimulationStep* StringSimulator::getCurrentStep() const {
    if (currentStepIndex >= 0 && currentStepIndex < static_cast<int>(cachedResult.steps.size())) {
        return &cachedResult.steps[currentStepIndex];
    }
    return nullptr;
}

} // namespace AutomataViz
