#ifndef AUTOMATAVIZ_GRAPHLAYOUT_H
#define AUTOMATAVIZ_GRAPHLAYOUT_H

#include "Visualization/Node.h"
#include "Visualization/Edge.h"
#include <vector>
#include <map>

namespace AutomataViz {

class GraphLayout {
public:
    static void computeLayout(std::vector<Node>& nodes, const std::vector<Edge>& edges,
                              int startStateId, float canvasWidth = 800.0f, float canvasHeight = 500.0f);

private:
    static void applyLayeredInitial(std::vector<Node>& nodes, const std::vector<Edge>& edges,
                                    int startStateId, float canvasWidth, float canvasHeight);
    static void applyForceDirectedRelaxation(std::vector<Node>& nodes, const std::vector<Edge>& edges,
                                             int iterations = 100);
    static void centerAndScaleGraph(std::vector<Node>& nodes, float canvasWidth, float canvasHeight);
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_GRAPHLAYOUT_H
