@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo   AutomataViz - Build Script
echo   Interactive Regular Expression to DFA Visualizer
echo ========================================================

REM Add MSYS2 / UCRT64 toolchain to PATH if available
if exist "C:\msys64\ucrt64\bin" (
    set "PATH=C:\msys64\ucrt64\bin;!PATH!"
)
if exist "C:\Program Files\CMake\bin" (
    set "PATH=C:\Program Files\CMake\bin;!PATH!"
)

REM Check for compiler
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo [Error] g++ compiler not found in PATH or C:\msys64\ucrt64\bin.
    echo Please ensure MinGW-w64 or MSYS2 UCRT64 is installed.
    exit /b 1
)

REM Create directories
if not exist "bin" mkdir bin

echo [1/2] Compiling AutomataViz Core and GUI Application...
g++ -std=c++17 -O2 -Iinclude ^
    src/Regex/RegexValidator.cpp ^
    src/Regex/RegexParser.cpp ^
    src/Automata/NFA.cpp ^
    src/Automata/DFA.cpp ^
    src/Algorithms/ThompsonConstruction.cpp ^
    src/Algorithms/SubsetConstruction.cpp ^
    src/Algorithms/DFAMinimization.cpp ^
    src/Algorithms/StringSimulator.cpp ^
    src/Visualization/GraphLayout.cpp ^
    src/Visualization/GraphRenderer.cpp ^
    src/Visualization/Animation.cpp ^
    src/UI/Controls.cpp ^
    src/UI/InfoPanel.cpp ^
    src/UI/TableView.cpp ^
    src/UI/MainWindow.cpp ^
    src/main.cpp ^
    -lsfml-graphics -lsfml-window -lsfml-system ^
    -o bin/AutomataViz.exe

if %errorlevel% neq 0 (
    echo [Error] Compilation failed for AutomataViz.exe.
    exit /b 1
)
echo [Success] Built bin/AutomataViz.exe

echo [2/2] Compiling AutomataViz Verification Test Suite...
g++ -std=c++17 -O2 -Iinclude ^
    src/Regex/RegexValidator.cpp ^
    src/Regex/RegexParser.cpp ^
    src/Automata/NFA.cpp ^
    src/Automata/DFA.cpp ^
    src/Algorithms/ThompsonConstruction.cpp ^
    src/Algorithms/SubsetConstruction.cpp ^
    src/Algorithms/DFAMinimization.cpp ^
    src/Algorithms/StringSimulator.cpp ^
    tests/test_runner.cpp ^
    -o bin/AutomataViz_Tests.exe

if %errorlevel% neq 0 (
    echo [Error] Compilation failed for AutomataViz_Tests.exe.
    exit /b 1
)
echo [Success] Built bin/AutomataViz_Tests.exe

echo.
echo ========================================================
echo   Build Completed Successfully!
echo   Run the application:  bin\AutomataViz.exe
echo   Run automated tests:  bin\AutomataViz_Tests.exe
echo ========================================================
