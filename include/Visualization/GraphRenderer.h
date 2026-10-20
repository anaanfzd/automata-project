#ifndef AUTOMATAVIZ_GRAPHRENDERER_H
#define AUTOMATAVIZ_GRAPHRENDERER_H

#include "Visualization/Node.h"
#include "Visualization/Edge.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <functional>

namespace AutomataViz {

class GraphRenderer {
public:
    GraphRenderer();

    void setNodesAndEdges(const std::vector<Node>& nodes, const std::vector<Edge>& edges, bool preserveExistingPositions = true);
    void updateNodePositions(const std::vector<Node>& nodes);
    void updateHighlights(int highlightedState, int currentSimState, const std::pair<int, char>& currentSimTrans);

    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::FloatRect& bounds);
    void render(sf::RenderTarget& target, const sf::Font& font, const sf::FloatRect& viewport);

    void zoom(float factor, sf::Vector2f center);
    void resetView(const sf::FloatRect& viewport);
    void pan(float dx, float dy);

    float getZoom() const { return zoomFactor; }
    sf::Vector2f getPanOffset() const { return panOffset; }
    void setPanAndZoom(sf::Vector2f pan, float zoom) { panOffset = pan; zoomFactor = zoom; }

    int getNodeAt(sf::Vector2f screenPos) const;
    int getHoveredNodeId() const { return hoveredNodeId; }
    int getSelectedNodeId() const { return selectedNodeId; }
    void setSelectedNodeId(int id) { selectedNodeId = id; }

    const std::vector<Node>& getNodes() const { return nodes; }
    const std::vector<Edge>& getEdges() const { return edges; }
    std::map<int, sf::Vector2f> getNodePositions() const;

    void setOnNodeClicked(std::function<void(int)> callback) { onNodeClicked = callback; }

private:
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    std::map<int, size_t> stateIdToNodeIndex;

    sf::Vector2f panOffset;
    float zoomFactor;

    // Interaction states
    bool isPanning;
    int draggedNodeId;
    int hoveredNodeId;
    int selectedNodeId;
    sf::Vector2i lastMousePos;
    std::function<void(int)> onNodeClicked;

    // Optimized drawing helpers
    void drawGrid(sf::RenderTarget& target, const sf::FloatRect& viewport);
    void drawEdge(sf::RenderTarget& target, const Edge& edge, const Node& from, const Node& to, const sf::Font& font);
    void drawSelfLoop(sf::RenderTarget& target, const Edge& edge, const Node& node, const sf::Font& font);
    void drawCurvedEdge(sf::RenderTarget& target, const Edge& edge, const Node& from, const Node& to,
                        float curveHeight, const sf::Font& font);
    void drawStraightEdge(sf::RenderTarget& target, const Edge& edge, const Node& from, const Node& to, const sf::Font& font);
    void drawArrowHead(sf::RenderTarget& target, sf::Vector2f tip, sf::Vector2f dir, float size, sf::Color color);
    void drawNode(sf::RenderTarget& target, const Node& node, const sf::Font& font);
    void drawStartIndicator(sf::RenderTarget& target, const Node& node);
    void drawEdgeLabel(sf::RenderTarget& target, sf::Vector2f pos, const std::string& text, const sf::Font& font, bool highlight);
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_GRAPHRENDERER_H
