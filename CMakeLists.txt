cmake_minimum_required(VERSION 3.16)
project(AutomataViz VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Discover SFML
find_package(SFML 2.5 COMPONENTS graphics window system QUIET)

# Core Algorithm Library (Automata, Regex, Transformations)
set(CORE_SOURCES
    src/Regex/RegexValidator.cpp
    src/Regex/RegexParser.cpp
    src/Automata/NFA.cpp
    src/Automata/DFA.cpp
    src/Algorithms/ThompsonConstruction.cpp
    src/Algorithms/SubsetConstruction.cpp
    src/Algorithms/DFAMinimization.cpp
    src/Algorithms/StringSimulator.cpp
)

add_library(AutomataCore STATIC ${CORE_SOURCES})
target_include_directories(AutomataCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# UI and Visualization Sources
set(APP_SOURCES
    src/main.cpp
    src/Visualization/GraphLayout.cpp
    src/Visualization/GraphRenderer.cpp
    src/Visualization/Animation.cpp
    src/UI/Controls.cpp
    src/UI/InfoPanel.cpp
    src/UI/TableView.cpp
    src/UI/MainWindow.cpp
)

add_executable(AutomataViz ${APP_SOURCES})
target_include_directories(AutomataViz PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(AutomataViz PRIVATE AutomataCore)

if(SFML_FOUND)
    target_link_libraries(AutomataViz PRIVATE sfml-graphics sfml-window sfml-system)
else()
    # Fallback to direct library linking for MinGW / MSYS2 UCRT64
    find_library(SFML_GRAPHICS_LIB sfml-graphics)
    find_library(SFML_WINDOW_LIB sfml-window)
    find_library(SFML_SYSTEM_LIB sfml-system)

    if(SFML_GRAPHICS_LIB AND SFML_WINDOW_LIB AND SFML_SYSTEM_LIB)
        target_link_libraries(AutomataViz PRIVATE ${SFML_GRAPHICS_LIB} ${SFML_WINDOW_LIB} ${SFML_SYSTEM_LIB})
    else()
        target_link_libraries(AutomataViz PRIVATE -lsfml-graphics -lsfml-window -lsfml-system)
    endif()
endif()

# Automated Test Suite Executable
add_executable(AutomataViz_Tests tests/test_runner.cpp)
target_include_directories(AutomataViz_Tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(AutomataViz_Tests PRIVATE AutomataCore)

enable_testing()
add_test(NAME TOC_Algorithms_Verification COMMAND AutomataViz_Tests)
