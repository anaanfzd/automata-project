#ifndef AUTOMATAVIZ_CONTROLS_H
#define AUTOMATAVIZ_CONTROLS_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>

namespace AutomataViz {

class Button {
public:
    Button(const std::string& label = "", const sf::FloatRect& bounds = {0, 0, 100, 32});

    void setBounds(const sf::FloatRect& bounds) { this->bounds = bounds; }
    const sf::FloatRect& getBounds() const { return bounds; }

    void setLabel(const std::string& label) { this->label = label; }
    const std::string& getLabel() const { return label; }

    void setColors(sf::Color normal, sf::Color hover, sf::Color text);
    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool isEnabled() const { return enabled; }

    void setSelected(bool sel) { this->selected = sel; }
    bool isSelected() const { return selected; }

    void setOnClick(std::function<void()> callback) { this->onClick = callback; }

    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void render(sf::RenderTarget& target, const sf::Font& font);

private:
    std::string label;
    sf::FloatRect bounds;
    sf::Color normalColor;
    sf::Color hoverColor;
    sf::Color textColor;
    bool enabled;
    bool selected;
    bool isHovered;
    bool isPressed;
    std::function<void()> onClick;
};

class TextInput {
public:
    TextInput(const std::string& placeholder = "", const sf::FloatRect& bounds = {0, 0, 200, 36});

    void setBounds(const sf::FloatRect& bounds) { this->bounds = bounds; }
    const sf::FloatRect& getBounds() const { return bounds; }

    void setText(const std::string& text);
    const std::string& getText() const { return content; }

    void setPlaceholder(const std::string& ph) { placeholder = ph; }
    bool isFocused() const { return focused; }
    void setFocused(bool f) { focused = f; }

    void setOnSubmit(std::function<void(const std::string&)> callback) { onSubmit = callback; }

    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void render(sf::RenderTarget& target, const sf::Font& font);

private:
    std::string content;
    std::string placeholder;
    sf::FloatRect bounds;
    bool focused;
    float cursorBlinkTimer;
    bool cursorVisible;
    std::function<void(const std::string&)> onSubmit;
};

class TabBar {
public:
    TabBar(const sf::FloatRect& bounds = {0, 0, 800, 40});

    void addTab(const std::string& title);
    void setBounds(const sf::FloatRect& bounds);
    
    int getSelectedTab() const { return selectedTab; }
    void setSelectedTab(int index);

    void setOnTabChanged(std::function<void(int)> callback) { onTabChanged = callback; }

    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void render(sf::RenderTarget& target, const sf::Font& font);

private:
    sf::FloatRect bounds;
    std::vector<std::string> tabTitles;
    std::vector<sf::FloatRect> tabBounds;
    int selectedTab;
    int hoveredTab;
    std::function<void(int)> onTabChanged;

    void recalculateTabBounds();
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_CONTROLS_H
