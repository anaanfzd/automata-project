#ifndef AUTOMATAVIZ_TRANSITION_H
#define AUTOMATAVIZ_TRANSITION_H

#include <string>

namespace AutomataViz {

constexpr char EPSILON_CHAR = '\0';

class Transition {
public:
    int from;
    int to;
    char symbol; // EPSILON_CHAR denotes an epsilon transition

    Transition(int from = -1, int to = -1, char symbol = EPSILON_CHAR)
        : from(from), to(to), symbol(symbol) {}

    bool isEpsilon() const {
        return symbol == EPSILON_CHAR;
    }

    std::string getDisplaySymbol() const {
        if (isEpsilon()) {
            return "ε";
        }
        return std::string(1, symbol);
    }

    bool operator<(const Transition& other) const {
        if (from != other.from) return from < other.from;
        if (to != other.to) return to < other.to;
        return symbol < other.symbol;
    }

    bool operator==(const Transition& other) const {
        return from == other.from && to == other.to && symbol == other.symbol;
    }
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_TRANSITION_H
