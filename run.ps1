<#
.SYNOPSIS
    Convenient build and test runner for the Toy Compiler project.
.DESCRIPTION
    Ensures MinGW/Bison/Flex PATH is set, builds the compiler, and runs tests.
.EXAMPLE
    .\run.ps1
    Runs all tests.
.EXAMPLE
    .\run.ps1 6
    Runs test6_const_fold.toy.
.EXAMPLE
    .\run.ps1 tests\test7_dce.toy
    Runs the specific test file.
.EXAMPLE
    .\run.ps1 -BuildOnly
    Only builds the compiler.
#>
param(
    [Parameter(Position=0)]
    [string]$Target = "all",

    [switch]$BuildOnly,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Ensure MSYS2 UCRT64 (MinGW) takes precedence over usr/bin (MSYS posix)
$cleanPath = ($env:PATH -split ';' | Where-Object { $_ -notlike '*msys64*' }) -join ';'
$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$cleanPath"

if ($Clean) {
    Write-Host "[Clean] Removing build artifacts..." -ForegroundColor Yellow
    mingw32-make clean
    return
}

# Always build first
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host " Building Compiler (mingw32-make)..." -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
mingw32-make

if ($LASTEXITCODE -ne 0) {
    Write-Host "[Error] Build failed!" -ForegroundColor Red
    exit 1
}

if ($BuildOnly) {
    Write-Host "[Success] Build finished." -ForegroundColor Green
    return
}

$Compiler = ".\bin\compiler.exe"
if (-not (Test-Path $Compiler)) {
    Write-Host "[Error] $Compiler not found!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "==========================================" -ForegroundColor Green
Write-Host " Running Tests" -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green

if ($Target -eq "all") {
    $testFiles = Get-ChildItem -Path "tests" -Filter "*.toy" | Sort-Object Name
    foreach ($file in $testFiles) {
        Write-Host ""
        Write-Host ">>> Running: $($file.Name) <<<" -ForegroundColor Magenta
        & $Compiler $file.FullName
        Write-Host "------------------------------------------" -ForegroundColor DarkGray
    }
}
elseif ($Target -match "^\d+$") {
    # If the user passed just a number like 1, 6, 7
    $match = Get-ChildItem -Path "tests" -Filter "test$Target*.toy" | Select-Object -First 1
    if ($match) {
        Write-Host ">>> Running: $($match.Name) <<<" -ForegroundColor Magenta
        & $Compiler $match.FullName
    } else {
        Write-Host "[Error] No test found matching test number $Target" -ForegroundColor Red
    }
}
elseif (Test-Path $Target) {
    Write-Host ">>> Running: $Target <<<" -ForegroundColor Magenta
    & $Compiler $Target
}
elseif (Test-Path "tests\$Target") {
    Write-Host ">>> Running: tests\$Target <<<" -ForegroundColor Magenta
    & $Compiler "tests\$Target"
}
else {
    Write-Host "[Error] Could not find test file or target: $Target" -ForegroundColor Red
}
