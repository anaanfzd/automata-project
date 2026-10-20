#ifndef AUTOMATAVIZ_STRINGSIMULATOR_H
#define AUTOMATAVIZ_STRINGSIMULATOR_H

#include "Automata/DFA.h"
#include <string>
#include <vector>

namespace AutomataViz {

enum class SimulationState {
    NOT_STARTED,
    RUNNING,
    ACCEPTED,
    REJECTED
};

class StringSimulator {
public:
    StringSimulator();

    void setDFA(const DFA& dfa);
    void setInputString(const std::string& input);

    void reset();
    bool stepForward();
    bool stepBackward();
    void runToEnd();

    int getCurrentStepIndex() const { return currentStepIndex; }
    int getCurrentStateId() const { return currentStateId; }
    size_t getCurrentCharIndex() const;
    char getCurrentChar() const;
    SimulationState getStatus() const { return status; }
    const std::string& getInputString() const { return inputStr; }

    const DFASimulationResult& getFullResult() const { return cachedResult; }
    const DFASimulationStep* getCurrentStep() const;

private:
    DFA dfa;
    std::string inputStr;
    DFASimulationResult cachedResult;
    int currentStepIndex;
    int currentStateId;
    SimulationState status;

    void updateStatus();
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_STRINGSIMULATOR_H
