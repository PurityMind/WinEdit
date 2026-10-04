#include "Editor.hpp"
#include <iostream>
#include <shellapi.h>

int wmain(int argc, wchar_t* argv[]) {
    // Enable UTF-8 console output for help text
    SetConsoleOutputCP(CP_UTF8);

    std::wstring targetFile;
    if (argc > 1) {
        std::wstring arg = argv[1];
        if (arg == L"--version" || arg == L"-v" || arg == L"/v") {
            std::wcout << Version::FullName << L" (Build " << Version::RetroBuild << L")\n";
            std::wcout << L"Native Windows Console TUI Text Editor\n";
            return 0;
        }
        if (arg == L"--help" || arg == L"-h" || arg == L"/?") {
            std::wcout << Version::FullName << L" - Native Windows TUI Text Editor\n\n";
            std::wcout << L"Usage: winedit.exe [filename] [options]\n\n";
            std::wcout << L"Options:\n";
            std::wcout << L"  -h, --help       Show this help message\n";
            std::wcout << L"  -v, --version    Show version and build information\n\n";
            std::wcout << L"Shortcuts:\n";
            std::wcout << L"  Alt + F/E/S/V/H  Open File, Edit, Search, View, Help menus\n";
            std::wcout << L"  Alt + T          Cycle themes (Win98 Dark, Notepad, DOS Edit)\n";
            std::wcout << L"  Ctrl + S / W     Save / Save As\n";
            std::wcout << L"  Ctrl + Q         Exit editor\n";
            std::wcout << L"  Ctrl + F / F3    Find / Find next\n";
            std::wcout << L"  Ctrl + G         Go to line\n";
            std::wcout << L"  Ctrl + Z / Y     Undo / Redo\n";
            std::wcout << L"  Ctrl + C / X / V Copy / Cut / Paste\n";
            std::wcout << L"  Ctrl + D         Delete line\n";
            std::wcout << L"  Ctrl + H / F1    Keyboard help overlay\n";
            std::wcout << L"  Tab / Shift+Tab  Indent / Unindent 4 spaces\n";
            return 0;
        }
        targetFile = arg;
    }

    Editor editor;
    if (!editor.init(targetFile)) {
        std::wcerr << L"Failed to initialize console interface.\n";
        return 1;
    }

    editor.run();
    return 0;
}
