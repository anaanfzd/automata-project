#ifndef AUTOMATAVIZ_INFOPANEL_H
#define AUTOMATAVIZ_INFOPANEL_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace AutomataViz {

struct InfoContent {
    std::string stageTitle;
    std::string stepIndicator; // "Step 3 of 7"
    std::string operationTitle;
    std::string inputMathText;
    std::string resultMathText;
    std::string explanation;
    std::vector<std::string> extraDetails;
    std::string statsText;
};

class InfoPanel {
public:
    InfoPanel(const sf::FloatRect& bounds = {0, 0, 320, 400});

    void setBounds(const sf::FloatRect& bounds) { this->bounds = bounds; }
    const sf::FloatRect& getBounds() const { return bounds; }

    void setContent(const InfoContent& content);
    void clear();

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void render(sf::RenderTarget& target, const sf::Font& font);

    static void drawWrappedText(sf::RenderTarget& target, const std::string& text, float x, float& y,
                                float maxWidth, unsigned int fontSize, sf::Color color, const sf::Font& font);

private:
    sf::FloatRect bounds;
    InfoContent content;
    float scrollOffset;
    float maxScroll;
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_INFOPANEL_H
