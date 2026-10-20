#include "UI/Controls.h"
#include "UI/UITheme.h"
#include <algorithm>

namespace AutomataViz {

// ================= Button =================
Button::Button(const std::string& label, const sf::FloatRect& bounds)
    : label(label), bounds(bounds), normalColor(UITheme::AccentBlue),
      hoverColor(sf::Color(59, 130, 246)), textColor(UITheme::TextWhite),
      enabled(true), selected(false), isHovered(false), isPressed(false) {}

void Button::setColors(sf::Color normal, sf::Color hover, sf::Color text) {
    normalColor = normal;
    hoverColor = hover;
    textColor = text;
}

bool Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!enabled) return false;

    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseMove.x, event.mouseMove.y});
        isHovered = bounds.contains(mousePos);
    } else if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
            if (bounds.contains(mousePos)) {
                isPressed = true;
                return true;
            }
        }
    } else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
            if (isPressed && bounds.contains(mousePos)) {
                isPressed = false;
                if (onClick) onClick();
                return true;
            }
            isPressed = false;
        }
    }
    return false;
}

void Button::update(float /*dt*/) {}

void Button::render(sf::RenderTarget& target, const sf::Font& font) {
    sf::RectangleShape shape({bounds.width, bounds.height});
    shape.setPosition(bounds.left, bounds.top);

    sf::Color fill = normalColor;
    if (!enabled) {
        fill = sf::Color(51, 65, 85);
    } else if (selected) {
        fill = UITheme::AccentCyan;
    } else if (isPressed) {
        fill = sf::Color(std::max(0, normalColor.r - 30), std::max(0, normalColor.g - 30), std::max(0, normalColor.b - 30));
    } else if (isHovered) {
        fill = hoverColor;
    }

    shape.setFillColor(fill);
    shape.setOutlineThickness(1.0f);
    shape.setOutlineColor(selected ? UITheme::TextWhite : UITheme::BorderLight);
    target.draw(shape);

    sf::Text text;
    text.setFont(font);
    text.setString(label);
    text.setCharacterSize(14);
    text.setFillColor(enabled ? textColor : UITheme::TextDim);

    sf::FloatRect tb = text.getLocalBounds();
    text.setOrigin(tb.left + tb.width / 2.0f, tb.top + tb.height / 2.0f);
    text.setPosition(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    target.draw(text);
}

// ================= TextInput =================
TextInput::TextInput(const std::string& placeholder, const sf::FloatRect& bounds)
    : content(""), placeholder(placeholder), bounds(bounds),
      focused(false), cursorBlinkTimer(0.0f), cursorVisible(true) {}

void TextInput::setText(const std::string& text) {
    content = text;
}

bool TextInput::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
            focused = bounds.contains(mousePos);
            cursorVisible = true;
            cursorBlinkTimer = 0.0f;
            return focused;
        }
    } else if (focused) {
        if (event.type == sf::Event::TextEntered) {
            sf::Uint32 unicode = event.text.unicode;
            if (unicode == 8) { // Backspace
                if (!content.empty()) {
                    content.pop_back();
                }
                return true;
            } else if (unicode == 13) { // Enter
                if (onSubmit) onSubmit(content);
                return true;
            } else if (unicode >= 32 && unicode < 127) { // Printable ASCII
                content += static_cast<char>(unicode);
                return true;
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.control && event.key.code == sf::Keyboard::V) {
                // Clipboard paste
                std::string clip = sf::Clipboard::getString().toAnsiString();
                for (char c : clip) {
                    if (c >= 32 && c < 127) {
                        content += c;
                    }
                }
                return true;
            }
        }
    }
    return false;
}

void TextInput::update(float dt) {
    if (focused) {
        cursorBlinkTimer += dt;
        if (cursorBlinkTimer >= 0.5f) {
            cursorBlinkTimer = 0.0f;
            cursorVisible = !cursorVisible;
        }
    } else {
        cursorVisible = false;
    }
}

void TextInput::render(sf::RenderTarget& target, const sf::Font& font) {
    sf::RectangleShape box({bounds.width, bounds.height});
    box.setPosition(bounds.left, bounds.top);
    box.setFillColor(UITheme::BgInput);
    box.setOutlineThickness(focused ? 2.0f : 1.0f);
    box.setOutlineColor(focused ? UITheme::AccentCyan : UITheme::BorderDark);
    target.draw(box);

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(16);

    if (content.empty() && !focused) {
        text.setString(placeholder);
        text.setFillColor(UITheme::TextDim);
    } else {
        text.setString(content);
        text.setFillColor(UITheme::TextWhite);
    }

    text.setPosition(bounds.left + 10.0f, bounds.top + (bounds.height - 20.0f) / 2.0f);
    target.draw(text);

    // Blinking cursor
    if (focused && cursorVisible) {
        float cursorX = bounds.left + 10.0f;
        if (!content.empty()) {
            cursorX += text.findCharacterPos(content.size()).x - text.findCharacterPos(0).x;
        }
        sf::RectangleShape cursor({2.0f, bounds.height - 12.0f});
        cursor.setPosition(cursorX, bounds.top + 6.0f);
        cursor.setFillColor(UITheme::AccentCyan);
        target.draw(cursor);
    }
}

// ================= TabBar =================
TabBar::TabBar(const sf::FloatRect& bounds)
    : bounds(bounds), selectedTab(0), hoveredTab(-1) {}

void TabBar::addTab(const std::string& title) {
    tabTitles.push_back(title);
    recalculateTabBounds();
}

void TabBar::setBounds(const sf::FloatRect& newBounds) {
    bounds = newBounds;
    recalculateTabBounds();
}

void TabBar::recalculateTabBounds() {
    tabBounds.clear();
    if (tabTitles.empty()) return;

    float tabWidth = bounds.width / static_cast<float>(tabTitles.size());
    for (size_t i = 0; i < tabTitles.size(); ++i) {
        tabBounds.emplace_back(bounds.left + i * tabWidth, bounds.top, tabWidth, bounds.height);
    }
}

void TabBar::setSelectedTab(int index) {
    if (index >= 0 && index < static_cast<int>(tabTitles.size())) {
        selectedTab = index;
    }
}

bool TabBar::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseMove.x, event.mouseMove.y});
        hoveredTab = -1;
        for (size_t i = 0; i < tabBounds.size(); ++i) {
            if (tabBounds[i].contains(mousePos)) {
                hoveredTab = static_cast<int>(i);
                break;
            }
        }
    } else if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
            for (size_t i = 0; i < tabBounds.size(); ++i) {
                if (tabBounds[i].contains(mousePos)) {
                    selectedTab = static_cast<int>(i);
                    if (onTabChanged) onTabChanged(selectedTab);
                    return true;
                }
            }
        }
    }
    return false;
}

void TabBar::render(sf::RenderTarget& target, const sf::Font& font) {
    sf::RectangleShape bg({bounds.width, bounds.height});
    bg.setPosition(bounds.left, bounds.top);
    bg.setFillColor(UITheme::BgPanel);
    target.draw(bg);

    for (size_t i = 0; i < tabTitles.size(); ++i) {
        const auto& tb = tabBounds[i];
        bool isSel = (static_cast<int>(i) == selectedTab);
        bool isHov = (static_cast<int>(i) == hoveredTab);

        if (isSel || isHov) {
            sf::RectangleShape tabBg({tb.width, tb.height});
            tabBg.setPosition(tb.left, tb.top);
            tabBg.setFillColor(isSel ? sf::Color(30, 58, 138, 150) : sf::Color(51, 65, 85, 100));
            target.draw(tabBg);
        }

        if (isSel) {
            // Indicator bar at bottom of active tab
            sf::RectangleShape indicator({tb.width, 3.0f});
            indicator.setPosition(tb.left, tb.top + tb.height - 3.0f);
            indicator.setFillColor(UITheme::AccentCyan);
            target.draw(indicator);
        }

        sf::Text text;
        text.setFont(font);
        text.setString(tabTitles[i]);
        text.setCharacterSize(14);
        text.setFillColor(isSel ? UITheme::AccentCyan : (isHov ? UITheme::TextWhite : UITheme::TextMuted));

        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
        text.setPosition(tb.left + tb.width / 2.0f, tb.top + tb.height / 2.0f);
        target.draw(text);
    }
}

} // namespace AutomataViz
