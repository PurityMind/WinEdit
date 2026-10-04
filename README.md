# WinEdit - Windows TUI Text Editor (C++)

A fast, lightweight, dependency-free **Terminal User Interface (TUI) Text Editor** written in modern C++ (C++17) specifically designed for **Microsoft Windows**.

WinEdit communicates directly with the native Windows Console API (`windows.h`) without requiring third-party libraries (no ncurses, PDCurses, or external dependencies).

---

## ✨ Features

- **Authentic Windows 9x / Classic Desktop UI**:
  - **Window Title Bar**: Navy blue title bar with `[■]` app icon, document name `WinEdit 98 - [file] *`, and clickable exit button `[ X ]` on the right.
  - **Windows 9x Menu Bar**: Dropdown menus (`File`, `Edit`, `Search`, `View`, `Help`) with hotkeys (`Alt+F`, `Alt+E`, `Alt+S`, `Alt+V`, `Alt+H`), `F10` activation, single-line borders, drop shadows, and mouse click navigation.
  - **Windows 9x Scrollbar**: Dedicated right column featuring `▲` up button, proportional `█` thumb slider, `░` track, and `▼` down button; responds to mouse clicks and wheel.
  - **Sunken 3D Multi-Pane Status Bar**: Classic beveled panels `[ Ready ] │ [ Ln X, Col Y ] │ [ 100% ] │ [ Lang ] │ [ INS ] │ [ CRLF ]`.
  - **Windows 9x 3D Modal Dialogs**: Authentic gray dialog boxes with navy title bars, sunken white input boxes (`[ text ]`), and push buttons (`[ OK ]`, `[ Cancel ]`, `[ Find Next ]`, `[ Save ]`) for Find, Go To Line, Save As, and About WinEdit 98.
  - **3 Authentic Built-In Themes**:
    - **Windows 98 Dark** (Default): Navy blue title bar, gray chrome, dark canvas with bright syntax colors.
    - **Windows 95 Notepad**: Pure white canvas, black text, dark blue/red syntax, classic Notepad styling.
    - **MS-DOS 7.0 Edit**: Iconic EDIT.COM / QBasic deep blue canvas, white text, and cyan margins/status bar.
    - Switch themes anytime with `Alt+T` or via the `View -> Theme` menu.
- **Native Windows Console Architecture**:
  - **Alternate Screen Buffer**: Launches in an isolated screen buffer (`CreateConsoleScreenBuffer`). When you exit, your command prompt, history, and terminal state are 100% restored.
  - **Flicker-Free Double Buffering**: Uses `WriteConsoleOutputW` to update the screen atomically in a single system call.
  - **Dynamic Console Resizing**: Automatically detects window resize events and recalculates viewport, wrapping, and margin layout.
  - **Mouse Support**: Click anywhere in the text area to place the cursor; click menus and dropdown items; click scrollbar arrows/track; click title bar `[ X ]`; and use the mouse wheel to scroll.
- **Windows System Clipboard Integration**:
  - `Ctrl + C`, `Ctrl + X`, and `Ctrl + V` interact directly with the Windows system clipboard (`CF_UNICODETEXT`).
  - Seamlessly copy/paste between WinEdit and other Windows applications (VS Code, Notepad, browser, etc.).
- **Real-Time Syntax Highlighting**:
  - Automatically detects language by file extension:
    - **C / C++** (`.cpp`, `.hpp`, `.c`, `.h`, `.cc`, etc.): Keywords (Cyan), Types (Green), Preprocessor `#include` (Magenta), Strings (Yellow), Numbers (Dark Cyan), Single-line & Multi-line Comments `/* ... */` (Dark Gray).
    - **Python** (`.py`): Keywords, Built-in types, Triple-quoted multiline strings `"""`, Decorators, Comments `#`.
    - **JSON** (`.json`): Object keys, Strings, Numbers, Booleans / Null.
    - **Markdown** (`.md`): Headers `#`, list bullets `-`, inline code blocks.
    - **Lua** (`.lua`): Keywords (`local`, `function`, `end`, etc.), Built-in libraries (`print`, `table`, `string`, etc.), Strings (`"..."`, `'...'`, and `[[ multiline ]]`), Numbers, Comments (`--` and `--[[ multiline ]]`).
  - Multi-line state tracker accurately preserves comments and docstrings across scrolling.
  - Detected language indicator in the bottom status bar (`[C++]`, `[Python]`, `[Lua]`, `[JSON]`, etc.).
- **Smart Auto-Closing Pairs & Auto-Formatting**:
  - **Auto-Pairs**: Automatically inserts matching closing pairs for `()`, `[]`, `{}`, `""`, `''`, and `` `` ``.
  - **Smart Skip-Over (Overtype)**: Typing a closing character when cursor is before it simply steps over without duplicate insertions.
  - **Pair Deletion**: Pressing `Backspace` between an empty pair (e.g. `(|)`, `"|"`) cleanly deletes both characters.
  - **Expanded Bracket Formatting**: Pressing `Enter` between `{}` automatically creates a middle line indented 4 spaces and places the closing brace on a new line.
  - **Selection Wrapping**: Typing any opening pair or quote with text selected surrounds the selection (e.g. `(selection)` or `"selection"`).
  - **Apostrophe Intelligence**: Single quote `'` is not auto-closed when preceded by an alphanumeric character (e.g. in `don't`, `it's`).
- **Text Editing & Formatting**:
  - Auto-indentation (inherits leading indentation on `Enter`).
  - Tabulation: `Tab` inserts 4 spaces; `Shift + Tab` un-indents lines.
  - Line duplicate & deletion: `Ctrl + D` to delete the current line.
- **Multi-Level Undo & Redo**:
  - `Ctrl + Z` to Undo, `Ctrl + Y` to Redo.
- **Text Selection (Visual Mode)**:
  - Hold `Shift + Arrow keys` (or `Shift + Home/End/PageUp/PageDown`) to select text.
  - Cut, copy, or replace selected ranges.
- **Search & Navigation**:
  - `Ctrl + F`: Interactive search prompt with highlighted occurrences across lines.
  - `F3` / `Shift + F3`: Jump to next / previous match.
  - `Ctrl + G`: Go to line dialog.
  - `Ctrl + Left` / `Ctrl + Right`: Word-by-word jumping.
  - `Ctrl + Home` / `Ctrl + End`: Jump to document start / end.
- **File Management & Unicode Support**:
  - Full UTF-8 support (with automatic UTF-8 BOM detection).
  - Preserves standard Windows CRLF line endings on save.
  - Unsaved changes confirmation on exit (`Ctrl + Q`).
  - Save (`Ctrl + S`) and Save As (`Ctrl + W`).

---

## ⌨️ Keyboard Shortcuts Reference

| Shortcut | Description |
| :--- | :--- |
| **Windows 9x Menus & Themes** | |
| `Alt + F / E / S / V / H` | Open File / Edit / Search / View / Help menu |
| `F10` | Focus Windows 9x menu bar |
| `Alt + T` | Cycle Themes (Win98 Dark -> Win95 Notepad -> MS-DOS Edit) |
| `Alt + 1 / 2 / 3` | Directly switch to Win98 Dark / Win95 Notepad / MS-DOS Edit |
| `Alt + F4` / `Ctrl + Q` | Exit WinEdit 98 |
| **File Operations** | |
| `Ctrl + N` | New file |
| `Ctrl + O` | Open file |
| `Ctrl + S` | Save file (prompts for name if untitled) |
| `Ctrl + W` | Save As... |
| `Ctrl + Q` | Quit editor (warns if unsaved changes exist) |
| **Navigation** | |
| `Arrow Keys` | Move cursor Up, Down, Left, Right |
| `Ctrl + Left / Right` | Move cursor one word left / right |
| `Home / End` | Jump to first non-space character / end of line |
| `PageUp / PageDown` | Scroll one screen page up / down |
| `Ctrl + Home / End` | Jump to top / bottom of document |
| `Ctrl + G` | Go to line number |
| **Editing** | |
| `Enter` | New line with auto-indentation |
| `Backspace / Delete` | Delete character or merge lines |
| `Tab` | Insert 4 spaces |
| `Shift + Tab` | Remove indentation (un-indent) |
| `Ctrl + D` | Delete current line |
| `Ctrl + Z` | Undo |
| `Ctrl + Y` | Redo |
| **Selection & Clipboard** | |
| `Shift + Movement` | Select text range |
| `Ctrl + C` | Copy selection (or current line if nothing selected) |
| `Ctrl + X` | Cut selection (or current line) |
| `Ctrl + V` | Paste from Windows Clipboard |
| **Search & Help** | |
| `Ctrl + F` | Find in file |
| `F3` / `Shift + F3` | Repeat search forward / backward |
| `Ctrl + H` or `F1` | Show interactive Help overlay |
| `Esc` | Cancel search / dismiss prompts / clear selection |

---

## 🏗️ Project Structure

```
win_tui_editor/
├── include/
│   ├── Common.hpp        # Colors, event structures, and Win32 headers
│   ├── Console.hpp       # Alternate screen buffer, double buffering, input reader
│   ├── Clipboard.hpp     # Win32 Clipboard integration (CF_UNICODETEXT)
│   ├── Document.hpp      # Document text buffer, UTF-8 file I/O, search & edits
│   ├── Syntax.hpp        # Syntax highlighter engine & multiline state machine
│   └── Editor.hpp        # UI rendering, viewport scrolling, event dispatch
├── src/
│   ├── Console.cpp
│   ├── Clipboard.cpp
│   ├── Document.cpp
│   ├── Syntax.cpp
│   ├── Editor.cpp
│   └── main.cpp          # CLI entry point and argument parsing
├── tests/
│   └── test_editor.cpp   # Automated test suite
├── build.bat             # MSVC automated build script
└── README.md
```

---

## 🚀 Building & Running

### Prerequisites
- Windows 10 / 11 / Server
- Visual Studio (2019 or 2022) with "Desktop development with C++" or MSVC Build Tools.

### Build with `build.bat`
Run `build.bat` from any Command Prompt or PowerShell:
```cmd
cd C:\Users\skv\win_tui_editor
build.bat
```
The script will locate your Visual Studio MSVC environment and compile `winedit.exe`.

### Manual Compilation
From a Visual Studio Developer Command Prompt:
```cmd
cl /nologo /EHsc /std:c++17 /W4 /O2 /Iinclude src\*.cpp /Fe:winedit.exe user32.lib shell32.lib
```

---

## 🖥️ Usage

Open a new empty file:
```cmd
winedit.exe
```

Open or edit an existing file:
```cmd
winedit.exe myfile.txt
winedit.exe C:\path\to\code.cpp
```

Show version and build info:
```cmd
winedit.exe --version
```

Show command-line help:
```cmd
winedit.exe --help
```
