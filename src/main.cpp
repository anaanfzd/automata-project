#include "UI/MainWindow.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "========================================================" << std::endl;
    std::cout << "  AutomataViz — Interactive RE to DFA Visualizer" << std::endl;
    std::cout << "  Theory of Computation College Project" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "[Init] Launching SFML graphical environment..." << std::endl;

    AutomataViz::MainWindow window(1360, 820);
    if (!window.init()) {
        std::cerr << "[Error] Failed to initialize AutomataViz application window." << std::endl;
        return 1;
    }

    std::cout << "[Ready] Application initialized. Starting main event loop." << std::endl;
    window.run();

    std::cout << "[Exit] AutomataViz terminated normally." << std::endl;
    return 0;
}
