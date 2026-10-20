# AutomataViz PowerShell Launch Script
$ErrorActionPreference = "Stop"

if (Test-Path "C:\msys64\ucrt64\bin") {
    $env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
}

if (-not (Test-Path "bin\AutomataViz.exe")) {
    Write-Host "[Info] Application binary not found. Building now..." -ForegroundColor Yellow
    & .\build.ps1
    if ($LASTEXITCODE -ne 0) { exit 1 }
}

Write-Host "Launching AutomataViz..." -ForegroundColor Green
Start-Process "bin\AutomataViz.exe"
