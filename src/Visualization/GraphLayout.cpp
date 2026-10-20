#include "Visualization/GraphLayout.h"
#include <queue>
#include <cmath>
#include <algorithm>

namespace AutomataViz {

void GraphLayout::computeLayout(std::vector<Node>& nodes, const std::vector<Edge>& edges,
                              int startStateId, float canvasWidth, float canvasHeight) {
    if (nodes.empty()) return;

    applyLayeredInitial(nodes, edges, startStateId, canvasWidth, canvasHeight);
    applyForceDirectedRelaxation(nodes, edges, 90);
    centerAndScaleGraph(nodes, canvasWidth, canvasHeight);
}

void GraphLayout::applyLayeredInitial(std::vector<Node>& nodes, const std::vector<Edge>& edges,
                                    int startStateId, float canvasWidth, float canvasHeight) {
    std::map<int, size_t> idToIndex;
    for (size_t i = 0; i < nodes.size(); ++i) {
        idToIndex[nodes[i].stateId] = i;
    }

    size_t startIdx = 0;
    auto itStart = idToIndex.find(startStateId);
    if (itStart != idToIndex.end()) {
        startIdx = itStart->second;
    }

    // BFS to assign layers (depth from start node)
    std::vector<int> depth(nodes.size(), -1);
    std::queue<size_t> q;

    depth[startIdx] = 0;
    q.push(startIdx);

    while (!q.empty()) {
        size_t u = q.front();
        q.pop();

        for (const auto& edge : edges) {
            if (edge.fromState == nodes[u].stateId && !edge.isSelfLoop()) {
                auto itV = idToIndex.find(edge.toState);
                if (itV != idToIndex.end()) {
                    size_t v = itV->second;
                    if (depth[v] == -1) {
                        depth[v] = depth[u] + 1;
                        q.push(v);
                    }
                }
            }
        }
    }

    // Assign layers to unreached nodes
    int maxDepth = 0;
    for (int d : depth) {
        if (d > maxDepth) maxDepth = d;
    }

    for (size_t i = 0; i < nodes.size(); ++i) {
        if (depth[i] == -1) {
            depth[i] = ++maxDepth;
        }
    }

    // Group nodes by layer
    std::map<int, std::vector<size_t>> layerNodes;
    for (size_t i = 0; i < nodes.size(); ++i) {
        layerNodes[depth[i]].push_back(i);
    }

    // Position each layer with clean horizontal and vertical spacing
    const float layerSpacingX = 180.0f;
    const float nodeSpacingY = 110.0f;

    for (const auto& kv : layerNodes) {
        int d = kv.first;
        const auto& indices = kv.second;
        float totalHeight = (static_cast<float>(indices.size()) - 1.0f) * nodeSpacingY;
        float startY = -totalHeight / 2.0f;

        for (size_t idx = 0; idx < indices.size(); ++idx) {
            size_t nodeIdx = indices[idx];
            nodes[nodeIdx].position.x = d * layerSpacingX;
            nodes[nodeIdx].position.y = startY + idx * nodeSpacingY;
        }
    }
}

void GraphLayout::applyForceDirectedRelaxation(std::vector<Node>& nodes, const std::vector<Edge>& edges,
                                             int iterations) {
    if (nodes.size() <= 1) return;

    std::map<int, size_t> idToIndex;
    for (size_t i = 0; i < nodes.size(); ++i) {
        idToIndex[nodes[i].stateId] = i;
    }

    const float kRepulse = 180.0f;
    const float kSpring = 150.0f;
    float temp = 15.0f;
    const float damping = 0.95f;

    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<sf::Vector2f> disp(nodes.size(), {0.0f, 0.0f});

        // 1. Repulsion between all node pairs
        for (size_t i = 0; i < nodes.size(); ++i) {
            for (size_t j = i + 1; j < nodes.size(); ++j) {
                float dx = nodes[i].position.x - nodes[j].position.x;
                float dy = nodes[i].position.y - nodes[j].position.y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < 1.0f) {
                    dx = 1.0f;
                    dy = 1.0f;
                    dist = 1.414f;
                }

                if (dist < 400.0f) {
                    float force = (kRepulse * kRepulse) / dist;
                    float fx = (dx / dist) * force;
                    float fy = (dy / dist) * force;

                    disp[i].x += fx;
                    disp[i].y += fy;
                    disp[j].x -= fx;
                    disp[j].y -= fy;
                }
            }
        }

        // 2. Attraction along edges
        for (const auto& edge : edges) {
            if (edge.isSelfLoop()) continue;
            auto itU = idToIndex.find(edge.fromState);
            auto itV = idToIndex.find(edge.toState);
            if (itU == idToIndex.end() || itV == idToIndex.end()) continue;

            size_t u = itU->second;
            size_t v = itV->second;

            float dx = nodes[v].position.x - nodes[u].position.x;
            float dy = nodes[v].position.y - nodes[u].position.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 1.0f) dist = 1.0f;

            float force = (dist * dist) / kSpring;
            float fx = (dx / dist) * force;
            float fy = (dy / dist) * force;

            // Reduce horizontal pull to preserve left-to-right topological order
            disp[u].x += fx * 0.3f;
            disp[u].y += fy * 0.8f;
            disp[v].x -= fx * 0.3f;
            disp[v].y -= fy * 0.8f;
        }

        // 3. Apply displacements capped by temperature
        for (size_t i = 0; i < nodes.size(); ++i) {
            float len = std::sqrt(disp[i].x * disp[i].x + disp[i].y * disp[i].y);
            if (len > 0.001f) {
                float cap = std::min(len, temp);
                nodes[i].position.x += (disp[i].x / len) * cap;
                nodes[i].position.y += (disp[i].y / len) * cap;
            }
        }

        temp *= damping;
    }
}

void GraphLayout::centerAndScaleGraph(std::vector<Node>& nodes, float canvasWidth, float canvasHeight) {
    if (nodes.empty()) return;

    float minX = nodes[0].position.x;
    float maxX = nodes[0].position.x;
    float minY = nodes[0].position.y;
    float maxY = nodes[0].position.y;

    for (const auto& n : nodes) {
        if (n.position.x < minX) minX = n.position.x;
        if (n.position.x > maxX) maxX = n.position.x;
        if (n.position.y < minY) minY = n.position.y;
        if (n.position.y > maxY) maxY = n.position.y;
    }

    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;

    // Desired center of canvas
    float targetX = canvasWidth / 2.0f;
    float targetY = canvasHeight / 2.0f;

    for (auto& n : nodes) {
        n.position.x += (targetX - centerX);
        n.position.y += (targetY - centerY);
    }
}

} // namespace AutomataViz
