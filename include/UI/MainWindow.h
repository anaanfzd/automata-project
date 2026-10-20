#ifndef AUTOMATAVIZ_MAINWINDOW_H
#define AUTOMATAVIZ_MAINWINDOW_H

#include "Regex/RegexParser.h"
#include "Algorithms/ThompsonConstruction.h"
#include "Algorithms/SubsetConstruction.h"
#include "Algorithms/DFAMinimization.h"
#include "Algorithms/StringSimulator.h"
#include "Visualization/GraphRenderer.h"
#include "Visualization/Animation.h"
#include "UI/Controls.h"
#include "UI/InfoPanel.h"
#include "UI/TableView.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace AutomataViz {

enum class ViewMode {
    REGEX_POSTFIX,
    THOMPSON_NFA,
    SUBSET_DFA,
    DFA_VIEW,
    DFA_MINIMIZATION,
    STRING_SIMULATION,
    THEORY_GUIDE
};

enum class DFADisplayType {
    GRAPH_VIEW,
    TABLE_VIEW
};

class MainWindow {
public:
    MainWindow(unsigned int width = 1360, unsigned int height = 820);
    ~MainWindow();

    bool init();
    void run();

private:
    unsigned int windowWidth;
    unsigned int windowHeight;
    sf::RenderWindow window;
    sf::Font font;
    sf::Font fontMono;

    // Pipeline Engine Data
    std::string currentRegexStr;
    ParseResult parseResult;
    ThompsonResult thompsonResult;
    SubsetResult subsetResult;
    MinimizationResult minimizationResult;
    StringSimulator stringSimulator;

    // Pipeline Step Tracking
    ViewMode currentMode;
    DFADisplayType dfaDisplayType;
    int currentThompsonStep;
    int currentSubsetStep;
    int currentMinimizationStep;

    // Dual View / Split Screen state (NFA on Left, DFA on Right in Subset Construction)
    bool isDualView;

    // UI Widgets
    TextInput regexInput;
    Button generateBtn;

    std::vector<Button> exampleButtons;
    TabBar modeTabBar;

    // Step Control Buttons
    Button prevStepBtn;
    Button nextStepBtn;
    Button playPauseBtn;
    Button resetStepBtn;
    Button speedBtn;
    int speedLevel;

    // View Control Buttons
    Button zoomInBtn;
    Button zoomOutBtn;
    Button resetViewBtn;
    Button toggleTableBtn;    // Switch between Graph / Table in DFA mode
    Button toggleDualViewBtn; // Toggle Side-by-Side NFA+DFA split view

    // String Simulation Controls
    TextInput simStringInput;
    Button simStepBtn;
    Button simResetBtn;
    Button simRunBtn;

    // Panels & Views
    InfoPanel infoPanel;
    GraphRenderer graphRenderer;
    GraphRenderer nfaSideRenderer; // For split-screen NFA display
    TableView tableView;
    AnimationController animationCtrl;

    // Layout geometry
    sf::FloatRect topBarBounds;
    sf::FloatRect tabBarBounds;
    sf::FloatRect controlBarBounds;
    sf::FloatRect canvasBounds;
    sf::FloatRect leftCanvasBounds;  // In dual-view: NFA
    sf::FloatRect rightCanvasBounds; // In dual-view: DFA
    sf::FloatRect infoPanelBounds;
    sf::FloatRect simBarBounds;

    // Theory guide scrolling
    float theoryScrollY;
    float maxTheoryScrollY;

    // Layout Caching Flags to prevent laggy recalculations
    bool nfaFullLayoutComputed;
    bool dfaFullLayoutComputed;
    bool minDfaLayoutComputed;

    // Initialization & Lifecycle
    bool loadFonts();
    void setupLayout();
    void setupUI();

    // Event Handling
    void processEvents();
    void handleResize(unsigned int newWidth, unsigned int newHeight);
    void handleExampleSelect(int index);

    // Pipeline Execution
    void executePipeline(const std::string& regexStr);
    void updateGraphForCurrentState();
    void updateInfoPanel();
    void cycleSpeed();

    // Step Navigation
    void goToPreviousStep();
    void goToNextStep();
    void resetCurrentStep();

    // Render Methods
    void render();
    void renderTopBar();
    void renderControlsBar();
    void renderSimulationBar();
    void renderRegexPostfixView();
    void renderTheoryGuide();

    // Helper conversion from NFA/DFA to Graph Renderer nodes & edges
    void loadNFAIntoRenderer(GraphRenderer& targetRenderer, const NFA& nfa,
                             const std::vector<int>& highlightedStates = {},
                             const std::vector<Transition>& highlightedTransitions = {},
                             bool recomputeLayout = false,
                             const sf::FloatRect& bounds = {0.f, 0.f, 800.f, 500.f});
    void loadDFAIntoRenderer(const DFA& dfa, int highlightedState = -1, int currentSimState = -1,
                             const std::pair<int, char>& currentSimTrans = {-1, '\0'},
                             bool recomputeLayout = false,
                             const sf::FloatRect& bounds = {0.f, 0.f, 800.f, 500.f});
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_MAINWINDOW_H
