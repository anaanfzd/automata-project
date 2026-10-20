#include "Visualization/GraphRenderer.h"
#include "UI/UITheme.h"
#include <cmath>
#include <algorithm>

namespace AutomataViz {

constexpr float PI = 3.14159265358979323846f;

GraphRenderer::GraphRenderer()
    : panOffset(0.0f, 0.0f), zoomFactor(1.0f),
      isPanning(false), draggedNodeId(-1), hoveredNodeId(-1), selectedNodeId(-1),
      lastMousePos(0, 0) {}

void GraphRenderer::setNodesAndEdges(const std::vector<Node>& newNodes, const std::vector<Edge>& newEdges, bool preserveExistingPositions) {
    std::map<int, sf::Vector2f> existingPositions;
    if (preserveExistingPositions) {
        for (const auto& n : nodes) {
            existingPositions[n.stateId] = n.position;
        }
    }

    nodes = newNodes;
    edges = newEdges;
    stateIdToNodeIndex.clear();

    for (size_t i = 0; i < nodes.size(); ++i) {
        stateIdToNodeIndex[nodes[i].stateId] = i;
        if (preserveExistingPositions) {
            auto it = existingPositions.find(nodes[i].stateId);
            if (it != existingPositions.end()) {
                nodes[i].position = it->second;
            }
        }
    }
}

void GraphRenderer::updateNodePositions(const std::vector<Node>& newNodes) {
    for (const auto& n : newNodes) {
        auto it = stateIdToNodeIndex.find(n.stateId);
        if (it != stateIdToNodeIndex.end()) {
            nodes[it->second].position = n.position;
            nodes[it->second].isHighlighted = n.isHighlighted;
            nodes[it->second].isCurrentSimState = n.isCurrentSimState;
            nodes[it->second].isNewlyAdded = n.isNewlyAdded;
        }
    }
}

void GraphRenderer::updateHighlights(int highlightedState, int currentSimState, const std::pair<int, char>& currentSimTrans) {
    for (auto& node : nodes) {
        node.isHighlighted = (node.stateId == highlightedState);
        node.isCurrentSimState = (node.stateId == currentSimState);
    }

    for (auto& edge : edges) {
        edge.isCurrentSimTransition = (currentSimTrans.first == edge.fromState &&
                                       std::find(edge.symbols.begin(), edge.symbols.end(),
                                                 std::string(1, currentSimTrans.second)) != edge.symbols.end());
    }
}

std::map<int, sf::Vector2f> GraphRenderer::getNodePositions() const {
    std::map<int, sf::Vector2f> res;
    for (const auto& n : nodes) {
        res[n.stateId] = n.position;
    }
    return res;
}

int GraphRenderer::getNodeAt(sf::Vector2f screenPos) const {
    sf::Vector2f worldPos = (screenPos - panOffset) / zoomFactor;
    for (const auto& node : nodes) {
        float dx = worldPos.x - node.position.x;
        float dy = worldPos.y - node.position.y;
        float r = node.radius + 6.0f; // slight padding for easy clicking
        if ((dx * dx + dy * dy) <= (r * r)) {
            return node.stateId;
        }
    }
    return -1;
}

bool GraphRenderer::handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::FloatRect& bounds) {
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        if (bounds.contains(mousePos)) {
            lastMousePos = {event.mouseButton.x, event.mouseButton.y};

            if (event.mouseButton.button == sf::Mouse::Left) {
                int hitNode = getNodeAt(mousePos);
                if (hitNode != -1) {
                    draggedNodeId = hitNode;
                    selectedNodeId = hitNode;
                    if (onNodeClicked) onNodeClicked(hitNode);
                    return true;
                } else {
                    isPanning = true;
                    return true;
                }
            } else if (event.mouseButton.button == sf::Mouse::Middle || event.mouseButton.button == sf::Mouse::Right) {
                isPanning = true;
                return true;
            }
        }
    } else if (event.type == sf::Event::MouseButtonReleased) {
        if (draggedNodeId != -1 || isPanning) {
            draggedNodeId = -1;
            isPanning = false;
            return true;
        }
    } else if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseMove.x, event.mouseMove.y});

        if (bounds.contains(mousePos)) {
            hoveredNodeId = getNodeAt(mousePos);
        } else {
            hoveredNodeId = -1;
        }

        if (draggedNodeId != -1) {
            float dx = static_cast<float>(event.mouseMove.x - lastMousePos.x);
            float dy = static_cast<float>(event.mouseMove.y - lastMousePos.y);
            auto it = stateIdToNodeIndex.find(draggedNodeId);
            if (it != stateIdToNodeIndex.end()) {
                nodes[it->second].position.x += dx / zoomFactor;
                nodes[it->second].position.y += dy / zoomFactor;
            }
            lastMousePos = {event.mouseMove.x, event.mouseMove.y};
            return true;
        } else if (isPanning) {
            float dx = static_cast<float>(event.mouseMove.x - lastMousePos.x);
            float dy = static_cast<float>(event.mouseMove.y - lastMousePos.y);
            panOffset.x += dx;
            panOffset.y += dy;
            lastMousePos = {event.mouseMove.x, event.mouseMove.y};
            return true;
        }
    } else if (event.type == sf::Event::MouseWheelScrolled) {
        sf::Vector2f mousePos = window.mapPixelToCoords({event.mouseWheelScroll.x, event.mouseWheelScroll.y});
        if (bounds.contains(mousePos)) {
            float factor = event.mouseWheelScroll.delta > 0 ? 1.12f : 0.89f;
            zoom(factor, mousePos);
            return true;
        }
    }
    return false;
}

void GraphRenderer::zoom(float factor, sf::Vector2f center) {
    float newZoom = zoomFactor * factor;
    if (newZoom < 0.25f) newZoom = 0.25f;
    if (newZoom > 3.0f) newZoom = 3.0f;

    // Zoom centered on given screen coordinate
    panOffset = center - (center - panOffset) * (newZoom / zoomFactor);
    zoomFactor = newZoom;
}

void GraphRenderer::resetView(const sf::FloatRect& viewport) {
    panOffset = {0.0f, 0.0f};
    zoomFactor = 1.0f;
}

void GraphRenderer::pan(float dx, float dy) {
    panOffset.x += dx;
    panOffset.y += dy;
}

// High performance single-draw-call grid rendering (eliminates thousands of draw calls per frame)
void GraphRenderer::drawGrid(sf::RenderTarget& target, const sf::FloatRect& viewport) {
    float gridSize = 40.0f * zoomFactor;
    while (gridSize < 24.0f) gridSize *= 2.0f;
    while (gridSize > 80.0f) gridSize *= 0.5f;

    float startX = viewport.left + std::fmod(panOffset.x, gridSize);
    if (startX < viewport.left) startX += gridSize;

    float startY = viewport.top + std::fmod(panOffset.y, gridSize);
    if (startY < viewport.top) startY += gridSize;

    int countX = static_cast<int>((viewport.left + viewport.width - startX) / gridSize) + 2;
    int countY = static_cast<int>((viewport.top + viewport.height - startY) / gridSize) + 2;
    if (countX <= 0 || countY <= 0) return;

    sf::VertexArray gridPoints(sf::Points, countX * countY);
    size_t idx = 0;
    for (float x = startX; x < viewport.left + viewport.width; x += gridSize) {
        for (float y = startY; y < viewport.top + viewport.height; y += gridSize) {
            gridPoints[idx].position = {x, y};
            gridPoints[idx].color = UITheme::GridDot;
            idx++;
        }
    }
    gridPoints.resize(idx);
    target.draw(gridPoints);
}

void GraphRenderer::drawArrowHead(sf::RenderTarget& target, sf::Vector2f tip, sf::Vector2f dir, float size, sf::Color color) {
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.001f) return;
    dir /= len;

    sf::Vector2f normal(-dir.y, dir.x);
    sf::Vector2f p1 = tip - dir * size + normal * (size * 0.45f);
    sf::Vector2f p2 = tip - dir * size - normal * (size * 0.45f);

    sf::ConvexShape arrow(3);
    arrow.setPoint(0, tip);
    arrow.setPoint(1, p1);
    arrow.setPoint(2, p2);
    arrow.setFillColor(color);
    target.draw(arrow);
}

void GraphRenderer::drawEdgeLabel(sf::RenderTarget& target, sf::Vector2f pos, const std::string& text,
                                 const sf::Font& font, bool highlight) {
    sf::Text labelText;
    labelText.setFont(font);
    labelText.setString(text);
    labelText.setCharacterSize(static_cast<unsigned int>(std::max(10.0f, 13.0f * zoomFactor)));
    labelText.setFillColor(highlight ? UITheme::AccentAmber : UITheme::TextWhite);

    sf::FloatRect textBounds = labelText.getLocalBounds();
    labelText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    labelText.setPosition(pos);

    // Pill background
    float padX = 7.0f * zoomFactor;
    float padY = 4.0f * zoomFactor;
    sf::RectangleShape badge({textBounds.width + padX * 2, textBounds.height + padY * 2});
    badge.setOrigin((textBounds.width + padX * 2) / 2.0f, (textBounds.height + padY * 2) / 2.0f);
    badge.setPosition(pos);
    badge.setFillColor(UITheme::EdgeLabelBg);
    badge.setOutlineThickness(1.5f);
    badge.setOutlineColor(highlight ? UITheme::AccentAmber : UITheme::EdgeLabelBorder);

    target.draw(badge);
    target.draw(labelText);
}

void GraphRenderer::drawStraightEdge(sf::RenderTarget& target, const Edge& edge, const Node& from, const Node& to, const sf::Font& font) {
    sf::Vector2f p1 = from.position * zoomFactor + panOffset;
    sf::Vector2f p2 = to.position * zoomFactor + panOffset;

    sf::Vector2f diff = p2 - p1;
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    if (dist < 0.001f) return;

    sf::Vector2f dir = diff / dist;
    float rFrom = from.radius * zoomFactor;
    float rTo = to.radius * zoomFactor;

    sf::Vector2f start = p1 + dir * rFrom;
    sf::Vector2f end = p2 - dir * rTo;

    sf::Color edgeColor = edge.isCurrentSimTransition ? UITheme::AccentCyan :
                          (edge.isHighlighted ? UITheme::AccentAmber :
                          (edge.isNewlyAdded ? UITheme::AccentGreen : UITheme::EdgeNormal));

    sf::Vertex line[] = {
        sf::Vertex(start, edgeColor),
        sf::Vertex(end, edgeColor)
    };
    target.draw(line, 2, sf::Lines);

    drawArrowHead(target, end, dir, 12.0f * zoomFactor, edgeColor);

    sf::Vector2f mid = (start + end) / 2.0f;
    drawEdgeLabel(target, mid, edge.getDisplayLabel(), font, edge.isHighlighted || edge.isCurrentSimTransition);
}

// Zero-allocation curved edge evaluation
void GraphRenderer::drawCurvedEdge(sf::RenderTarget& target, const Edge& edge, const Node& from, const Node& to,
                                  float curveHeight, const sf::Font& font) {
    sf::Vector2f p1 = from.position * zoomFactor + panOffset;
    sf::Vector2f p2 = to.position * zoomFactor + panOffset;

    sf::Vector2f diff = p2 - p1;
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    if (dist < 0.001f) return;

    sf::Vector2f dir = diff / dist;
    sf::Vector2f normal(-dir.y, dir.x);
    sf::Vector2f mid = (p1 + p2) / 2.0f + normal * (curveHeight * zoomFactor);

    constexpr int segments = 24;
    sf::Vector2f curvePoints[segments + 1];

    for (int i = 0; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;
        float u = 1.0f - t;
        curvePoints[i] = u * u * p1 + 2.0f * u * t * mid + t * t * p2;
    }

    float rFrom = from.radius * zoomFactor;
    float rTo = to.radius * zoomFactor;

    int startIdx = 0;
    while (startIdx < segments && std::hypot(curvePoints[startIdx].x - p1.x, curvePoints[startIdx].y - p1.y) < rFrom) {
        startIdx++;
    }

    int endIdx = segments;
    while (endIdx > 0 && std::hypot(curvePoints[endIdx].x - p2.x, curvePoints[endIdx].y - p2.y) < rTo) {
        endIdx--;
    }

    if (startIdx >= endIdx) return;

    sf::Color edgeColor = edge.isCurrentSimTransition ? UITheme::AccentCyan :
                          (edge.isHighlighted ? UITheme::AccentAmber :
                          (edge.isNewlyAdded ? UITheme::AccentGreen : UITheme::EdgeNormal));

    sf::VertexArray va(sf::LineStrip, endIdx - startIdx + 1);
    for (int i = startIdx; i <= endIdx; ++i) {
        va[i - startIdx].position = curvePoints[i];
        va[i - startIdx].color = edgeColor;
    }
    target.draw(va);

    sf::Vector2f arrowDir = curvePoints[endIdx] - curvePoints[std::max(0, endIdx - 2)];
    drawArrowHead(target, curvePoints[endIdx], arrowDir, 12.0f * zoomFactor, edgeColor);

    drawEdgeLabel(target, mid, edge.getDisplayLabel(), font, edge.isHighlighted || edge.isCurrentSimTransition);
}

void GraphRenderer::drawSelfLoop(sf::RenderTarget& target, const Edge& edge, const Node& node, const sf::Font& font) {
    sf::Vector2f center = node.position * zoomFactor + panOffset;
    float r = node.radius * zoomFactor;

    float loopR = r * 0.75f;
    sf::Vector2f loopCenter = center + sf::Vector2f(0.0f, -r - loopR * 0.8f);

    constexpr int segments = 28;
    sf::VertexArray va(sf::LineStrip, segments + 1);

    sf::Color edgeColor = edge.isCurrentSimTransition ? UITheme::AccentCyan :
                          (edge.isHighlighted ? UITheme::AccentAmber :
                          (edge.isNewlyAdded ? UITheme::AccentGreen : UITheme::EdgeNormal));

    for (int i = 0; i <= segments; ++i) {
        float angle = PI * 0.15f + (PI * 1.7f) * (static_cast<float>(i) / segments);
        float x = loopCenter.x + std::cos(angle) * loopR;
        float y = loopCenter.y + std::sin(angle) * loopR;
        va[i].position = {x, y};
        va[i].color = edgeColor;
    }
    target.draw(va);

    sf::Vector2f tip = va[segments].position;
    sf::Vector2f dir = tip - va[segments - 2].position;
    drawArrowHead(target, tip, dir, 11.0f * zoomFactor, edgeColor);

    sf::Vector2f labelPos = loopCenter + sf::Vector2f(0.0f, -loopR - 10.0f * zoomFactor);
    drawEdgeLabel(target, labelPos, edge.getDisplayLabel(), font, edge.isHighlighted || edge.isCurrentSimTransition);
}

void GraphRenderer::drawEdge(sf::RenderTarget& target, const Edge& edge, const Node& from, const Node& to, const sf::Font& font) {
    if (edge.isSelfLoop()) {
        drawSelfLoop(target, edge, from, font);
    } else if (edge.isBidirectional) {
        drawCurvedEdge(target, edge, from, to, 35.0f, font);
    } else {
        drawStraightEdge(target, edge, from, to, font);
    }
}

void GraphRenderer::drawStartIndicator(sf::RenderTarget& target, const Node& node) {
    sf::Vector2f center = node.position * zoomFactor + panOffset;
    float r = node.radius * zoomFactor;

    sf::Vector2f tip = center + sf::Vector2f(-r - 2.0f, 0.0f);
    sf::Vector2f start = tip + sf::Vector2f(-32.0f * zoomFactor, 0.0f);

    sf::Vertex line[] = {
        sf::Vertex(start, UITheme::NodeStartBorder),
        sf::Vertex(tip, UITheme::NodeStartBorder)
    };
    target.draw(line, 2, sf::Lines);
    drawArrowHead(target, tip, {1.0f, 0.0f}, 12.0f * zoomFactor, UITheme::NodeStartBorder);
}

void GraphRenderer::drawNode(sf::RenderTarget& target, const Node& node, const sf::Font& font) {
    sf::Vector2f center = node.position * zoomFactor + panOffset;
    float r = node.radius * zoomFactor;

    bool isHovered = (node.stateId == hoveredNodeId);
    bool isSelected = (node.stateId == selectedNodeId);

    sf::Color fillColor = UITheme::NodeDefaultFill;
    sf::Color borderColor = UITheme::NodeDefaultBorder;

    if (node.isCurrentSimState) {
        fillColor = UITheme::NodeActiveFill;
        borderColor = UITheme::AccentCyan;
    } else if (node.isHighlighted) {
        fillColor = sf::Color(120, 53, 15);
        borderColor = UITheme::AccentAmber;
    } else if (node.isNewlyAdded) {
        fillColor = UITheme::NodeNewFill;
        borderColor = UITheme::NodeNewBorder;
    } else if (isSelected) {
        fillColor = sf::Color(30, 58, 138);
        borderColor = UITheme::AccentCyan;
    } else if (isHovered) {
        fillColor = sf::Color(45, 60, 85);
        borderColor = UITheme::AccentCyan;
    } else if (node.isFinal) {
        borderColor = UITheme::NodeFinalBorder;
    } else if (node.isStart) {
        borderColor = UITheme::NodeStartBorder;
    }

    // Outer glow for active/highlighted/hovered nodes
    if (node.isCurrentSimState || node.isHighlighted || isHovered || isSelected) {
        float glowPad = (node.isCurrentSimState || node.isHighlighted) ? 8.0f : 4.0f;
        sf::CircleShape glow(r + glowPad * zoomFactor);
        glow.setOrigin(r + glowPad * zoomFactor, r + glowPad * zoomFactor);
        glow.setPosition(center);
        glow.setFillColor(sf::Color(borderColor.r, borderColor.g, borderColor.b, isHovered ? 90 : 60));
        target.draw(glow);
    }

    // Main Circle
    sf::CircleShape circle(r);
    circle.setOrigin(r, r);
    circle.setPosition(center);
    circle.setFillColor(fillColor);
    circle.setOutlineThickness((isHovered || isSelected ? 3.0f : 2.5f) * zoomFactor);
    circle.setOutlineColor(borderColor);
    target.draw(circle);

    // Final state inner double circle
    if (node.isFinal) {
        float innerR = r - 6.0f * zoomFactor;
        if (innerR > 4.0f) {
            sf::CircleShape inner(innerR);
            inner.setOrigin(innerR, innerR);
            inner.setPosition(center);
            inner.setFillColor(sf::Color::Transparent);
            inner.setOutlineThickness(2.0f * zoomFactor);
            inner.setOutlineColor(borderColor);
            target.draw(inner);
        }
    }

    // Start state indicator arrow
    if (node.isStart) {
        drawStartIndicator(target, node);
    }

    // Primary Label Text
    sf::Text text;
    text.setFont(font);
    text.setString(node.label);
    text.setCharacterSize(static_cast<unsigned int>(std::max(10.0f, 15.0f * zoomFactor)));
    text.setFillColor(UITheme::TextWhite);

    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    text.setPosition(center);
    target.draw(text);

    // Sublabel (e.g. subset {q0,q1})
    if (!node.subLabel.empty()) {
        sf::Text subText;
        subText.setFont(font);
        subText.setString(node.subLabel);
        subText.setCharacterSize(static_cast<unsigned int>(std::max(9.0f, 11.0f * zoomFactor)));
        subText.setFillColor(UITheme::TextMuted);

        sf::FloatRect subBounds = subText.getLocalBounds();
        subText.setOrigin(subBounds.left + subBounds.width / 2.0f, subBounds.top);
        subText.setPosition(center.x, center.y + r + 4.0f * zoomFactor);
        target.draw(subText);
    }
}

void GraphRenderer::render(sf::RenderTarget& target, const sf::Font& font, const sf::FloatRect& viewport) {
    sf::View originalView = target.getView();

    // Create scissor / clip view for graph viewport
    sf::View graphView(viewport);
    graphView.setViewport(sf::FloatRect(
        viewport.left / originalView.getSize().x,
        viewport.top / originalView.getSize().y,
        viewport.width / originalView.getSize().x,
        viewport.height / originalView.getSize().y
    ));

    target.setView(graphView);

    // Background fill
    sf::RectangleShape bg({viewport.width, viewport.height});
    bg.setPosition(viewport.left, viewport.top);
    bg.setFillColor(UITheme::BgDark);
    target.draw(bg);

    // Batched single-draw blueprint grid
    drawGrid(target, viewport);

    // Draw Edges
    for (const auto& edge : edges) {
        auto itFrom = stateIdToNodeIndex.find(edge.fromState);
        auto itTo = stateIdToNodeIndex.find(edge.toState);
        if (itFrom != stateIdToNodeIndex.end() && itTo != stateIdToNodeIndex.end()) {
            drawEdge(target, edge, nodes[itFrom->second], nodes[itTo->second], font);
        }
    }

    // Draw Nodes
    for (const auto& node : nodes) {
        drawNode(target, node, font);
    }

    target.setView(originalView);
}

} // namespace AutomataViz
