#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cwctype>

namespace Version {
    constexpr int Major = 1;
    constexpr int Minor = 3;
    constexpr int Patch = 0;
    constexpr const wchar_t* String = L"1.3.0";
    constexpr const wchar_t* RetroBuild = L"4.10.1998 (Second Edition)";
    constexpr const wchar_t* FullName = L"WinEdit 98 v1.3.0";
}

namespace Color {
    // Foreground colors
    constexpr WORD Black        = 0;
    constexpr WORD DarkBlue     = FOREGROUND_BLUE;
    constexpr WORD DarkGreen    = FOREGROUND_GREEN;
    constexpr WORD DarkCyan     = FOREGROUND_GREEN | FOREGROUND_BLUE;
    constexpr WORD DarkRed      = FOREGROUND_RED;
    constexpr WORD DarkMagenta  = FOREGROUND_RED | FOREGROUND_BLUE;
    constexpr WORD DarkYellow   = FOREGROUND_RED | FOREGROUND_GREEN;
    constexpr WORD Gray         = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    constexpr WORD DarkGray     = FOREGROUND_INTENSITY;
    constexpr WORD Blue         = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    constexpr WORD Green        = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    constexpr WORD Cyan         = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    constexpr WORD Red          = FOREGROUND_RED | FOREGROUND_INTENSITY;
    constexpr WORD Magenta      = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    constexpr WORD Yellow       = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    constexpr WORD White        = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

    // Background colors
    constexpr WORD BgBlack      = 0;
    constexpr WORD BgDarkBlue   = BACKGROUND_BLUE;
    constexpr WORD BgDarkGreen  = BACKGROUND_GREEN;
    constexpr WORD BgDarkCyan   = BACKGROUND_GREEN | BACKGROUND_BLUE;
    constexpr WORD BgDarkRed    = BACKGROUND_RED;
    constexpr WORD BgDarkMagenta= BACKGROUND_RED | BACKGROUND_BLUE;
    constexpr WORD BgDarkYellow = BACKGROUND_RED | BACKGROUND_GREEN;
    constexpr WORD BgGray       = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
    constexpr WORD BgDarkGray   = BACKGROUND_INTENSITY;
    constexpr WORD BgBlue       = BACKGROUND_BLUE | BACKGROUND_INTENSITY;
    constexpr WORD BgGreen      = BACKGROUND_GREEN | BACKGROUND_INTENSITY;
    constexpr WORD BgCyan       = BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
    constexpr WORD BgRed        = BACKGROUND_RED | BACKGROUND_INTENSITY;
    constexpr WORD BgMagenta    = BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
    constexpr WORD BgYellow     = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
    constexpr WORD BgWhite      = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
}

enum class EventType {
    None,
    Key,
    Mouse,
    Resize
};

struct KeyEvent {
    WORD vkCode = 0;
    wchar_t ch = 0;
    bool ctrl = false;
    bool alt = false;
    bool shift = false;
    WORD repeatCount = 1;
};

struct MouseEvent {
    int x = 0;
    int y = 0;
    bool leftButton = false;
    bool rightButton = false;
    int wheelDelta = 0; // Positive = up, negative = down
};

struct AppEvent {
    EventType type = EventType::None;
    KeyEvent key;
    MouseEvent mouse;
    int newWidth = 0;
    int newHeight = 0;
};

// Auto-closing pair helper functions
inline bool isMatchingPair(wchar_t open, wchar_t close) {
    if (open == L'(' && close == L')') return true;
    if (open == L'[' && close == L']') return true;
    if (open == L'{' && close == L'}') return true;
    if (open == L'"' && close == L'"') return true;
    if (open == L'\'' && close == L'\'') return true;
    if (open == L'`' && close == L'`') return true;
    return false;
}

inline wchar_t getClosingPair(wchar_t open) {
    switch (open) {
        case L'(': return L')';
        case L'[': return L']';
        case L'{': return L'}';
        case L'"': return L'"';
        case L'\'': return L'\'';
        case L'`': return L'`';
        default: return 0;
    }
}

inline bool isClosingChar(wchar_t ch) {
    return ch == L')' || ch == L']' || ch == L'}' || ch == L'"' || ch == L'\'' || ch == L'`';
}

// Windows 9x Themes
enum class ThemeType {
    Win98Dark,     // Navy Blue title, Gray menus, Dark IDE canvas with bright syntax
    Win95Notepad,  // Classic Win95 Notepad: White canvas, Black text, Gray chrome
    DosEditBlue    // MS-DOS 7.0 / QBasic: Deep Blue canvas, White text, Cyan margin
};

struct ThemeColors {
    WORD titleBarActive;
    WORD titleBarButton;
    WORD titleBarCloseBtn;

    WORD menuBar;
    WORD menuBarHotkey;
    WORD menuDropdown;
    WORD menuDropdownSelected;
    WORD menuDropdownBorder;
    WORD menuDropdownHotkey;
    WORD menuDropdownShadow;

    WORD editorBg;
    WORD textDefault;
    WORD margin;
    WORD marginCurrent;
    WORD marginSep;
    WORD selection;
    WORD searchMatch;

    WORD scrollbarTrack;
    WORD scrollbarThumb;
    WORD scrollbarArrow;

    WORD statusBar;
    WORD statusBarPanel;
    WORD statusBarBorder;

    WORD dialogTitle;
    WORD dialogBody;
    WORD dialogBorder;
    WORD dialogInput;
    WORD dialogButton;
    WORD dialogButtonFocus;
};

inline ThemeColors getThemeColors(ThemeType type) {
    ThemeColors tc{};
    if (type == ThemeType::Win95Notepad) {
        tc.titleBarActive = Color::White | Color::BgDarkBlue;
        tc.titleBarButton = Color::Black | Color::BgGray;
        tc.titleBarCloseBtn = Color::Black | Color::BgGray;

        tc.menuBar = Color::Black | Color::BgGray;
        tc.menuBarHotkey = Color::DarkBlue | Color::BgGray;
        tc.menuDropdown = Color::Black | Color::BgGray;
        tc.menuDropdownSelected = Color::White | Color::BgDarkBlue;
        tc.menuDropdownBorder = Color::Black | Color::BgGray;
        tc.menuDropdownHotkey = Color::DarkBlue | Color::BgGray;
        tc.menuDropdownShadow = Color::DarkGray | Color::BgBlack;

        tc.editorBg = Color::BgWhite;
        tc.textDefault = Color::Black | Color::BgWhite;
        tc.margin = Color::DarkBlue | Color::BgWhite;
        tc.marginCurrent = Color::DarkRed | Color::BgWhite;
        tc.marginSep = Color::DarkGray | Color::BgWhite;
        tc.selection = Color::White | Color::BgDarkBlue;
        tc.searchMatch = Color::Yellow | Color::BgDarkRed;

        tc.scrollbarTrack = Color::DarkGray | Color::BgGray;
        tc.scrollbarThumb = Color::White | Color::BgDarkGray;
        tc.scrollbarArrow = Color::Black | Color::BgGray;

        tc.statusBar = Color::Black | Color::BgGray;
        tc.statusBarPanel = Color::Black | Color::BgGray;
        tc.statusBarBorder = Color::DarkGray | Color::BgGray;

        tc.dialogTitle = Color::White | Color::BgDarkBlue;
        tc.dialogBody = Color::Black | Color::BgGray;
        tc.dialogBorder = Color::Black | Color::BgGray;
        tc.dialogInput = Color::Black | Color::BgWhite;
        tc.dialogButton = Color::Black | Color::BgGray;
        tc.dialogButtonFocus = Color::White | Color::BgDarkBlue;
    } else if (type == ThemeType::DosEditBlue) {
        tc.titleBarActive = Color::Black | Color::BgGray;
        tc.titleBarButton = Color::Black | Color::BgGray;
        tc.titleBarCloseBtn = Color::Black | Color::BgGray;

        tc.menuBar = Color::Black | Color::BgGray;
        tc.menuBarHotkey = Color::DarkRed | Color::BgGray;
        tc.menuDropdown = Color::Black | Color::BgGray;
        tc.menuDropdownSelected = Color::Black | Color::BgCyan;
        tc.menuDropdownBorder = Color::Black | Color::BgGray;
        tc.menuDropdownHotkey = Color::DarkRed | Color::BgGray;
        tc.menuDropdownShadow = Color::Black | Color::BgBlack;

        tc.editorBg = Color::BgDarkBlue;
        tc.textDefault = Color::White | Color::BgDarkBlue;
        tc.margin = Color::Cyan | Color::BgDarkBlue;
        tc.marginCurrent = Color::Yellow | Color::BgDarkBlue;
        tc.marginSep = Color::DarkCyan | Color::BgDarkBlue;
        tc.selection = Color::Black | Color::BgCyan;
        tc.searchMatch = Color::Yellow | Color::BgDarkRed;

        tc.scrollbarTrack = Color::DarkCyan | Color::BgDarkBlue;
        tc.scrollbarThumb = Color::White | Color::BgCyan;
        tc.scrollbarArrow = Color::Black | Color::BgCyan;

        tc.statusBar = Color::Black | Color::BgCyan;
        tc.statusBarPanel = Color::Black | Color::BgCyan;
        tc.statusBarBorder = Color::DarkCyan | Color::BgCyan;

        tc.dialogTitle = Color::White | Color::BgDarkBlue;
        tc.dialogBody = Color::Black | Color::BgGray;
        tc.dialogBorder = Color::Black | Color::BgGray;
        tc.dialogInput = Color::White | Color::BgDarkBlue;
        tc.dialogButton = Color::Black | Color::BgGray;
        tc.dialogButtonFocus = Color::Black | Color::BgCyan;
    } else { // Win98Dark (Default)
        tc.titleBarActive = Color::White | Color::BgDarkBlue;
        tc.titleBarButton = Color::Black | Color::BgGray;
        tc.titleBarCloseBtn = Color::Black | Color::BgGray;

        tc.menuBar = Color::Black | Color::BgGray;
        tc.menuBarHotkey = Color::DarkRed | Color::BgGray;
        tc.menuDropdown = Color::Black | Color::BgGray;
        tc.menuDropdownSelected = Color::White | Color::BgDarkBlue;
        tc.menuDropdownBorder = Color::Black | Color::BgGray;
        tc.menuDropdownHotkey = Color::DarkRed | Color::BgGray;
        tc.menuDropdownShadow = Color::DarkGray | Color::BgBlack;

        tc.editorBg = Color::BgBlack;
        tc.textDefault = Color::White | Color::BgBlack;
        tc.margin = Color::DarkCyan | Color::BgBlack;
        tc.marginCurrent = Color::Yellow | Color::BgBlack;
        tc.marginSep = Color::DarkGray | Color::BgBlack;
        tc.selection = Color::White | Color::BgDarkBlue;
        tc.searchMatch = Color::Yellow | Color::BgDarkRed;

        tc.scrollbarTrack = Color::DarkGray | Color::BgBlack;
        tc.scrollbarThumb = Color::White | Color::BgGray;
        tc.scrollbarArrow = Color::Black | Color::BgGray;

        tc.statusBar = Color::Black | Color::BgGray;
        tc.statusBarPanel = Color::Black | Color::BgGray;
        tc.statusBarBorder = Color::DarkGray | Color::BgGray;

        tc.dialogTitle = Color::White | Color::BgDarkBlue;
        tc.dialogBody = Color::Black | Color::BgGray;
        tc.dialogBorder = Color::Black | Color::BgGray;
        tc.dialogInput = Color::Black | Color::BgWhite;
        tc.dialogButton = Color::Black | Color::BgGray;
        tc.dialogButtonFocus = Color::White | Color::BgDarkBlue;
    }
    return tc;
}

// Windows 9x Menu Actions
enum MenuAction {
    ACTION_NONE = 0,
    ACTION_NEW,
    ACTION_OPEN,
    ACTION_SAVE,
    ACTION_SAVE_AS,
    ACTION_EXIT,
    ACTION_UNDO,
    ACTION_REDO,
    ACTION_CUT,
    ACTION_COPY,
    ACTION_PASTE,
    ACTION_DELETE_LINE,
    ACTION_SELECT_ALL,
    ACTION_FIND,
    ACTION_FIND_NEXT,
    ACTION_FIND_PREV,
    ACTION_GOTO_LINE,
    ACTION_THEME_WIN98,
    ACTION_THEME_NOTEPAD,
    ACTION_THEME_DOSEDIT,
    ACTION_TOGGLE_LINE_NUMS,
    ACTION_TOGGLE_AUTOPAIRS,
    ACTION_HELP_TOPICS,
    ACTION_SHORTCUTS,
    ACTION_ABOUT
};

struct MenuItem {
    std::wstring label;
    std::wstring shortcut;
    wchar_t hotkeyChar = 0;
    int actionId = ACTION_NONE;
    bool isSeparator = false;
};

struct Menu {
    std::wstring title;
    wchar_t hotkeyChar = 0;
    std::vector<MenuItem> items;
    int startX = 0;
    int width = 0;
};
