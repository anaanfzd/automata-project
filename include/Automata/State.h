#ifndef AUTOMATAVIZ_STATE_H
#define AUTOMATAVIZ_STATE_H

#include <string>
#include <set>

namespace AutomataViz {

class State {
public:
    int id;
    std::string label;
    bool isStart;
    bool isFinal;
    std::set<int> nfaSubsetState; // Used by DFA states to store the subset of NFA states they represent

    State(int id = -1, const std::string& label = "", bool start = false, bool final = false)
        : id(id), label(label), isStart(start), isFinal(final) {}

    bool operator<(const State& other) const {
        return id < other.id;
    }

    bool operator==(const State& other) const {
        return id == other.id;
    }
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_STATE_H
