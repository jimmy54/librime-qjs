@echo off
setlocal enabledelayedexpansion

:: Get the script directory and set root path
set "root=%~dp0.."

:: Get version from CMake output
for /f "tokens=4" %%i in ('cmake "%root%" 2^>^&1 ^| findstr "LibrimeQjs version:"') do (
    set "version=%%i"
)

:: Check if version was found
if not defined version (
    echo Error: Could not extract version number
    exit /b 1
)

:: Get git reference if Nightly parameter is passed
if "%1"=="Nightly" (
    for /f "tokens=*" %%i in ('git describe --always') do set "gitref=%%i"
    set "version=%version%+!gitref!"
)

:: Update version in rime.d.ts using PowerShell (more reliable than batch file string replacement)
powershell -Command "(Get-Content '%root%\contrib\rime.d.ts') -replace 'LIB_RIME_QJS_VERSION', '%version%' | Set-Content '%root%\contrib\rime.d.ts'"

echo Updated version to %version% in rime.d.ts
