#ifndef AUTOMATAVIZ_TABLEVIEW_H
#define AUTOMATAVIZ_TABLEVIEW_H

#include "Automata/DFA.h"
#include <SFML/Graphics.hpp>
#include <vector>

namespace AutomataViz {

class TableView {
public:
    TableView(const sf::FloatRect& bounds = {0, 0, 800, 500});

    void setBounds(const sf::FloatRect& bounds) { this->bounds = bounds; }
    void setDFA(const DFA& dfa);

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void render(sf::RenderTarget& target, const sf::Font& font);

private:
    sf::FloatRect bounds;
    std::vector<DFATableRow> rows;
    std::vector<char> alphabetSorted;
    float scrollOffsetY;
    float maxScrollY;
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_TABLEVIEW_H
