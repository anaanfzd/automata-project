#include "UI/InfoPanel.h"
#include "UI/UITheme.h"
#include <sstream>
#include <algorithm>

namespace AutomataViz {

InfoPanel::InfoPanel(const sf::FloatRect& bounds)
    : bounds(bounds), scrollOffset(0.0f), maxScroll(0.0f) {}

void InfoPanel::setContent(const InfoContent& newContent) {
    content = newContent;
    scrollOffset = 0.0f;
}

void InfoPanel::clear() {
    content = InfoContent();
    scrollOffset = 0.0f;
}

void InfoPanel::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseWheelScrolled) {
        sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseWheelScroll.x, event.mouseWheelScroll.y});
        if (bounds.contains(mousePos)) {
            scrollOffset -= event.mouseWheelScroll.delta * 24.0f;
            if (scrollOffset < 0.0f) scrollOffset = 0.0f;
            if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        }
    }
}

void InfoPanel::drawWrappedText(sf::RenderTarget& target, const std::string& text, float x, float& y,
                               float maxWidth, unsigned int fontSize, sf::Color color, const sf::Font& font) {
    std::istringstream words(text);
    std::string word;
    std::string currentLine;

    sf::Text sfText;
    sfText.setFont(font);
    sfText.setCharacterSize(fontSize);
    sfText.setFillColor(color);

    float lineHeight = fontSize * 1.35f;

    while (words >> word) {
        std::string testLine = currentLine.empty() ? word : (currentLine + " " + word);
        sfText.setString(testLine);
        if (sfText.getLocalBounds().width > maxWidth) {
            sfText.setString(currentLine);
            sfText.setPosition(x, y);
            target.draw(sfText);
            y += lineHeight;
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }

    if (!currentLine.empty()) {
        sfText.setString(currentLine);
        sfText.setPosition(x, y);
        target.draw(sfText);
        y += lineHeight;
    }
}

void InfoPanel::render(sf::RenderTarget& target, const sf::Font& font) {
    // Panel Background
    sf::RectangleShape bg({bounds.width, bounds.height});
    bg.setPosition(bounds.left, bounds.top);
    bg.setFillColor(UITheme::BgPanel);
    bg.setOutlineThickness(1.0f);
    bg.setOutlineColor(UITheme::BorderDark);
    target.draw(bg);

    // Save and set clipping view
    sf::View originalView = target.getView();
    sf::View panelView(bounds);
    panelView.setViewport(sf::FloatRect(
        bounds.left / originalView.getSize().x,
        bounds.top / originalView.getSize().y,
        bounds.width / originalView.getSize().x,
        bounds.height / originalView.getSize().y
    ));
    target.setView(panelView);

    float curY = bounds.top + 16.0f - scrollOffset;
    const float padX = 16.0f;
    const float contentWidth = bounds.width - padX * 2.0f;

    // Stage Title & Step Indicator
    if (!content.stageTitle.empty()) {
        sf::Text stageText;
        stageText.setFont(font);
        stageText.setString(content.stageTitle);
        stageText.setCharacterSize(16);
        stageText.setStyle(sf::Text::Bold);
        stageText.setFillColor(UITheme::AccentCyan);
        stageText.setPosition(bounds.left + padX, curY);
        target.draw(stageText);

        if (!content.stepIndicator.empty()) {
            sf::Text stepText;
            stepText.setFont(font);
            stepText.setString(content.stepIndicator);
            stepText.setCharacterSize(12);
            stepText.setFillColor(UITheme::TextMuted);
            sf::FloatRect stBounds = stepText.getLocalBounds();
            stepText.setPosition(bounds.left + bounds.width - padX - stBounds.width, curY + 2.0f);
            target.draw(stepText);
        }

        curY += 28.0f;

        // Divider
        sf::RectangleShape div({contentWidth, 1.0f});
        div.setPosition(bounds.left + padX, curY);
        div.setFillColor(UITheme::BorderDark);
        target.draw(div);
        curY += 12.0f;
    }

    // Operation Title
    if (!content.operationTitle.empty()) {
        sf::Text opText;
        opText.setFont(font);
        opText.setString(content.operationTitle);
        opText.setCharacterSize(15);
        opText.setStyle(sf::Text::Bold);
        opText.setFillColor(UITheme::AccentAmber);
        opText.setPosition(bounds.left + padX, curY);
        target.draw(opText);
        curY += 24.0f;
    }

    // Input Math Card
    if (!content.inputMathText.empty()) {
        sf::Text lbl;
        lbl.setFont(font);
        lbl.setString("INPUT / TARGET:");
        lbl.setCharacterSize(11);
        lbl.setFillColor(UITheme::TextDim);
        lbl.setPosition(bounds.left + padX, curY);
        target.draw(lbl);
        curY += 16.0f;

        sf::RectangleShape mathBox({contentWidth, 34.0f});
        mathBox.setPosition(bounds.left + padX, curY);
        mathBox.setFillColor(UITheme::BgInput);
        mathBox.setOutlineThickness(1.0f);
        mathBox.setOutlineColor(UITheme::BorderDark);
        target.draw(mathBox);

        sf::Text val;
        val.setFont(font);
        val.setString(content.inputMathText);
        val.setCharacterSize(13);
        val.setFillColor(UITheme::AccentCyan);
        val.setPosition(bounds.left + padX + 8.0f, curY + 8.0f);
        target.draw(val);
        curY += 42.0f;
    }

    // Result Math Card
    if (!content.resultMathText.empty()) {
        sf::Text lbl;
        lbl.setFont(font);
        lbl.setString("RESULT / TRANSITION:");
        lbl.setCharacterSize(11);
        lbl.setFillColor(UITheme::TextDim);
        lbl.setPosition(bounds.left + padX, curY);
        target.draw(lbl);
        curY += 16.0f;

        sf::RectangleShape mathBox({contentWidth, 34.0f});
        mathBox.setPosition(bounds.left + padX, curY);
        mathBox.setFillColor(UITheme::BgInput);
        mathBox.setOutlineThickness(1.0f);
        mathBox.setOutlineColor(UITheme::BorderDark);
        target.draw(mathBox);

        sf::Text val;
        val.setFont(font);
        val.setString(content.resultMathText);
        val.setCharacterSize(13);
        val.setFillColor(UITheme::AccentGreen);
        val.setPosition(bounds.left + padX + 8.0f, curY + 8.0f);
        target.draw(val);
        curY += 42.0f;
    }

    // Detailed Pedagogical Explanation
    if (!content.explanation.empty()) {
        sf::Text lbl;
        lbl.setFont(font);
        lbl.setString("THEORY & EXPLANATION:");
        lbl.setCharacterSize(11);
        lbl.setFillColor(UITheme::TextDim);
        lbl.setPosition(bounds.left + padX, curY);
        target.draw(lbl);
        curY += 18.0f;

        drawWrappedText(target, content.explanation, bounds.left + padX, curY,
                        contentWidth, 13, UITheme::TextWhite, font);
        curY += 10.0f;
    }

    // Extra details / partition lists / statistics
    if (!content.extraDetails.empty()) {
        curY += 6.0f;
        for (const auto& detail : content.extraDetails) {
            drawWrappedText(target, "• " + detail, bounds.left + padX, curY,
                            contentWidth, 12, UITheme::TextMuted, font);
            curY += 4.0f;
        }
    }

    // Statistics Badge (e.g. Minimized DFA Reduction)
    if (!content.statsText.empty()) {
        curY += 12.0f;
        sf::RectangleShape statBox({contentWidth, 54.0f});
        statBox.setPosition(bounds.left + padX, curY);
        statBox.setFillColor(sf::Color(20, 83, 45, 120));
        statBox.setOutlineThickness(1.0f);
        statBox.setOutlineColor(UITheme::AccentGreen);
        target.draw(statBox);

        sf::Text statHeading;
        statHeading.setFont(font);
        statHeading.setString("MINIMIZATION REDUCTION:");
        statHeading.setCharacterSize(11);
        statHeading.setFillColor(UITheme::AccentGreen);
        statHeading.setPosition(bounds.left + padX + 10.0f, curY + 6.0f);
        target.draw(statHeading);

        sf::Text statVal;
        statVal.setFont(font);
        statVal.setString(content.statsText);
        statVal.setCharacterSize(13);
        statVal.setStyle(sf::Text::Bold);
        statVal.setFillColor(UITheme::TextWhite);
        statVal.setPosition(bounds.left + padX + 10.0f, curY + 26.0f);
        target.draw(statVal);

        curY += 64.0f;
    }

    float totalContentHeight = (curY + scrollOffset) - bounds.top;
    maxScroll = std::max(0.0f, totalContentHeight - bounds.height + 20.0f);

    target.setView(originalView);
}

} // namespace AutomataViz
