#include "UI/TableView.h"
#include "UI/UITheme.h"
#include <algorithm>

namespace AutomataViz {

TableView::TableView(const sf::FloatRect& bounds)
    : bounds(bounds), scrollOffsetY(0.0f), maxScrollY(0.0f) {}

void TableView::setDFA(const DFA& dfa) {
    rows = dfa.getTransitionTable();
    alphabetSorted.clear();
    for (char c : dfa.getAlphabet()) {
        alphabetSorted.push_back(c);
    }
    std::sort(alphabetSorted.begin(), alphabetSorted.end());
    scrollOffsetY = 0.0f;
}

void TableView::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseWheelScrolled) {
        sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseWheelScroll.x, event.mouseWheelScroll.y});
        if (bounds.contains(mousePos)) {
            scrollOffsetY -= event.mouseWheelScroll.delta * 24.0f;
            if (scrollOffsetY < 0.0f) scrollOffsetY = 0.0f;
            if (scrollOffsetY > maxScrollY) scrollOffsetY = maxScrollY;
        }
    }
}

void TableView::render(sf::RenderTarget& target, const sf::Font& font) {
    sf::View originalView = target.getView();
    sf::View tableViewPort(bounds);
    tableViewPort.setViewport(sf::FloatRect(
        bounds.left / originalView.getSize().x,
        bounds.top / originalView.getSize().y,
        bounds.width / originalView.getSize().x,
        bounds.height / originalView.getSize().y
    ));
    target.setView(tableViewPort);

    // Background
    sf::RectangleShape bg({bounds.width, bounds.height});
    bg.setPosition(bounds.left, bounds.top);
    bg.setFillColor(UITheme::BgDark);
    target.draw(bg);

    const float startX = bounds.left + 24.0f;
    const float startY = bounds.top + 24.0f - scrollOffsetY;
    const float rowHeight = 36.0f;
    const float colWidthState = 120.0f;
    const float colWidthSubset = 220.0f;
    const float colWidthSymbol = 100.0f;

    float totalTableWidth = colWidthState + colWidthSubset + alphabetSorted.size() * colWidthSymbol;

    // Header Background
    sf::RectangleShape headerBg({totalTableWidth, rowHeight});
    headerBg.setPosition(startX, startY);
    headerBg.setFillColor(sf::Color(30, 58, 138, 200));
    headerBg.setOutlineThickness(1.0f);
    headerBg.setOutlineColor(UITheme::BorderDark);
    target.draw(headerBg);

    // Header Labels
    auto drawCellText = [&](const std::string& text, float x, float y, float w, sf::Color col, bool bold = false) {
        sf::Text txt;
        txt.setFont(font);
        txt.setString(text);
        txt.setCharacterSize(14);
        txt.setFillColor(col);
        if (bold) txt.setStyle(sf::Text::Bold);
        sf::FloatRect b = txt.getLocalBounds();
        txt.setOrigin(b.left, b.top + b.height / 2.0f);
        txt.setPosition(x + 12.0f, y + rowHeight / 2.0f);
        target.draw(txt);
    };

    drawCellText("DFA State", startX, startY, colWidthState, UITheme::TextWhite, true);
    drawCellText("NFA Subset Q'", startX + colWidthState, startY, colWidthSubset, UITheme::TextWhite, true);

    for (size_t col = 0; col < alphabetSorted.size(); ++col) {
        float cx = startX + colWidthState + colWidthSubset + col * colWidthSymbol;
        std::string colHeader = "δ (q, '" + std::string(1, alphabetSorted[col]) + "')";
        drawCellText(colHeader, cx, startY, colWidthSymbol, UITheme::AccentCyan, true);
    }

    // Rows
    float currentY = startY + rowHeight;
    for (size_t r = 0; r < rows.size(); ++r) {
        const auto& row = rows[r];

        sf::RectangleShape rowBg({totalTableWidth, rowHeight});
        rowBg.setPosition(startX, currentY);
        rowBg.setFillColor((r % 2 == 0) ? UITheme::BgPanel : UITheme::BgPanelSecondary);
        rowBg.setOutlineThickness(0.5f);
        rowBg.setOutlineColor(UITheme::BorderDark);
        target.draw(rowBg);

        // State indicator: -> for start, * for final
        std::string statePrefix = "";
        if (row.isStart) statePrefix += "-> ";
        if (row.isFinal) statePrefix += "* ";
        std::string fullStateLabel = statePrefix + row.label;

        sf::Color stateColor = row.isFinal ? UITheme::AccentGreen :
                              (row.isStart ? UITheme::AccentCyan : UITheme::TextWhite);

        drawCellText(fullStateLabel, startX, currentY, colWidthState, stateColor, row.isStart || row.isFinal);
        drawCellText(row.subsetString, startX + colWidthState, currentY, colWidthSubset, UITheme::TextMuted, false);

        for (size_t col = 0; col < alphabetSorted.size(); ++col) {
            float cx = startX + colWidthState + colWidthSubset + col * colWidthSymbol;
            char sym = alphabetSorted[col];

            std::string destLabel = "-";
            sf::Color destColor = UITheme::TextDim;

            auto it = row.transitions.find(sym);
            if (it != row.transitions.end() && it->second != -1) {
                for (const auto& rSearch : rows) {
                    if (rSearch.stateId == it->second) {
                        destLabel = rSearch.label;
                        destColor = UITheme::TextWhite;
                        break;
                    }
                }
            }

            drawCellText(destLabel, cx, currentY, colWidthSymbol, destColor, false);
        }

        currentY += rowHeight;
    }

    float totalTableHeight = (currentY + scrollOffsetY) - bounds.top;
    maxScrollY = std::max(0.0f, totalTableHeight - bounds.height + 40.0f);

    target.setView(originalView);
}

} // namespace AutomataViz
