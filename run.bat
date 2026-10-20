@echo off
setlocal enabledelayedexpansion

REM Ensure ucrt64 DLLs are in PATH for SFML runtime
if exist "C:\msys64\ucrt64\bin" (
    set "PATH=C:\msys64\ucrt64\bin;!PATH!"
)

if not exist "bin\AutomataViz.exe" (
    echo [Info] Application not built yet. Building now...
    call build.bat
    if %errorlevel% neq 0 exit /b 1
)

echo Starting AutomataViz...
start "" "bin\AutomataViz.exe"
