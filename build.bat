@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   Building WinEdit - Native Windows TUI Editor
echo ===================================================

where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo Searching for Visual Studio environment...
    
    set "VS_PATH="
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (
        set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
        set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
        set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
        set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
    )

    if defined VS_PATH (
        echo Initializing MSVC from: !VS_PATH!
        call "!VS_PATH!" x64
    ) else (
        echo [ERROR] MSVC cl.exe not found! Please run from Developer Command Prompt for Visual Studio.
        exit /b 1
    )
)

echo Compiling C++ sources...
cl /nologo /EHsc /std:c++17 /utf-8 /W4 /O2 /Iinclude src\Console.cpp src\Clipboard.cpp src\Document.cpp src\Syntax.cpp src\Editor.cpp src\main.cpp /Fe:winedit.exe user32.lib shell32.lib

if %errorlevel% equ 0 (
    echo.
    echo ===================================================
    echo   BUILD SUCCESSFUL: winedit.exe created!
    echo ===================================================
    del *.obj >nul 2>nul
) else (
    echo.
    echo [ERROR] Build failed with exit code %errorlevel%
    exit /b %errorlevel%
)
