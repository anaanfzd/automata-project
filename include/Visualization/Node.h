#ifndef AUTOMATAVIZ_NODE_H
#define AUTOMATAVIZ_NODE_H

#include <SFML/Graphics.hpp>
#include <string>

namespace AutomataViz {

class Node {
public:
    int stateId;
    std::string label;
    std::string subLabel; // e.g. subset "{q0, q1, q3}"
    sf::Vector2f position;
    float radius;
    bool isStart;
    bool isFinal;
    bool isHighlighted;
    bool isCurrentSimState;
    bool isNewlyAdded;
    bool isHovered;

    Node(int id = -1, const std::string& lbl = "", sf::Vector2f pos = {0.f, 0.f},
         bool start = false, bool final = false)
        : stateId(id), label(lbl), subLabel(""), position(pos), radius(28.0f),
          isStart(start), isFinal(final), isHighlighted(false),
          isCurrentSimState(false), isNewlyAdded(false), isHovered(false) {}

    bool contains(sf::Vector2f point) const {
        float dx = point.x - position.x;
        float dy = point.y - position.y;
        return (dx * dx + dy * dy) <= (radius * radius);
    }
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_NODE_H
