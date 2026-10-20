#ifndef AUTOMATAVIZ_EDGE_H
#define AUTOMATAVIZ_EDGE_H

#include <string>
#include <vector>

namespace AutomataViz {

class Edge {
public:
    int fromState;
    int toState;
    std::vector<std::string> symbols;
    bool isBidirectional;
    bool isHighlighted;
    bool isCurrentSimTransition;
    bool isNewlyAdded;

    Edge(int from = -1, int to = -1)
        : fromState(from), toState(to), isBidirectional(false),
          isHighlighted(false), isCurrentSimTransition(false), isNewlyAdded(false) {}

    bool isSelfLoop() const {
        return fromState == toState;
    }

    void addSymbol(const std::string& sym) {
        for (const auto& s : symbols) {
            if (s == sym) return;
        }
        symbols.push_back(sym);
    }

    std::string getDisplayLabel() const {
        std::string res;
        for (size_t i = 0; i < symbols.size(); ++i) {
            if (i > 0) res += ", ";
            res += symbols[i];
        }
        return res;
    }
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_EDGE_H
