#include "UI/MainWindow.h"
#include "UI/UITheme.h"
#include "Visualization/GraphLayout.h"
#include <iostream>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace AutomataViz {

MainWindow::MainWindow(unsigned int width, unsigned int height)
    : windowWidth(width), windowHeight(height),
      currentRegexStr("(a|b)*abb"),
      currentMode(ViewMode::THOMPSON_NFA),
      dfaDisplayType(DFADisplayType::GRAPH_VIEW),
      currentThompsonStep(0),
      currentSubsetStep(0),
      currentMinimizationStep(0),
      isDualView(false),
      speedLevel(0),
      theoryScrollY(0.0f),
      maxTheoryScrollY(0.0f),
      nfaFullLayoutComputed(false),
      dfaFullLayoutComputed(false),
      minDfaLayoutComputed(false) {}

MainWindow::~MainWindow() = default;

bool MainWindow::loadFonts() {
    const std::vector<std::string> fontCandidates = {
        "assets/fonts/Inter-Regular.ttf",
        "assets/fonts/Roboto-Regular.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    };

    bool loaded = false;
    for (const auto& path : fontCandidates) {
        if (font.loadFromFile(path)) {
            loaded = true;
            break;
        }
    }

    const std::vector<std::string> monoCandidates = {
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/cour.ttf",
        "C:/Windows/Fonts/lucon.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
    };

    for (const auto& path : monoCandidates) {
        if (fontMono.loadFromFile(path)) {
            break;
        }
    }

    return loaded;
}

void MainWindow::setupLayout() {
    float w = static_cast<float>(windowWidth);
    float h = static_cast<float>(windowHeight);

    float sidePanelWidth = 360.0f;
    float topBarHeight = 76.0f;
    float tabBarHeight = 40.0f;
    float controlBarHeight = 44.0f;
    float simBarHeight = 72.0f;

    topBarBounds = {0.0f, 0.0f, w, topBarHeight};
    tabBarBounds = {0.0f, topBarHeight, w, tabBarHeight};
    
    float mainCanvasWidth = w - sidePanelWidth;
    controlBarBounds = {0.0f, topBarHeight + tabBarHeight, mainCanvasWidth, controlBarHeight};

    float canvasTop = topBarHeight + tabBarHeight + controlBarHeight;
    float canvasH = h - canvasTop - (currentMode == ViewMode::STRING_SIMULATION ? simBarHeight : 0.0f);
    canvasBounds = {0.0f, canvasTop, mainCanvasWidth, canvasH};

    // Dual view split geometry
    float halfWidth = (mainCanvasWidth - 8.0f) * 0.5f;
    leftCanvasBounds = {0.0f, canvasTop, halfWidth, canvasH};
    rightCanvasBounds = {halfWidth + 8.0f, canvasTop, halfWidth, canvasH};

    simBarBounds = {0.0f, h - simBarHeight, mainCanvasWidth, simBarHeight};
    infoPanelBounds = {mainCanvasWidth, topBarHeight + tabBarHeight, sidePanelWidth, h - (topBarHeight + tabBarHeight)};

    infoPanel.setBounds(infoPanelBounds);
    tableView.setBounds(canvasBounds);
    modeTabBar.setBounds(tabBarBounds);
}

void MainWindow::setupUI() {
    setupLayout();

    // Regex Input Bar
    regexInput.setBounds({140.0f, 18.0f, 320.0f, 38.0f});
    regexInput.setText(currentRegexStr);
    regexInput.setPlaceholder("e.g. (a|b)*abb");
    regexInput.setOnSubmit([this](const std::string& val) {
        executePipeline(val);
    });

    generateBtn.setBounds({470.0f, 18.0f, 120.0f, 38.0f});
    generateBtn.setLabel("Generate ▶");
    generateBtn.setColors(UITheme::AccentBlue, sf::Color(59, 130, 246), UITheme::TextWhite);
    generateBtn.setOnClick([this]() {
        executePipeline(regexInput.getText());
    });

    // Preset Example Buttons
    const std::vector<std::string> presets = {
        "(a|b)*abb",
        "a*",
        "(a|b)*",
        "(a|b)(a|b)",
        "a(b|c)*d"
    };

    float exX = 610.0f;
    for (size_t i = 0; i < presets.size(); ++i) {
        Button btn(presets[i], {exX, 20.0f, 96.0f, 34.0f});
        btn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
        btn.setOnClick([this, i]() {
            handleExampleSelect(static_cast<int>(i));
        });
        exampleButtons.push_back(btn);
        exX += 102.0f;
    }

    // Tabs
    modeTabBar.addTab("1. Postfix / AST");
    modeTabBar.addTab("2. ε-NFA (Thompson)");
    modeTabBar.addTab("3. Subset Construction");
    modeTabBar.addTab("4. DFA (Graph/Table)");
    modeTabBar.addTab("5. Minimized DFA");
    modeTabBar.addTab("6. String Simulation");
    modeTabBar.addTab("7. TOC Course Guide");

    modeTabBar.setSelectedTab(static_cast<int>(currentMode));
    modeTabBar.setOnTabChanged([this](int index) {
        currentMode = static_cast<ViewMode>(index);
        setupLayout();
        updateGraphForCurrentState();
        updateInfoPanel();
    });

    // Step Navigation Controls
    float btnY = controlBarBounds.top + 6.0f;
    prevStepBtn.setBounds({14.0f, btnY, 105.0f, 32.0f});
    prevStepBtn.setLabel("◀ Prev Step");
    prevStepBtn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
    prevStepBtn.setOnClick([this]() { goToPreviousStep(); });

    nextStepBtn.setBounds({124.0f, btnY, 105.0f, 32.0f});
    nextStepBtn.setLabel("Next Step ▶");
    nextStepBtn.setColors(UITheme::AccentBlue, sf::Color(59, 130, 246), UITheme::TextWhite);
    nextStepBtn.setOnClick([this]() { goToNextStep(); });

    playPauseBtn.setBounds({234.0f, btnY, 100.0f, 32.0f});
    playPauseBtn.setLabel("▶ Auto Play");
    playPauseBtn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
    playPauseBtn.setOnClick([this]() {
        animationCtrl.togglePlayPause();
        playPauseBtn.setLabel(animationCtrl.isPlaying() ? "⏸ Pause" : "▶ Auto Play");
        playPauseBtn.setColors(animationCtrl.isPlaying() ? UITheme::AccentAmber : UITheme::BgPanel,
                               sf::Color(51, 65, 85), UITheme::TextWhite);
    });

    resetStepBtn.setBounds({339.0f, btnY, 75.0f, 32.0f});
    resetStepBtn.setLabel("↺ Reset");
    resetStepBtn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
    resetStepBtn.setOnClick([this]() { resetCurrentStep(); });

    speedBtn.setBounds({419.0f, btnY, 95.0f, 32.0f});
    speedBtn.setLabel("Speed: 1x");
    speedBtn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
    speedBtn.setOnClick([this]() { cycleSpeed(); });

    // Viewport Controls on the Right
    float rightBtnX = controlBarBounds.left + controlBarBounds.width - 85.0f;
    resetViewBtn.setBounds({rightBtnX, btnY, 78.0f, 32.0f});
    resetViewBtn.setLabel("Fit View");
    resetViewBtn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
    resetViewBtn.setOnClick([this]() {
        graphRenderer.resetView(canvasBounds);
        nfaSideRenderer.resetView(leftCanvasBounds);
    });

    zoomOutBtn.setBounds({rightBtnX - 76.0f, btnY, 70.0f, 32.0f});
    zoomOutBtn.setLabel("Zoom -");
    zoomOutBtn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
    zoomOutBtn.setOnClick([this]() {
        graphRenderer.zoom(0.85f, {canvasBounds.left + canvasBounds.width / 2.0f, canvasBounds.top + canvasBounds.height / 2.0f});
    });

    zoomInBtn.setBounds({rightBtnX - 152.0f, btnY, 70.0f, 32.0f});
    zoomInBtn.setLabel("Zoom +");
    zoomInBtn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
    zoomInBtn.setOnClick([this]() {
        graphRenderer.zoom(1.18f, {canvasBounds.left + canvasBounds.width / 2.0f, canvasBounds.top + canvasBounds.height / 2.0f});
    });

    toggleTableBtn.setBounds({rightBtnX - 262.0f, btnY, 102.0f, 32.0f});
    toggleTableBtn.setLabel("Table View");
    toggleTableBtn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
    toggleTableBtn.setOnClick([this]() {
        if (dfaDisplayType == DFADisplayType::GRAPH_VIEW) {
            dfaDisplayType = DFADisplayType::TABLE_VIEW;
            toggleTableBtn.setLabel("Graph View");
        } else {
            dfaDisplayType = DFADisplayType::GRAPH_VIEW;
            toggleTableBtn.setLabel("Table View");
        }
    });

    toggleDualViewBtn.setBounds({rightBtnX - 268.0f, btnY, 110.0f, 32.0f});
    toggleDualViewBtn.setLabel("Split View: OFF");
    toggleDualViewBtn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
    toggleDualViewBtn.setOnClick([this]() {
        isDualView = !isDualView;
        toggleDualViewBtn.setLabel(isDualView ? "Split View: ON" : "Split View: OFF");
        toggleDualViewBtn.setColors(isDualView ? UITheme::AccentBlue : UITheme::BgPanel,
                                   sf::Color(51, 65, 85), UITheme::TextWhite);
        updateGraphForCurrentState();
    });

    // Simulation Bar Controls
    simStringInput.setBounds({14.0f, simBarBounds.top + 16.0f, 220.0f, 38.0f});
    simStringInput.setPlaceholder("Enter string, e.g. aabb");
    simStringInput.setText("aabb");
    simStringInput.setOnSubmit([this](const std::string& str) {
        stringSimulator.setInputString(str);
        updateGraphForCurrentState();
        updateInfoPanel();
    });

    simStepBtn.setBounds({242.0f, simBarBounds.top + 16.0f, 100.0f, 38.0f});
    simStepBtn.setLabel("Step Sim ▶");
    simStepBtn.setColors(UITheme::AccentBlue, sf::Color(59, 130, 246), UITheme::TextWhite);
    simStepBtn.setOnClick([this]() {
        stringSimulator.stepForward();
        updateGraphForCurrentState();
        updateInfoPanel();
    });

    simRunBtn.setBounds({348.0f, simBarBounds.top + 16.0f, 100.0f, 38.0f});
    simRunBtn.setLabel("Run All ⏩");
    simRunBtn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
    simRunBtn.setOnClick([this]() {
        stringSimulator.runToEnd();
        updateGraphForCurrentState();
        updateInfoPanel();
    });

    simResetBtn.setBounds({454.0f, simBarBounds.top + 16.0f, 80.0f, 38.0f});
    simResetBtn.setLabel("↺ Reset");
    simResetBtn.setColors(UITheme::BgPanel, sf::Color(51, 65, 85), UITheme::TextWhite);
    simResetBtn.setOnClick([this]() {
        stringSimulator.reset();
        updateGraphForCurrentState();
        updateInfoPanel();
    });

    animationCtrl.setOnStepTrigger([this]() {
        goToNextStep();
    });
}

void MainWindow::cycleSpeed() {
    speedLevel = (speedLevel + 1) % 4;
    switch (speedLevel) {
        case 0:
            animationCtrl.setSpeed(1.0f);
            speedBtn.setLabel("Speed: 1x");
            break;
        case 1:
            animationCtrl.setSpeed(0.5f);
            speedBtn.setLabel("Speed: 2x");
            break;
        case 2:
            animationCtrl.setSpeed(0.2f);
            speedBtn.setLabel("Speed: 4x");
            break;
        case 3:
            animationCtrl.setSpeed(1.8f);
            speedBtn.setLabel("Speed: 0.5x");
            break;
    }
}

bool MainWindow::init() {
    if (!loadFonts()) {
        std::cerr << "Warning: Could not load any true type fonts from disk." << std::endl;
    }

    // High performance settings: 4x MSAA for sharp lines with optimal fill rate
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;

    window.create(sf::VideoMode(windowWidth, windowHeight),
                  "AutomataViz — Interactive Regular Expression to DFA Visualizer",
                  sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize,
                  settings);
    
    // Smooth refresh rate without Windows timer resolution jitter
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(120);

    setupUI();
    executePipeline(currentRegexStr);

    return true;
}

void MainWindow::handleResize(unsigned int newWidth, unsigned int newHeight) {
    windowWidth = std::max(1024u, newWidth);
    windowHeight = std::max(700u, newHeight);
    window.setSize({windowWidth, windowHeight});
    sf::View view(sf::FloatRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
    window.setView(view);
    setupLayout();
}

void MainWindow::handleExampleSelect(int index) {
    const std::vector<std::string> presets = {
        "(a|b)*abb",
        "a*",
        "(a|b)*",
        "(a|b)(a|b)",
        "a(b|c)*d"
    };

    if (index >= 0 && index < static_cast<int>(presets.size())) {
        currentRegexStr = presets[index];
        regexInput.setText(currentRegexStr);
        executePipeline(currentRegexStr);
    }
}

void MainWindow::loadNFAIntoRenderer(GraphRenderer& targetRenderer, const NFA& nfa,
                                     const std::vector<int>& highlightedStates,
                                     const std::vector<Transition>& highlightedTransitions,
                                     bool recomputeLayout,
                                     const sf::FloatRect& bounds) {
    std::vector<Node> nodes;
    for (const auto& st : nfa.getStates()) {
        Node node(st.id, st.label, {0.f, 0.f}, st.isStart, st.isFinal);
        for (int hId : highlightedStates) {
            if (hId == st.id) {
                node.isHighlighted = true;
                break;
            }
        }
        nodes.push_back(node);
    }

    std::map<std::pair<int, int>, Edge> edgeMap;
    for (const auto& trans : nfa.getTransitions()) {
        auto key = std::make_pair(trans.from, trans.to);
        if (edgeMap.find(key) == edgeMap.end()) {
            edgeMap[key] = Edge(trans.from, trans.to);
        }
        edgeMap[key].addSymbol(trans.getDisplaySymbol());

        for (const auto& ht : highlightedTransitions) {
            if (ht.from == trans.from && ht.to == trans.to && ht.symbol == trans.symbol) {
                edgeMap[key].isHighlighted = true;
            }
        }
    }

    for (auto& kv : edgeMap) {
        auto revKey = std::make_pair(kv.first.second, kv.first.first);
        if (kv.first.first != kv.first.second && edgeMap.find(revKey) != edgeMap.end()) {
            kv.second.isBidirectional = true;
        }
    }

    std::vector<Edge> edges;
    for (const auto& kv : edgeMap) {
        edges.push_back(kv.second);
    }

    if (recomputeLayout || targetRenderer.getNodes().empty()) {
        GraphLayout::computeLayout(nodes, edges, nfa.getStartState(), bounds.width, bounds.height);
        targetRenderer.setNodesAndEdges(nodes, edges, false);
    } else {
        targetRenderer.setNodesAndEdges(nodes, edges, true);
    }
}

void MainWindow::loadDFAIntoRenderer(const DFA& dfa, int highlightedState, int currentSimState,
                                     const std::pair<int, char>& currentSimTrans,
                                     bool recomputeLayout,
                                     const sf::FloatRect& bounds) {
    std::vector<Node> nodes;
    for (const auto& st : dfa.getStates()) {
        Node node(st.id, st.label, {0.f, 0.f}, st.isStart, st.isFinal);
        if (!st.nfaSubsetState.empty()) {
            std::string sub = "{";
            bool first = true;
            for (int sid : st.nfaSubsetState) {
                if (!first) sub += ",";
                sub += "q" + std::to_string(sid);
                first = false;
            }
            sub += "}";
            node.subLabel = sub;
        }

        if (st.id == highlightedState) {
            node.isHighlighted = true;
        }
        if (st.id == currentSimState) {
            node.isCurrentSimState = true;
        }
        nodes.push_back(node);
    }

    std::map<std::pair<int, int>, Edge> edgeMap;
    for (const auto& trans : dfa.getAllTransitions()) {
        auto key = std::make_pair(trans.from, trans.to);
        if (edgeMap.find(key) == edgeMap.end()) {
            edgeMap[key] = Edge(trans.from, trans.to);
        }
        edgeMap[key].addSymbol(trans.getDisplaySymbol());

        if (currentSimTrans.first == trans.from && currentSimTrans.second == trans.symbol) {
            edgeMap[key].isCurrentSimTransition = true;
        }
    }

    for (auto& kv : edgeMap) {
        auto revKey = std::make_pair(kv.first.second, kv.first.first);
        if (kv.first.first != kv.first.second && edgeMap.find(revKey) != edgeMap.end()) {
            kv.second.isBidirectional = true;
        }
    }

    std::vector<Edge> edges;
    for (const auto& kv : edgeMap) {
        edges.push_back(kv.second);
    }

    if (recomputeLayout || graphRenderer.getNodes().empty()) {
        GraphLayout::computeLayout(nodes, edges, dfa.getStartState(), bounds.width, bounds.height);
        graphRenderer.setNodesAndEdges(nodes, edges, false);
    } else {
        graphRenderer.setNodesAndEdges(nodes, edges, true);
    }
}

void MainWindow::executePipeline(const std::string& regexStr) {
    currentRegexStr = regexStr;
    parseResult = RegexParser::parse(currentRegexStr);

    if (!parseResult.success) {
        InfoContent errContent;
        errContent.stageTitle = "Syntax Validation Error";
        errContent.stepIndicator = "Error";
        errContent.operationTitle = "Invalid Regular Expression";
        errContent.explanation = parseResult.errorMessage;
        infoPanel.setContent(errContent);
        return;
    }

    thompsonResult = ThompsonConstruction::build(parseResult.postfixTokens);
    if (!thompsonResult.success) {
        InfoContent errContent;
        errContent.stageTitle = "Thompson Construction Error";
        errContent.explanation = thompsonResult.errorMessage;
        infoPanel.setContent(errContent);
        return;
    }

    subsetResult = SubsetConstruction::convert(thompsonResult.finalNFA);
    minimizationResult = DFAMinimization::minimize(subsetResult.finalDFA);

    stringSimulator.setDFA(minimizationResult.minimizedDFA);
    stringSimulator.setInputString(simStringInput.getText());

    currentThompsonStep = static_cast<int>(thompsonResult.steps.size()) - 1;
    currentSubsetStep = static_cast<int>(subsetResult.steps.size()) - 1;
    currentMinimizationStep = static_cast<int>(minimizationResult.steps.size()) - 1;

    tableView.setDFA(subsetResult.finalDFA);

    // Reset layout caches for clean initial layout
    nfaFullLayoutComputed = false;
    dfaFullLayoutComputed = false;
    minDfaLayoutComputed = false;

    updateGraphForCurrentState();
    updateInfoPanel();
    graphRenderer.resetView(canvasBounds);
    nfaSideRenderer.resetView(leftCanvasBounds);
}

void MainWindow::updateGraphForCurrentState() {
    switch (currentMode) {
        case ViewMode::THOMPSON_NFA: {
            if (!thompsonResult.steps.empty() && currentThompsonStep >= 0 &&
                currentThompsonStep < static_cast<int>(thompsonResult.steps.size())) {
                const auto& step = thompsonResult.steps[currentThompsonStep];
                loadNFAIntoRenderer(graphRenderer, step.nfaSnapshot, step.newStates, step.newTransitions,
                                    !nfaFullLayoutComputed, canvasBounds);
                nfaFullLayoutComputed = true;
            } else {
                loadNFAIntoRenderer(graphRenderer, thompsonResult.finalNFA, {}, {}, false, canvasBounds);
            }
            break;
        }
        case ViewMode::SUBSET_DFA: {
            if (!subsetResult.steps.empty() && currentSubsetStep >= 0 &&
                currentSubsetStep < static_cast<int>(subsetResult.steps.size())) {
                const auto& step = subsetResult.steps[currentSubsetStep];
                if (isDualView) {
                    std::vector<int> nfaHighlights(step.sourceNFASubset.begin(), step.sourceNFASubset.end());
                    loadNFAIntoRenderer(nfaSideRenderer, thompsonResult.finalNFA, nfaHighlights, {}, false, leftCanvasBounds);
                    loadDFAIntoRenderer(step.dfaSnapshot, step.targetDFAStateId, -1, {-1, '\0'},
                                        !dfaFullLayoutComputed, rightCanvasBounds);
                    dfaFullLayoutComputed = true;
                } else {
                    loadDFAIntoRenderer(step.dfaSnapshot, step.targetDFAStateId, -1, {-1, '\0'},
                                        !dfaFullLayoutComputed, canvasBounds);
                    dfaFullLayoutComputed = true;
                }
            } else {
                if (isDualView) {
                    loadNFAIntoRenderer(nfaSideRenderer, thompsonResult.finalNFA, {}, {}, false, leftCanvasBounds);
                    loadDFAIntoRenderer(subsetResult.finalDFA, -1, -1, {-1, '\0'}, false, rightCanvasBounds);
                } else {
                    loadDFAIntoRenderer(subsetResult.finalDFA, -1, -1, {-1, '\0'}, false, canvasBounds);
                }
            }
            break;
        }
        case ViewMode::DFA_VIEW: {
            loadDFAIntoRenderer(subsetResult.finalDFA, -1, -1, {-1, '\0'}, !dfaFullLayoutComputed, canvasBounds);
            dfaFullLayoutComputed = true;
            break;
        }
        case ViewMode::DFA_MINIMIZATION: {
            loadDFAIntoRenderer(minimizationResult.minimizedDFA, -1, -1, {-1, '\0'}, !minDfaLayoutComputed, canvasBounds);
            minDfaLayoutComputed = true;
            break;
        }
        case ViewMode::STRING_SIMULATION: {
            int curSimState = stringSimulator.getCurrentStateId();
            std::pair<int, char> curSimTrans = {-1, '\0'};
            const auto* curStep = stringSimulator.getCurrentStep();
            if (curStep) {
                curSimTrans = {curStep->fromState, curStep->symbolRead};
            }
            if (!minDfaLayoutComputed) {
                loadDFAIntoRenderer(minimizationResult.minimizedDFA, -1, curSimState, curSimTrans, true, canvasBounds);
                minDfaLayoutComputed = true;
            } else {
                // Instantaneous O(1) highlight update with zero layout recomputation lag
                graphRenderer.updateHighlights(-1, curSimState, curSimTrans);
            }
            break;
        }
        default:
            break;
    }
}

void MainWindow::updateInfoPanel() {
    InfoContent info;

    switch (currentMode) {
        case ViewMode::REGEX_POSTFIX: {
            info.stageTitle = "Stage 1: Regex & Postfix Parser";
            info.stepIndicator = "Infix → Postfix";
            info.operationTitle = "Shunting-Yard Conversion";
            info.inputMathText = "Infix: " + currentRegexStr;
            info.resultMathText = "Postfix: " + parseResult.postfixString;
            info.explanation = "The regular expression was tokenized with implicit concatenations explicitly inserted (.). The Shunting-Yard algorithm produced Reverse Polish Notation (RPN) according to operator precedence (*, +, ? > . > |).";
            info.extraDetails.push_back("Alphabet Σ = {" + [&]() {
                std::string a;
                for (char c : parseResult.alphabet) {
                    if (!a.empty()) a += ", ";
                    a += c;
                }
                return a;
            }() + "}");
            info.extraDetails.push_back("Total Postfix Tokens: " + std::to_string(parseResult.postfixTokens.size()));
            break;
        }
        case ViewMode::THOMPSON_NFA: {
            info.stageTitle = "Stage 2: Thompson's Construction (ε-NFA)";
            if (!thompsonResult.steps.empty() && currentThompsonStep >= 0 &&
                currentThompsonStep < static_cast<int>(thompsonResult.steps.size())) {
                const auto& step = thompsonResult.steps[currentThompsonStep];
                info.stepIndicator = "Step " + std::to_string(currentThompsonStep + 1) + " of " +
                                     std::to_string(thompsonResult.steps.size());
                info.operationTitle = step.operation;
                info.inputMathText = "Token: " + step.tokenStr;
                info.resultMathText = "States added: " + std::to_string(step.newStates.size()) +
                                      ", Trans added: " + std::to_string(step.newTransitions.size());
                info.explanation = step.description;
            }
            info.extraDetails.push_back("Current NFA States: " + std::to_string(thompsonResult.finalNFA.stateCount()));
            info.extraDetails.push_back("Current NFA Transitions: " + std::to_string(thompsonResult.finalNFA.transitionCount()));
            break;
        }
        case ViewMode::SUBSET_DFA: {
            info.stageTitle = "Stage 3: Subset Construction (ε-NFA → DFA)";
            if (!subsetResult.steps.empty() && currentSubsetStep >= 0 &&
                currentSubsetStep < static_cast<int>(subsetResult.steps.size())) {
                const auto& step = subsetResult.steps[currentSubsetStep];
                info.stepIndicator = "Step " + std::to_string(currentSubsetStep + 1) + " of " +
                                     std::to_string(subsetResult.steps.size());
                info.operationTitle = step.actionName;

                if (step.sourceDFAStateId != -1) {
                    info.inputMathText = "DFA State " + step.sourceDFAStateLabel + " on '" + std::string(1, step.symbol) + "'";
                } else {
                    info.inputMathText = "Start state ε-closure";
                }

                if (!step.targetDFAStateLabel.empty()) {
                    info.resultMathText = "Target State: " + step.targetDFAStateLabel;
                }

                info.explanation = step.explanation;
            }
            info.extraDetails.push_back("Total DFA States: " + std::to_string(subsetResult.finalDFA.stateCount()));
            info.extraDetails.push_back("Total DFA Transitions: " + std::to_string(subsetResult.finalDFA.transitionCount()));
            break;
        }
        case ViewMode::DFA_VIEW: {
            info.stageTitle = "Stage 4: DFA Representation";
            info.stepIndicator = dfaDisplayType == DFADisplayType::GRAPH_VIEW ? "Graph Mode" : "Table Mode";
            info.operationTitle = "Deterministic Finite Automaton";
            info.inputMathText = "States: " + std::to_string(subsetResult.finalDFA.stateCount());
            info.resultMathText = "Alphabet: " + std::to_string(subsetResult.finalDFA.getAlphabet().size()) + " symbols";
            info.explanation = "A fully deterministic automaton: for every state and input symbol, there exists at most one deterministic next state, and no epsilon transitions exist.";
            info.extraDetails.push_back("Initial State: " + [&]() {
                const State* s = subsetResult.finalDFA.getState(subsetResult.finalDFA.getStartState());
                return s ? s->label : "-";
            }());
            break;
        }
        case ViewMode::DFA_MINIMIZATION: {
            info.stageTitle = "Stage 5: DFA Minimization";
            if (!minimizationResult.steps.empty() && currentMinimizationStep >= 0 &&
                currentMinimizationStep < static_cast<int>(minimizationResult.steps.size())) {
                const auto& step = minimizationResult.steps[currentMinimizationStep];
                info.stepIndicator = "Round " + std::to_string(step.roundNumber) + " of " +
                                     std::to_string(minimizationResult.steps.size() - 1);
                info.operationTitle = step.phaseName;
                info.inputMathText = step.splitReason;
                info.explanation = step.explanation;
            }

            std::ostringstream ss;
            ss << "States: " << minimizationResult.stats.originalStateCount << " → "
               << minimizationResult.stats.minimizedStateCount << "\nTransitions: "
               << minimizationResult.stats.originalTransitionCount << " → "
               << minimizationResult.stats.minimizedTransitionCount;
            info.statsText = ss.str();

            for (const auto& mg : minimizationResult.stats.mergedGroups) {
                std::string details = "State " + mg.first + " merged: {";
                for (size_t i = 0; i < mg.second.size(); ++i) {
                    if (i > 0) details += ", ";
                    details += mg.second[i];
                }
                details += "}";
                info.extraDetails.push_back(details);
            }
            break;
        }
        case ViewMode::STRING_SIMULATION: {
            info.stageTitle = "Stage 6: String Acceptance Simulation";
            info.stepIndicator = "Simulation";
            info.operationTitle = "DFA Execution Trace";
            info.inputMathText = "String: \"" + stringSimulator.getInputString() + "\"";

            const auto* step = stringSimulator.getCurrentStep();
            if (step) {
                const State* curSt = minimizationResult.minimizedDFA.getState(step->fromState);
                const State* nxtSt = minimizationResult.minimizedDFA.getState(step->toState);
                std::string cL = curSt ? curSt->label : std::to_string(step->fromState);
                std::string nL = nxtSt ? nxtSt->label : std::to_string(step->toState);
                info.resultMathText = "Read '" + std::string(1, step->symbolRead) + "': " + cL + " → " + nL;
                info.explanation = step->explanation;
            } else {
                info.resultMathText = "Status: At Start State";
                info.explanation = "Enter a test string below and click 'Step Sim ▶' to watch character-by-character state transitions.";
            }

            info.extraDetails.push_back("Result: " + stringSimulator.getFullResult().statusMessage);
            break;
        }
        case ViewMode::THEORY_GUIDE: {
            info.stageTitle = "Unit 1: Theory of Computation";
            info.stepIndicator = "Syllabus Guide";
            info.operationTitle = "Formal Language Foundations";
            info.explanation = "Detailed course guide reviewing the theoretical foundations of regular expressions, finite automata equivalences, subset construction, and minimization.";
            break;
        }
    }

    infoPanel.setContent(info);
}

void MainWindow::goToPreviousStep() {
    switch (currentMode) {
        case ViewMode::THOMPSON_NFA:
            if (currentThompsonStep > 0) {
                currentThompsonStep--;
                updateGraphForCurrentState();
                updateInfoPanel();
            }
            break;
        case ViewMode::SUBSET_DFA:
            if (currentSubsetStep > 0) {
                currentSubsetStep--;
                updateGraphForCurrentState();
                updateInfoPanel();
            }
            break;
        case ViewMode::DFA_MINIMIZATION:
            if (currentMinimizationStep > 0) {
                currentMinimizationStep--;
                updateGraphForCurrentState();
                updateInfoPanel();
            }
            break;
        case ViewMode::STRING_SIMULATION:
            stringSimulator.stepBackward();
            updateGraphForCurrentState();
            updateInfoPanel();
            break;
        default:
            break;
    }
}

void MainWindow::goToNextStep() {
    switch (currentMode) {
        case ViewMode::THOMPSON_NFA:
            if (currentThompsonStep < static_cast<int>(thompsonResult.steps.size()) - 1) {
                currentThompsonStep++;
                updateGraphForCurrentState();
                updateInfoPanel();
            } else {
                animationCtrl.pause();
                playPauseBtn.setLabel("▶ Auto Play");
            }
            break;
        case ViewMode::SUBSET_DFA:
            if (currentSubsetStep < static_cast<int>(subsetResult.steps.size()) - 1) {
                currentSubsetStep++;
                updateGraphForCurrentState();
                updateInfoPanel();
            } else {
                animationCtrl.pause();
                playPauseBtn.setLabel("▶ Auto Play");
            }
            break;
        case ViewMode::DFA_MINIMIZATION:
            if (currentMinimizationStep < static_cast<int>(minimizationResult.steps.size()) - 1) {
                currentMinimizationStep++;
                updateGraphForCurrentState();
                updateInfoPanel();
            } else {
                animationCtrl.pause();
                playPauseBtn.setLabel("▶ Auto Play");
            }
            break;
        case ViewMode::STRING_SIMULATION:
            stringSimulator.stepForward();
            updateGraphForCurrentState();
            updateInfoPanel();
            break;
        default:
            break;
    }
}

void MainWindow::resetCurrentStep() {
    animationCtrl.pause();
    playPauseBtn.setLabel("▶ Auto Play");

    switch (currentMode) {
        case ViewMode::THOMPSON_NFA:
            currentThompsonStep = 0;
            break;
        case ViewMode::SUBSET_DFA:
            currentSubsetStep = 0;
            break;
        case ViewMode::DFA_MINIMIZATION:
            currentMinimizationStep = 0;
            break;
        case ViewMode::STRING_SIMULATION:
            stringSimulator.reset();
            break;
        default:
            break;
    }
    updateGraphForCurrentState();
    updateInfoPanel();
}

void MainWindow::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        } else if (event.type == sf::Event::Resized) {
            handleResize(event.size.width, event.size.height);
        }

        regexInput.handleEvent(event, window);
        generateBtn.handleEvent(event, window);

        for (auto& eb : exampleButtons) {
            eb.handleEvent(event, window);
        }

        modeTabBar.handleEvent(event, window);
        prevStepBtn.handleEvent(event, window);
        nextStepBtn.handleEvent(event, window);
        playPauseBtn.handleEvent(event, window);
        resetStepBtn.handleEvent(event, window);
        speedBtn.handleEvent(event, window);
        zoomInBtn.handleEvent(event, window);
        zoomOutBtn.handleEvent(event, window);
        resetViewBtn.handleEvent(event, window);

        if (currentMode == ViewMode::DFA_VIEW) {
            toggleTableBtn.handleEvent(event, window);
        } else if (currentMode == ViewMode::SUBSET_DFA) {
            toggleDualViewBtn.handleEvent(event, window);
        }

        infoPanel.handleEvent(event, window);

        if (currentMode == ViewMode::STRING_SIMULATION) {
            simStringInput.handleEvent(event, window);
            simStepBtn.handleEvent(event, window);
            simRunBtn.handleEvent(event, window);
            simResetBtn.handleEvent(event, window);
        }

        if (currentMode == ViewMode::DFA_VIEW && dfaDisplayType == DFADisplayType::TABLE_VIEW) {
            tableView.handleEvent(event, window);
        } else if (currentMode == ViewMode::THEORY_GUIDE) {
            if (event.type == sf::Event::MouseWheelScrolled) {
                theoryScrollY -= event.mouseWheelScroll.delta * 28.0f;
                if (theoryScrollY < 0.0f) theoryScrollY = 0.0f;
                if (theoryScrollY > maxTheoryScrollY) theoryScrollY = maxTheoryScrollY;
            }
        } else if (currentMode == ViewMode::SUBSET_DFA && isDualView) {
            sf::Vector2i pixelPos(0, 0);
            if (event.type == sf::Event::MouseMoved) {
                pixelPos = {event.mouseMove.x, event.mouseMove.y};
            } else if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseButtonReleased) {
                pixelPos = {event.mouseButton.x, event.mouseButton.y};
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                pixelPos = {event.mouseWheelScroll.x, event.mouseWheelScroll.y};
            }

            sf::Vector2f mPos = window.mapPixelToCoords(pixelPos);
            if (leftCanvasBounds.contains(mPos)) {
                nfaSideRenderer.handleEvent(event, window, leftCanvasBounds);
            } else if (rightCanvasBounds.contains(mPos)) {
                graphRenderer.handleEvent(event, window, rightCanvasBounds);
            }
        } else {
            graphRenderer.handleEvent(event, window, canvasBounds);
        }
    }
}

void MainWindow::renderTopBar() {
    sf::RectangleShape bg({topBarBounds.width, topBarBounds.height});
    bg.setPosition(topBarBounds.left, topBarBounds.top);
    bg.setFillColor(UITheme::BgPanel);
    window.draw(bg);

    sf::Text title;
    title.setFont(font);
    title.setString("AutomataViz");
    title.setCharacterSize(22);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(UITheme::AccentCyan);
    title.setPosition(18.0f, 14.0f);
    window.draw(title);

    sf::Text subtitle;
    subtitle.setFont(font);
    subtitle.setString("TOC Educational Visualizer");
    subtitle.setCharacterSize(11);
    subtitle.setFillColor(UITheme::TextMuted);
    subtitle.setPosition(18.0f, 44.0f);
    window.draw(subtitle);

    regexInput.render(window, font);
    generateBtn.render(window, font);

    for (auto& eb : exampleButtons) {
        eb.render(window, font);
    }
}

void MainWindow::renderControlsBar() {
    sf::RectangleShape bg({controlBarBounds.width, controlBarBounds.height});
    bg.setPosition(controlBarBounds.left, controlBarBounds.top);
    bg.setFillColor(UITheme::BgPanelSecondary);
    bg.setOutlineThickness(1.0f);
    bg.setOutlineColor(UITheme::BorderDark);
    window.draw(bg);

    prevStepBtn.render(window, font);
    nextStepBtn.render(window, font);
    playPauseBtn.render(window, font);
    resetStepBtn.render(window, font);
    speedBtn.render(window, font);

    zoomInBtn.render(window, font);
    zoomOutBtn.render(window, font);
    resetViewBtn.render(window, font);

    if (currentMode == ViewMode::DFA_VIEW) {
        toggleTableBtn.render(window, font);
    } else if (currentMode == ViewMode::SUBSET_DFA) {
        toggleDualViewBtn.render(window, font);
    }
}

void MainWindow::renderSimulationBar() {
    sf::RectangleShape bg({simBarBounds.width, simBarBounds.height});
    bg.setPosition(simBarBounds.left, simBarBounds.top);
    bg.setFillColor(UITheme::BgPanel);
    bg.setOutlineThickness(1.0f);
    bg.setOutlineColor(UITheme::BorderDark);
    window.draw(bg);

    simStringInput.render(window, font);
    simStepBtn.render(window, font);
    simRunBtn.render(window, font);
    simResetBtn.render(window, font);

    float tapeStartX = 550.0f;
    float tapeY = simBarBounds.top + 16.0f;
    const std::string& str = stringSimulator.getInputString();
    size_t curIdx = stringSimulator.getCurrentCharIndex();

    for (size_t i = 0; i < str.size(); ++i) {
        bool isCurrentChar = (i == curIdx && stringSimulator.getStatus() != SimulationState::NOT_STARTED);

        sf::RectangleShape cell({34.0f, 38.0f});
        cell.setPosition(tapeStartX + i * 38.0f, tapeY);
        cell.setFillColor(isCurrentChar ? UITheme::AccentAmber : UITheme::BgInput);
        cell.setOutlineThickness(1.5f);
        cell.setOutlineColor(isCurrentChar ? UITheme::TextWhite : UITheme::BorderDark);
        window.draw(cell);

        sf::Text chText;
        chText.setFont(font);
        chText.setString(std::string(1, str[i]));
        chText.setCharacterSize(16);
        chText.setFillColor(isCurrentChar ? UITheme::BgDark : UITheme::TextWhite);
        sf::FloatRect cb = chText.getLocalBounds();
        chText.setOrigin(cb.left + cb.width / 2.0f, cb.top + cb.height / 2.0f);
        chText.setPosition(tapeStartX + i * 38.0f + 17.0f, tapeY + 19.0f);
        window.draw(chText);
    }

    SimulationState simStatus = stringSimulator.getStatus();
    if (simStatus == SimulationState::ACCEPTED || simStatus == SimulationState::REJECTED) {
        float badgeX = tapeStartX + (str.size() * 38.0f) + 18.0f;
        sf::RectangleShape badge({120.0f, 38.0f});
        badge.setPosition(badgeX, tapeY);
        badge.setFillColor(simStatus == SimulationState::ACCEPTED ? UITheme::AccentGreen : UITheme::AccentRed);
        window.draw(badge);

        sf::Text statusText;
        statusText.setFont(font);
        statusText.setString(simStatus == SimulationState::ACCEPTED ? "ACCEPTED ✓" : "REJECTED ✗");
        statusText.setCharacterSize(14);
        statusText.setStyle(sf::Text::Bold);
        statusText.setFillColor(UITheme::TextWhite);
        sf::FloatRect sb = statusText.getLocalBounds();
        statusText.setOrigin(sb.left + sb.width / 2.0f, sb.top + sb.height / 2.0f);
        statusText.setPosition(badgeX + 60.0f, tapeY + 19.0f);
        window.draw(statusText);
    }
}

void MainWindow::renderRegexPostfixView() {
    sf::RectangleShape bg({canvasBounds.width, canvasBounds.height});
    bg.setPosition(canvasBounds.left, canvasBounds.top);
    bg.setFillColor(UITheme::BgDark);
    window.draw(bg);

    float startY = canvasBounds.top + 28.0f;
    float startX = canvasBounds.left + 32.0f;

    sf::Text h1;
    h1.setFont(font);
    h1.setString("Stage 1 — Regular Expression Tokenization & Postfix Conversion");
    h1.setCharacterSize(18);
    h1.setStyle(sf::Text::Bold);
    h1.setFillColor(UITheme::AccentCyan);
    h1.setPosition(startX, startY);
    window.draw(h1);

    startY += 36.0f;

    auto drawCard = [&](const std::string& title, const std::string& content, float x, float y, float w, float h) {
        sf::RectangleShape card({w, h});
        card.setPosition(x, y);
        card.setFillColor(UITheme::BgPanel);
        card.setOutlineThickness(1.0f);
        card.setOutlineColor(UITheme::BorderDark);
        window.draw(card);

        sf::Text tTitle;
        tTitle.setFont(font);
        tTitle.setString(title);
        tTitle.setCharacterSize(12);
        tTitle.setFillColor(UITheme::TextDim);
        tTitle.setPosition(x + 14.0f, y + 10.0f);
        window.draw(tTitle);

        sf::Text tContent;
        tContent.setFont(fontMono.getInfo().family.empty() ? font : fontMono);
        tContent.setString(content);
        tContent.setCharacterSize(16);
        tContent.setStyle(sf::Text::Bold);
        tContent.setFillColor(UITheme::TextWhite);
        tContent.setPosition(x + 14.0f, y + 28.0f);
        window.draw(tContent);
    };

    drawCard("RAW INPUT REGEX", currentRegexStr, startX, startY, 260.0f, 64.0f);
    drawCard("EXPLICIT CONCATENATION", [&]() {
        std::string s;
        for (const auto& t : parseResult.explicitTokens) s += t.toString();
        return s;
    }(), startX + 276.0f, startY, 290.0f, 64.0f);
    drawCard("POSTFIX NOTATION (RPN)", parseResult.postfixString, startX + 582.0f, startY, 260.0f, 64.0f);

    startY += 84.0f;

    sf::Text h2;
    h2.setFont(font);
    h2.setString("Operator Precedence Hierarchy in Regular Expressions:");
    h2.setCharacterSize(15);
    h2.setStyle(sf::Text::Bold);
    h2.setFillColor(UITheme::TextWhite);
    h2.setPosition(startX, startY);
    window.draw(h2);

    startY += 26.0f;

    const std::vector<std::tuple<std::string, std::string, std::string, std::string>> precData = {
        {"1 (Highest)", "*, +, ?", "Kleene Star, Plus, Optional", "Unary Postfix (repetition)"},
        {"2 (Middle)", ".", "Concatenation", "Binary Infix (sequence)"},
        {"3 (Lowest)", "|", "Alternation / Union", "Binary Infix (choice)"}
    };

    for (const auto& item : precData) {
        sf::RectangleShape row({840.0f, 32.0f});
        row.setPosition(startX, startY);
        row.setFillColor(UITheme::BgPanelSecondary);
        row.setOutlineThickness(0.5f);
        row.setOutlineColor(UITheme::BorderDark);
        window.draw(row);

        sf::Text t1; t1.setFont(font); t1.setString(std::get<0>(item)); t1.setCharacterSize(13); t1.setFillColor(UITheme::AccentAmber); t1.setPosition(startX + 14.0f, startY + 7.0f); window.draw(t1);
        sf::Text t2; t2.setFont(font); t2.setString(std::get<1>(item)); t2.setCharacterSize(13); t2.setFillColor(UITheme::AccentCyan); t2.setPosition(startX + 160.0f, startY + 7.0f); window.draw(t2);
        sf::Text t3; t3.setFont(font); t3.setString(std::get<2>(item)); t3.setCharacterSize(13); t3.setFillColor(UITheme::TextWhite); t3.setPosition(startX + 270.0f, startY + 7.0f); window.draw(t3);
        sf::Text t4; t4.setFont(font); t4.setString(std::get<3>(item)); t4.setCharacterSize(13); t4.setFillColor(UITheme::TextMuted); t4.setPosition(startX + 550.0f, startY + 7.0f); window.draw(t4);

        startY += 34.0f;
    }

    startY += 18.0f;

    sf::Text h3;
    h3.setFont(font);
    h3.setString("Shunting-Yard Step-by-Step Conversion Log:");
    h3.setCharacterSize(15);
    h3.setStyle(sf::Text::Bold);
    h3.setFillColor(UITheme::TextWhite);
    h3.setPosition(startX, startY);
    window.draw(h3);

    startY += 26.0f;

    size_t displayCount = std::min(parseResult.stepLogs.size(), size_t(8));
    for (size_t i = 0; i < displayCount; ++i) {
        const auto& log = parseResult.stepLogs[i];

        sf::RectangleShape row({840.0f, 28.0f});
        row.setPosition(startX, startY);
        row.setFillColor((i % 2 == 0) ? UITheme::BgPanel : UITheme::BgPanelSecondary);
        row.setOutlineThickness(0.5f);
        row.setOutlineColor(UITheme::BorderDark);
        window.draw(row);

        sf::Text stepNum; stepNum.setFont(font); stepNum.setString("Step " + std::to_string(log.stepNumber)); stepNum.setCharacterSize(12); stepNum.setFillColor(UITheme::TextDim); stepNum.setPosition(startX + 12.0f, startY + 6.0f); window.draw(stepNum);
        sf::Text tok; tok.setFont(font); tok.setString(log.tokenDescription); tok.setCharacterSize(12); tok.setFillColor(UITheme::AccentCyan); tok.setPosition(startX + 80.0f, startY + 6.0f); window.draw(tok);
        sf::Text st; st.setFont(font); st.setString("Stack: " + log.operatorStackSnapshot); st.setCharacterSize(12); st.setFillColor(UITheme::AccentAmber); st.setPosition(startX + 230.0f, startY + 6.0f); window.draw(st);
        sf::Text pf; pf.setFont(font); pf.setString("Output: " + log.postfixOutputSnapshot); pf.setCharacterSize(12); pf.setFillColor(UITheme::AccentGreen); pf.setPosition(startX + 370.0f, startY + 6.0f); window.draw(pf);

        startY += 30.0f;
    }
}

void MainWindow::renderTheoryGuide() {
    sf::View origView = window.getView();
    sf::View guideView(canvasBounds);
    guideView.setViewport(sf::FloatRect(
        canvasBounds.left / windowWidth,
        canvasBounds.top / windowHeight,
        canvasBounds.width / windowWidth,
        canvasBounds.height / windowHeight
    ));
    window.setView(guideView);

    sf::RectangleShape bg({canvasBounds.width, canvasBounds.height});
    bg.setPosition(canvasBounds.left, canvasBounds.top);
    bg.setFillColor(UITheme::BgDark);
    window.draw(bg);

    float startX = canvasBounds.left + 36.0f;
    float curY = canvasBounds.top + 28.0f - theoryScrollY;
    float contentW = canvasBounds.width - 72.0f;

    auto printHeader = [&](const std::string& title) {
        sf::Text t;
        t.setFont(font);
        t.setString(title);
        t.setCharacterSize(19);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(UITheme::AccentCyan);
        t.setPosition(startX, curY);
        window.draw(t);
        curY += 28.0f;
    };

    auto printSubheader = [&](const std::string& subtitle) {
        sf::Text t;
        t.setFont(font);
        t.setString(subtitle);
        t.setCharacterSize(15);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(UITheme::AccentAmber);
        t.setPosition(startX, curY);
        window.draw(t);
        curY += 22.0f;
    };

    auto printParagraph = [&](const std::string& text) {
        infoPanel.drawWrappedText(window, text, startX, curY, contentW, 13, UITheme::TextWhite, font);
        curY += 12.0f;
    };

    printHeader("Theory of Computation — Unit 1 Comprehensive Academic Guide");
    printParagraph("This interactive educational environment visualizes the foundational theorems of formal language theory, illustrating how regular expressions are compiled into equivalent finite automata.");

    printSubheader("1. Regular Expressions (RE)");
    printParagraph("A regular expression is an algebraic specification of a regular language. Given an alphabet Σ, the basic regular expressions are ∅ (empty language), ε (empty string), and 'a' for every a ∈ Σ. Operators include Union (a|b), Concatenation (ab), and Kleene Star (a*).");

    printSubheader("2. Thompson's Construction (RE → ε-NFA)");
    printParagraph("Thompson's Construction algorithm recursively translates regular expression fragments into equivalent non-deterministic finite automata with ε-transitions (ε-NFA). Each basic symbol creates a 2-state automaton with one forward transition. Operators (|, ., *) link fragments with ε-transitions without cycles through external nodes, ensuring exactly one start state and one accepting state per fragment.");

    printSubheader("3. Equivalence of NFA and DFA");
    printParagraph("Every non-deterministic finite automaton with or without ε-transitions can be transformed into an equivalent deterministic finite automaton (DFA) that accepts the exact same language (L(NFA) = L(DFA)). Although the worst-case number of states can be exponential (2^|Q|), typical practical expressions yield compact DFAs.");

    printSubheader("4. Subset Construction Algorithm (Powerset Construction)");
    printParagraph("The subset construction algorithm computes DFA states as subsets of NFA states. Starting with the ε-closure of the NFA start state, for every discovered DFA state and each alphabet symbol 'a', it calculates move(Q, a) followed by ε-closure(move(Q, a)). A DFA state is marked accepting if its subset contains any NFA final state.");

    printSubheader("5. DFA Minimization (Partition Refinement)");
    printParagraph("A minimal DFA for any regular language is unique up to state isomorphism (Myhill-Nerode theorem). The partition refinement algorithm begins with P0 = {Non-Final, Final}. In successive rounds, partition groups are split if their states transition to different partition groups for any input symbol. When no group splits, equivalent states are merged into single minimal states.");

    printSubheader("6. Applications of Finite Automata");
    printParagraph("Finite automata are widely used across computer science, including lexical analysis in compilers (lex/flex), pattern matching engines (grep), network intrusion detection systems, protocol verification, and hardware digital circuit design.");

    float totalHeight = (curY + theoryScrollY) - canvasBounds.top;
    maxTheoryScrollY = std::max(0.0f, totalHeight - canvasBounds.height + 40.0f);

    window.setView(origView);
}

void MainWindow::render() {
    window.clear(UITheme::BgDark);

    if (currentMode == ViewMode::REGEX_POSTFIX) {
        renderRegexPostfixView();
    } else if (currentMode == ViewMode::THEORY_GUIDE) {
        renderTheoryGuide();
    } else if (currentMode == ViewMode::DFA_VIEW && dfaDisplayType == DFADisplayType::TABLE_VIEW) {
        tableView.render(window, font);
    } else if (currentMode == ViewMode::SUBSET_DFA && isDualView) {
        // Dual split view rendering
        nfaSideRenderer.render(window, font, leftCanvasBounds);

        // Divider
        sf::RectangleShape divider({2.0f, canvasBounds.height});
        divider.setPosition(leftCanvasBounds.left + leftCanvasBounds.width + 3.0f, canvasBounds.top);
        divider.setFillColor(UITheme::BorderLight);
        window.draw(divider);

        graphRenderer.render(window, font, rightCanvasBounds);

        // Header labels for each split pane
        sf::Text nfaHeader;
        nfaHeader.setFont(font);
        nfaHeader.setString("ε-NFA (Active Subset Glowing)");
        nfaHeader.setCharacterSize(13);
        nfaHeader.setStyle(sf::Text::Bold);
        nfaHeader.setFillColor(UITheme::AccentCyan);
        nfaHeader.setPosition(leftCanvasBounds.left + 14.0f, leftCanvasBounds.top + 8.0f);
        window.draw(nfaHeader);

        sf::Text dfaHeader;
        dfaHeader.setFont(font);
        dfaHeader.setString("Constructed DFA");
        dfaHeader.setCharacterSize(13);
        dfaHeader.setStyle(sf::Text::Bold);
        dfaHeader.setFillColor(UITheme::AccentGreen);
        dfaHeader.setPosition(rightCanvasBounds.left + 14.0f, rightCanvasBounds.top + 8.0f);
        window.draw(dfaHeader);
    } else {
        graphRenderer.render(window, font, canvasBounds);
    }

    renderControlsBar();

    if (currentMode == ViewMode::STRING_SIMULATION) {
        renderSimulationBar();
    }

    renderTopBar();
    modeTabBar.render(window, font);
    infoPanel.render(window, font);

    window.display();
}

void MainWindow::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        processEvents();
        animationCtrl.update(dt);
        regexInput.update(dt);
        if (currentMode == ViewMode::STRING_SIMULATION) {
            simStringInput.update(dt);
        }

        render();
    }
}

} // namespace AutomataViz
