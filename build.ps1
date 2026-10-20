# AutomataViz PowerShell Build Script
$ErrorActionPreference = "Stop"

Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "  AutomataViz - High-Performance Build Engine" -ForegroundColor White
Write-Host "  Interactive Regular Expression to DFA Visualizer" -ForegroundColor DarkGray
Write-Host "========================================================" -ForegroundColor Cyan

# Ensure UCRT64 toolchain and CMake are in PATH
if (Test-Path "C:\msys64\ucrt64\bin") {
    $env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
}
if (Test-Path "C:\Program Files\CMake\bin") {
    $env:PATH = "C:\Program Files\CMake\bin;$env:PATH"
}

# Verify g++ compiler
$gpp = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $gpp) {
    Write-Host "[Error] g++ compiler was not found in PATH." -ForegroundColor Red
    Write-Host "Please ensure MSYS2 (ucrt64) or MinGW-w64 is installed." -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path "bin")) {
    New-Item -ItemType Directory -Path "bin" | Out-Null
}

Write-Host "[1/2] Compiling AutomataViz Core and Desktop GUI Application..." -ForegroundColor Yellow
& g++ -std=c++17 -O2 -Iinclude `
    src/Regex/RegexValidator.cpp `
    src/Regex/RegexParser.cpp `
    src/Automata/NFA.cpp `
    src/Automata/DFA.cpp `
    src/Algorithms/ThompsonConstruction.cpp `
    src/Algorithms/SubsetConstruction.cpp `
    src/Algorithms/DFAMinimization.cpp `
    src/Algorithms/StringSimulator.cpp `
    src/Visualization/GraphLayout.cpp `
    src/Visualization/GraphRenderer.cpp `
    src/Visualization/Animation.cpp `
    src/UI/Controls.cpp `
    src/UI/InfoPanel.cpp `
    src/UI/TableView.cpp `
    src/UI/MainWindow.cpp `
    src/main.cpp `
    -lsfml-graphics -lsfml-window -lsfml-system `
    -o bin/AutomataViz.exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "[Error] Compilation failed for AutomataViz.exe" -ForegroundColor Red
    exit 1
}
Write-Host "[Success] Built bin/AutomataViz.exe" -ForegroundColor Green

Write-Host "[2/2] Compiling AutomataViz Verification Test Suite..." -ForegroundColor Yellow
& g++ -std=c++17 -O2 -Iinclude `
    src/Regex/RegexValidator.cpp `
    src/Regex/RegexParser.cpp `
    src/Automata/NFA.cpp `
    src/Automata/DFA.cpp `
    src/Algorithms/ThompsonConstruction.cpp `
    src/Algorithms/SubsetConstruction.cpp `
    src/Algorithms/DFAMinimization.cpp `
    src/Algorithms/StringSimulator.cpp `
    tests/test_runner.cpp `
    -o bin/AutomataViz_Tests.exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "[Error] Compilation failed for AutomataViz_Tests.exe" -ForegroundColor Red
    exit 1
}
Write-Host "[Success] Built bin/AutomataViz_Tests.exe" -ForegroundColor Green

Write-Host "`n========================================================" -ForegroundColor Cyan
Write-Host "  Build Completed Successfully!" -ForegroundColor Green
Write-Host "  Launch Application:  .\run.ps1   (or .\bin\AutomataViz.exe)" -ForegroundColor White
Write-Host "  Run Unit Tests:      .\bin\AutomataViz_Tests.exe" -ForegroundColor White
Write-Host "========================================================" -ForegroundColor Cyan
