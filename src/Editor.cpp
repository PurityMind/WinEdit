#include "Editor.hpp"
#include "Clipboard.hpp"
#include <algorithm>
#include <cwctype>
#include <iomanip>
#include <sstream>

Editor::Editor() = default;

bool Editor::init(const std::wstring& initialFile) {
    if (!m_console.init()) {
        return false;
    }

    initMenus();

    if (!initialFile.empty()) {
        std::wstring err;
        DWORD attrs = GetFileAttributesW(initialFile.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            if (m_doc.load(initialFile, err)) {
                setStatus(L"Loaded: " + m_doc.getFileName());
            } else {
                setStatus(L"Error: " + err, Color::Red | Color::BgBlack);
            }
        } else {
            m_doc.setFilePath(initialFile);
            setStatus(L"New file: " + m_doc.getFileName());
        }
    } else {
        setStatus(L"WinEdit 98 ready. Press Alt+F for menu, Alt+T to cycle theme.");
    }

    updateSyntax();
    return true;
}

void Editor::initMenus() {
    m_menus.clear();

    // 1. File Menu
    Menu fileMenu;
    fileMenu.title = L"File";
    fileMenu.hotkeyChar = L'F';
    fileMenu.items = {
        { L"New",               L"Ctrl+N", L'N', ACTION_NEW, false },
        { L"Open...",           L"Ctrl+O", L'O', ACTION_OPEN, false },
        { L"Save",              L"Ctrl+S", L'S', ACTION_SAVE, false },
        { L"Save As...",        L"Ctrl+W", L'A', ACTION_SAVE_AS, false },
        { L"",                  L"",       0,    ACTION_NONE, true },
        { L"Exit",              L"Ctrl+Q", L'X', ACTION_EXIT, false }
    };
    m_menus.push_back(fileMenu);

    // 2. Edit Menu
    Menu editMenu;
    editMenu.title = L"Edit";
    editMenu.hotkeyChar = L'E';
    editMenu.items = {
        { L"Undo",              L"Ctrl+Z", L'U', ACTION_UNDO, false },
        { L"Redo",              L"Ctrl+Y", L'R', ACTION_REDO, false },
        { L"",                  L"",       0,    ACTION_NONE, true },
        { L"Cut",               L"Ctrl+X", L'T', ACTION_CUT, false },
        { L"Copy",              L"Ctrl+C", L'C', ACTION_COPY, false },
        { L"Paste",             L"Ctrl+V", L'P', ACTION_PASTE, false },
        { L"Delete Line",       L"Ctrl+D", L'D', ACTION_DELETE_LINE, false },
        { L"",                  L"",       0,    ACTION_NONE, true },
        { L"Select All",        L"Ctrl+A", L'A', ACTION_SELECT_ALL, false }
    };
    m_menus.push_back(editMenu);

    // 3. Search Menu
    Menu searchMenu;
    searchMenu.title = L"Search";
    searchMenu.hotkeyChar = L'S';
    searchMenu.items = {
        { L"Find...",           L"Ctrl+F",    L'F', ACTION_FIND, false },
        { L"Find Next",         L"F3",        L'N', ACTION_FIND_NEXT, false },
        { L"Find Previous",     L"Shift+F3",  L'P', ACTION_FIND_PREV, false },
        { L"",                  L"",          0,    ACTION_NONE, true },
        { L"Go To Line...",     L"Ctrl+G",    L'G', ACTION_GOTO_LINE, false }
    };
    m_menus.push_back(searchMenu);

    // 4. View Menu
    Menu viewMenu;
    viewMenu.title = L"View";
    viewMenu.hotkeyChar = L'V';
    viewMenu.items = {
        { L"Windows 98 Dark",     L"Alt+1", L'D', ACTION_THEME_WIN98, false },
        { L"Windows 95 Notepad",  L"Alt+2", L'N', ACTION_THEME_NOTEPAD, false },
        { L"MS-DOS 7.0 Edit",     L"Alt+3", L'M', ACTION_THEME_DOSEDIT, false },
        { L"Cycle Theme",         L"Alt+T", L'T', ACTION_NONE, false },
        { L"",                    L"",      0,    ACTION_NONE, true },
        { L"Toggle Line Numbers", L"",      L'L', ACTION_TOGGLE_LINE_NUMS, false },
        { L"Toggle Auto-Pairs",   L"",      L'P', ACTION_TOGGLE_AUTOPAIRS, false }
    };
    m_menus.push_back(viewMenu);

    // 5. Help Menu
    Menu helpMenu;
    helpMenu.title = L"Help";
    helpMenu.hotkeyChar = L'H';
    helpMenu.items = {
        { L"Help Topics",         L"F1",     L'H', ACTION_HELP_TOPICS, false },
        { L"Keyboard Shortcuts",  L"Ctrl+H", L'K', ACTION_SHORTCUTS, false },
        { L"",                    L"",       0,    ACTION_NONE, true },
        { L"About WinEdit 98...", L"",       L'A', ACTION_ABOUT, false }
    };
    m_menus.push_back(helpMenu);

    // Calculate layout coordinates on the menu bar
    int currentX = 1;
    for (auto& menu : m_menus) {
        menu.startX = currentX;
        menu.width = static_cast<int>(menu.title.size()) + 2; // " Title "
        currentX += menu.width + 1; // 1 space between menu headers
    }
}

void Editor::setTheme(ThemeType theme) {
    m_currentTheme = theme;
    switch (theme) {
        case ThemeType::Win98Dark:
            setStatus(L"Theme switched to: Windows 98 Dark");
            break;
        case ThemeType::Win95Notepad:
            setStatus(L"Theme switched to: Windows 95 Notepad");
            break;
        case ThemeType::DosEditBlue:
            setStatus(L"Theme switched to: MS-DOS 7.0 Edit");
            break;
    }
}

void Editor::cycleTheme() {
    if (m_currentTheme == ThemeType::Win98Dark) {
        setTheme(ThemeType::Win95Notepad);
    } else if (m_currentTheme == ThemeType::Win95Notepad) {
        setTheme(ThemeType::DosEditBlue);
    } else {
        setTheme(ThemeType::Win98Dark);
    }
}

int Editor::getMarginWidth() const {
    if (!m_showLineNumbers) {
        return 1; // Minimal 1 column margin
    }
    int digits = static_cast<int>(std::to_string(m_doc.lineCount()).length());
    digits = (std::max)(3, digits);
    return digits + 3; // e.g. " 10 │ "
}

int Editor::getEditorHeight() const {
    // Row 0: Title bar
    // Row 1: Menu bar
    // Rows 2 .. Height-2: Document canvas
    // Row Height-1: Status bar
    return (std::max)(1, m_console.getHeight() - 3);
}

int Editor::getEditorWidth() const {
    // Width minus left margin and right scrollbar column
    return (std::max)(1, m_console.getWidth() - getMarginWidth() - 1);
}

int Editor::charToVisualCol(int row, int charCol) const {
    const std::wstring& line = m_doc.getLine(row);
    int vCol = 0;
    int limit = (std::min)(charCol, static_cast<int>(line.size()));
    for (int i = 0; i < limit; ++i) {
        if (line[i] == L'\t') {
            vCol += 4 - (vCol % 4);
        } else {
            vCol += 1;
        }
    }
    return vCol;
}

int Editor::visualToCharCol(int row, int visualCol) const {
    const std::wstring& line = m_doc.getLine(row);
    int vCol = 0;
    for (size_t i = 0; i < line.size(); ++i) {
        int nextVCol = vCol + (line[i] == L'\t' ? (4 - (vCol % 4)) : 1);
        if (nextVCol > visualCol) {
            return static_cast<int>(i);
        }
        vCol = nextVCol;
    }
    return static_cast<int>(line.size());
}

void Editor::adjustScroll() {
    int vCol = charToVisualCol(m_cursorRow, m_cursorCol);
    int eHeight = getEditorHeight();
    int eWidth = getEditorWidth();

    // Vertical scroll
    if (m_cursorRow < m_scrollRow) {
        m_scrollRow = m_cursorRow;
    }
    if (m_cursorRow >= m_scrollRow + eHeight) {
        m_scrollRow = m_cursorRow - eHeight + 1;
    }

    // Horizontal scroll
    if (vCol < m_scrollCol) {
        m_scrollCol = vCol;
    }
    if (vCol >= m_scrollCol + eWidth) {
        m_scrollCol = vCol - eWidth + 1;
    }
}

void Editor::updateSyntax() {
    m_language = SyntaxHighlighter::detectLanguage(m_doc.getFileName());
    m_syntax.updateStates(m_doc.getLines(), m_language);
}

void Editor::recordUndo(bool isTyping) {
    if (isTyping && m_isTypingGroup && !m_undoStack.empty()) {
        return;
    }
    m_isTypingGroup = isTyping;

    UndoSnapshot snap;
    snap.lines = m_doc.getLines();
    snap.cursorRow = m_cursorRow;
    snap.cursorCol = m_cursorCol;
    snap.modified = m_doc.isModified();

    m_undoStack.push_back(snap);
    if (m_undoStack.size() > 100) {
        m_undoStack.erase(m_undoStack.begin());
    }

    m_redoStack.clear();
}

void Editor::performUndo() {
    if (m_undoStack.empty()) {
        setStatus(L"Already at oldest change");
        return;
    }

    m_isTypingGroup = false;

    UndoSnapshot current;
    current.lines = m_doc.getLines();
    current.cursorRow = m_cursorRow;
    current.cursorCol = m_cursorCol;
    current.modified = m_doc.isModified();
    m_redoStack.push_back(current);

    UndoSnapshot prev = m_undoStack.back();
    m_undoStack.pop_back();

    m_doc.setLines(prev.lines);
    m_cursorRow = (std::min)(prev.cursorRow, static_cast<int>(m_doc.lineCount()) - 1);
    m_cursorCol = (std::min)(prev.cursorCol, static_cast<int>(m_doc.getLine(m_cursorRow).size()));
    m_preferredCol = m_cursorCol;
    m_doc.setModified(prev.modified);
    m_hasSelection = false;

    setStatus(L"Undo");
}

void Editor::performRedo() {
    if (m_redoStack.empty()) {
        setStatus(L"Already at newest change");
        return;
    }

    m_isTypingGroup = false;

    UndoSnapshot current;
    current.lines = m_doc.getLines();
    current.cursorRow = m_cursorRow;
    current.cursorCol = m_cursorCol;
    current.modified = m_doc.isModified();
    m_undoStack.push_back(current);

    UndoSnapshot next = m_redoStack.back();
    m_redoStack.pop_back();

    m_doc.setLines(next.lines);
    m_cursorRow = (std::min)(next.cursorRow, static_cast<int>(m_doc.lineCount()) - 1);
    m_cursorCol = (std::min)(next.cursorCol, static_cast<int>(m_doc.getLine(m_cursorRow).size()));
    m_preferredCol = m_cursorCol;
    m_doc.setModified(next.modified);
    m_hasSelection = false;

    setStatus(L"Redo");
}

void Editor::getSelectionBounds(int& startRow, int& startCol, int& endRow, int& endCol) const {
    if (m_cursorRow < m_selAnchorRow ||
        (m_cursorRow == m_selAnchorRow && m_cursorCol < m_selAnchorCol)) {
        startRow = m_cursorRow;
        startCol = m_cursorCol;
        endRow = m_selAnchorRow;
        endCol = m_selAnchorCol;
    } else {
        startRow = m_selAnchorRow;
        startCol = m_selAnchorCol;
        endRow = m_cursorRow;
        endCol = m_cursorCol;
    }
}

bool Editor::isCellSelected(int row, int col) const {
    if (!m_hasSelection) return false;
    int sR, sC, eR, eC;
    getSelectionBounds(sR, sC, eR, eC);

    if (row < sR || row > eR) return false;
    if (sR == eR) {
        return col >= sC && col < eC;
    }
    if (row == sR) return col >= sC;
    if (row == eR) return col < eC;
    return true;
}

void Editor::deleteSelection() {
    if (!m_hasSelection) return;
    recordUndo();
    int sR, sC, eR, eC;
    getSelectionBounds(sR, sC, eR, eC);
    m_doc.deleteRange(sR, sC, eR, eC);
    m_cursorRow = sR;
    m_cursorCol = sC;
    m_preferredCol = sC;
    m_hasSelection = false;
}

void Editor::selectAll() {
    m_hasSelection = true;
    m_selAnchorRow = 0;
    m_selAnchorCol = 0;
    m_cursorRow = static_cast<int>(m_doc.lineCount()) - 1;
    m_cursorCol = static_cast<int>(m_doc.getLine(m_cursorRow).size());
    m_preferredCol = m_cursorCol;
    setStatus(L"Selected all text");
}

void Editor::copyToClipboard() {
    if (m_hasSelection) {
        int sR, sC, eR, eC;
        getSelectionBounds(sR, sC, eR, eC);
        std::wstring text = m_doc.getSelectedText(sR, sC, eR, eC);
        if (Clipboard::SetText(text)) {
            setStatus(L"Copied selection to clipboard");
        }
    } else {
        std::wstring text = m_doc.getLine(m_cursorRow) + L"\n";
        if (Clipboard::SetText(text)) {
            setStatus(L"Copied current line to clipboard");
        }
    }
}

void Editor::cutToClipboard() {
    if (m_hasSelection) {
        copyToClipboard();
        deleteSelection();
        setStatus(L"Cut selection to clipboard");
    } else {
        copyToClipboard();
        recordUndo();
        m_doc.deleteLine(m_cursorRow, m_cursorCol);
        m_preferredCol = m_cursorCol;
        setStatus(L"Cut current line to clipboard");
    }
}

void Editor::pasteFromClipboard() {
    std::wstring text;
    if (!Clipboard::GetText(text) || text.empty()) {
        setStatus(L"Clipboard is empty");
        return;
    }

    if (m_hasSelection) {
        deleteSelection();
    }

    recordUndo();
    m_doc.insertText(m_cursorRow, m_cursorCol, text);
    m_preferredCol = m_cursorCol;
    setStatus(L"Pasted from clipboard");
}

void Editor::moveCursor(int dRow, int dCol, bool keepSelection) {
    if (keepSelection) {
        if (!m_hasSelection) {
            m_hasSelection = true;
            m_selAnchorRow = m_cursorRow;
            m_selAnchorCol = m_cursorCol;
        }
    } else {
        m_hasSelection = false;
    }

    int totalLines = static_cast<int>(m_doc.lineCount());

    if (dRow != 0) {
        m_cursorRow = (std::max)(0, (std::min)(totalLines - 1, m_cursorRow + dRow));
        const std::wstring& line = m_doc.getLine(m_cursorRow);
        m_cursorCol = (std::min)(m_preferredCol, static_cast<int>(line.size()));
    }

    if (dCol != 0) {
        m_cursorCol += dCol;
        if (m_cursorCol < 0) {
            if (m_cursorRow > 0) {
                m_cursorRow--;
                m_cursorCol = static_cast<int>(m_doc.getLine(m_cursorRow).size());
            } else {
                m_cursorCol = 0;
            }
        } else if (m_cursorCol > static_cast<int>(m_doc.getLine(m_cursorRow).size())) {
            if (m_cursorRow + 1 < totalLines) {
                m_cursorRow++;
                m_cursorCol = 0;
            } else {
                m_cursorCol = static_cast<int>(m_doc.getLine(m_cursorRow).size());
            }
        }
        m_preferredCol = m_cursorCol;
    }
}

void Editor::moveWordLeft(bool keepSelection) {
    if (keepSelection) {
        if (!m_hasSelection) {
            m_hasSelection = true;
            m_selAnchorRow = m_cursorRow;
            m_selAnchorCol = m_cursorCol;
        }
    } else {
        m_hasSelection = false;
    }

    const std::wstring& line = m_doc.getLine(m_cursorRow);
    if (m_cursorCol == 0) {
        if (m_cursorRow > 0) {
            m_cursorRow--;
            m_cursorCol = static_cast<int>(m_doc.getLine(m_cursorRow).size());
            m_preferredCol = m_cursorCol;
        }
        return;
    }

    int col = m_cursorCol;
    while (col > 0 && iswspace(line[col - 1])) {
        col--;
    }
    while (col > 0 && !iswspace(line[col - 1])) {
        col--;
    }

    m_cursorCol = col;
    m_preferredCol = m_cursorCol;
}

void Editor::moveWordRight(bool keepSelection) {
    if (keepSelection) {
        if (!m_hasSelection) {
            m_hasSelection = true;
            m_selAnchorRow = m_cursorRow;
            m_selAnchorCol = m_cursorCol;
        }
    } else {
        m_hasSelection = false;
    }

    const std::wstring& line = m_doc.getLine(m_cursorRow);
    int lineLen = static_cast<int>(line.size());
    if (m_cursorCol >= lineLen) {
        if (m_cursorRow + 1 < static_cast<int>(m_doc.lineCount())) {
            m_cursorRow++;
            m_cursorCol = 0;
            m_preferredCol = 0;
        }
        return;
    }

    int col = m_cursorCol;
    while (col < lineLen && !iswspace(line[col])) {
        col++;
    }
    while (col < lineLen && iswspace(line[col])) {
        col++;
    }

    m_cursorCol = col;
    m_preferredCol = m_cursorCol;
}

void Editor::moveToLineStart(bool keepSelection) {
    if (keepSelection) {
        if (!m_hasSelection) {
            m_hasSelection = true;
            m_selAnchorRow = m_cursorRow;
            m_selAnchorCol = m_cursorCol;
        }
    } else {
        m_hasSelection = false;
    }

    const std::wstring& line = m_doc.getLine(m_cursorRow);
    int firstNonSpace = 0;
    while (firstNonSpace < static_cast<int>(line.size()) && iswspace(line[firstNonSpace])) {
        firstNonSpace++;
    }

    if (m_cursorCol == firstNonSpace) {
        m_cursorCol = 0;
    } else {
        m_cursorCol = firstNonSpace;
    }
    m_preferredCol = m_cursorCol;
}

void Editor::moveToLineEnd(bool keepSelection) {
    if (keepSelection) {
        if (!m_hasSelection) {
            m_hasSelection = true;
            m_selAnchorRow = m_cursorRow;
            m_selAnchorCol = m_cursorCol;
        }
    } else {
        m_hasSelection = false;
    }

    m_cursorCol = static_cast<int>(m_doc.getLine(m_cursorRow).size());
    m_preferredCol = m_cursorCol;
}

void Editor::pageUp(bool keepSelection) {
    moveCursor(-getEditorHeight(), 0, keepSelection);
}

void Editor::pageDown(bool keepSelection) {
    moveCursor(getEditorHeight(), 0, keepSelection);
}

void Editor::moveToDocStart(bool keepSelection) {
    if (keepSelection) {
        if (!m_hasSelection) {
            m_hasSelection = true;
            m_selAnchorRow = m_cursorRow;
            m_selAnchorCol = m_cursorCol;
        }
    } else {
        m_hasSelection = false;
    }

    m_cursorRow = 0;
    m_cursorCol = 0;
    m_preferredCol = 0;
}

void Editor::moveToDocEnd(bool keepSelection) {
    if (keepSelection) {
        if (!m_hasSelection) {
            m_hasSelection = true;
            m_selAnchorRow = m_cursorRow;
            m_selAnchorCol = m_cursorCol;
        }
    } else {
        m_hasSelection = false;
    }

    m_cursorRow = static_cast<int>(m_doc.lineCount()) - 1;
    m_cursorCol = static_cast<int>(m_doc.getLine(m_cursorRow).size());
    m_preferredCol = m_cursorCol;
}

void Editor::setStatus(const std::wstring& msg, WORD color) {
    m_statusMessage = msg;
    m_statusColor = color;
    m_statusTimestamp = GetTickCount();
}

void Editor::cmdSave() {
    if (m_doc.isNewFile() || m_doc.getFilePath().empty()) {
        cmdSaveAs();
        return;
    }

    std::wstring err;
    if (m_doc.save(m_doc.getFilePath(), err)) {
        setStatus(L"Saved " + std::to_wstring(m_doc.lineCount()) + L" lines to " + m_doc.getFileName());
        m_quitConfirmPending = false;
    } else {
        setStatus(L"Save failed: " + err, Color::Red | Color::BgBlack);
    }
}

void Editor::cmdSaveAs() {
    m_mode = EditorMode::SavePrompt;
    m_promptLabel = L"File name:";
    m_promptInput = m_doc.getFilePath().empty() ? L"Untitled.txt" : m_doc.getFilePath();
    m_promptCursor = static_cast<int>(m_promptInput.size());
    m_dialogFocus = 0;
}

void Editor::cmdFind() {
    m_mode = EditorMode::Search;
    m_promptLabel = L"Find what:";
    m_promptInput = m_lastSearchQuery;
    m_promptCursor = static_cast<int>(m_promptInput.size());
    m_dialogFocus = 0;
}

void Editor::cmdFindNext(bool forward) {
    if (m_lastSearchQuery.empty()) {
        cmdFind();
        return;
    }

    int foundRow = -1;
    int foundCol = -1;
    bool found = false;

    if (forward) {
        found = m_doc.findNext(m_lastSearchQuery, m_cursorRow, m_cursorCol + 1, foundRow, foundCol);
    } else {
        found = m_doc.findPrev(m_lastSearchQuery, m_cursorRow, m_cursorCol, foundRow, foundCol);
    }

    if (found) {
        m_cursorRow = foundRow;
        m_cursorCol = foundCol;
        m_preferredCol = m_cursorCol;
        m_hasSelection = false;
        setStatus(L"Match found at line " + std::to_wstring(foundRow + 1));
    } else {
        setStatus(L"Pattern not found: " + m_lastSearchQuery);
    }
}

void Editor::cmdGoToLine() {
    m_mode = EditorMode::GoToLinePrompt;
    m_promptLabel = L"Line number (1 - " + std::to_wstring(m_doc.lineCount()) + L"):";
    m_promptInput = std::to_wstring(m_cursorRow + 1);
    m_promptCursor = static_cast<int>(m_promptInput.size());
    m_dialogFocus = 0;
}

void Editor::cmdExit() {
    if (!m_doc.isModified() || m_quitConfirmPending) {
        m_running = false;
    } else {
        m_quitConfirmPending = true;
        setStatus(L"Unsaved changes! Press Exit again or Ctrl+Q to force quit.", Color::Red | Color::BgBlack);
    }
}

void Editor::executeMenuAction(int actionId) {
    m_activeMenu = -1; // Close dropdown menu

    switch (actionId) {
        case ACTION_NEW: {
            if (m_doc.isModified() && !m_quitConfirmPending) {
                m_quitConfirmPending = true;
                setStatus(L"Unsaved changes! Select New again to discard changes.", Color::Red | Color::BgBlack);
                return;
            }
            m_doc.setLines({ L"" });
            m_doc.setFilePath(L"");
            m_cursorRow = 0;
            m_cursorCol = 0;
            m_preferredCol = 0;
            m_scrollRow = 0;
            m_scrollCol = 0;
            m_hasSelection = false;
            m_undoStack.clear();
            m_redoStack.clear();
            updateSyntax();
            setStatus(L"New file created");
            break;
        }
        case ACTION_OPEN:
            cmdSaveAs(); // Or prompt for open
            break;
        case ACTION_SAVE:
            cmdSave();
            break;
        case ACTION_SAVE_AS:
            cmdSaveAs();
            break;
        case ACTION_EXIT:
            cmdExit();
            break;
        case ACTION_UNDO:
            performUndo();
            break;
        case ACTION_REDO:
            performRedo();
            break;
        case ACTION_CUT:
            cutToClipboard();
            break;
        case ACTION_COPY:
            copyToClipboard();
            break;
        case ACTION_PASTE:
            pasteFromClipboard();
            break;
        case ACTION_DELETE_LINE: {
            recordUndo();
            m_doc.deleteLine(m_cursorRow, m_cursorCol);
            m_preferredCol = m_cursorCol;
            setStatus(L"Deleted line");
            break;
        }
        case ACTION_SELECT_ALL:
            selectAll();
            break;
        case ACTION_FIND:
            cmdFind();
            break;
        case ACTION_FIND_NEXT:
            cmdFindNext(true);
            break;
        case ACTION_FIND_PREV:
            cmdFindNext(false);
            break;
        case ACTION_GOTO_LINE:
            cmdGoToLine();
            break;
        case ACTION_THEME_WIN98:
            setTheme(ThemeType::Win98Dark);
            break;
        case ACTION_THEME_NOTEPAD:
            setTheme(ThemeType::Win95Notepad);
            break;
        case ACTION_THEME_DOSEDIT:
            setTheme(ThemeType::DosEditBlue);
            break;
        case ACTION_TOGGLE_LINE_NUMS:
            m_showLineNumbers = !m_showLineNumbers;
            setStatus(m_showLineNumbers ? L"Line numbers enabled" : L"Line numbers hidden");
            break;
        case ACTION_TOGGLE_AUTOPAIRS:
            m_autoPairsEnabled = !m_autoPairsEnabled;
            setStatus(m_autoPairsEnabled ? L"Auto-close pairs enabled" : L"Auto-close pairs disabled");
            break;
        case ACTION_HELP_TOPICS:
        case ACTION_SHORTCUTS:
            m_mode = EditorMode::HelpModal;
            break;
        case ACTION_ABOUT:
            m_mode = EditorMode::AboutModal;
            break;
        default:
            break;
    }
}

void Editor::handleMenuKey(const KeyEvent& key) {
    if (m_activeMenu < 0 || m_activeMenu >= static_cast<int>(m_menus.size())) {
        m_activeMenu = -1;
        return;
    }

    Menu& currentMenu = m_menus[m_activeMenu];

    if (key.vkCode == VK_ESCAPE) {
        m_activeMenu = -1;
        return;
    }

    if (key.vkCode == VK_LEFT) {
        m_activeMenu = (m_activeMenu - 1 + static_cast<int>(m_menus.size())) % static_cast<int>(m_menus.size());
        m_selectedMenuItem = 0;
        // Skip separator if first item is separator
        if (m_menus[m_activeMenu].items[m_selectedMenuItem].isSeparator) {
            m_selectedMenuItem++;
        }
        return;
    }

    if (key.vkCode == VK_RIGHT) {
        m_activeMenu = (m_activeMenu + 1) % static_cast<int>(m_menus.size());
        m_selectedMenuItem = 0;
        if (m_menus[m_activeMenu].items[m_selectedMenuItem].isSeparator) {
            m_selectedMenuItem++;
        }
        return;
    }

    if (key.vkCode == VK_UP) {
        int count = static_cast<int>(currentMenu.items.size());
        do {
            m_selectedMenuItem = (m_selectedMenuItem - 1 + count) % count;
        } while (currentMenu.items[m_selectedMenuItem].isSeparator);
        return;
    }

    if (key.vkCode == VK_DOWN) {
        int count = static_cast<int>(currentMenu.items.size());
        do {
            m_selectedMenuItem = (m_selectedMenuItem + 1) % count;
        } while (currentMenu.items[m_selectedMenuItem].isSeparator);
        return;
    }

    if (key.vkCode == VK_RETURN) {
        if (m_selectedMenuItem >= 0 && m_selectedMenuItem < static_cast<int>(currentMenu.items.size())) {
            const auto& item = currentMenu.items[m_selectedMenuItem];
            if (!item.isSeparator) {
                if (item.actionId == ACTION_NONE && item.label == L"Cycle Theme") {
                    cycleTheme();
                    m_activeMenu = -1;
                } else {
                    executeMenuAction(item.actionId);
                }
            }
        }
        return;
    }

    // Letter accelerator for menu item
    if (key.ch >= 32) {
        wchar_t upperCh = towupper(key.ch);
        for (size_t i = 0; i < currentMenu.items.size(); ++i) {
            const auto& item = currentMenu.items[i];
            if (!item.isSeparator && towupper(item.hotkeyChar) == upperCh) {
                if (item.actionId == ACTION_NONE && item.label == L"Cycle Theme") {
                    cycleTheme();
                    m_activeMenu = -1;
                } else {
                    executeMenuAction(item.actionId);
                }
                return;
            }
        }
    }
}

void Editor::handleDialogKey(const KeyEvent& key) {
    if (key.vkCode == VK_ESCAPE) {
        m_mode = EditorMode::Normal;
        setStatus(L"Cancelled");
        return;
    }

    if (key.vkCode == VK_TAB) {
        m_dialogFocus = (m_dialogFocus + 1) % 3; // 0: Input, 1: Action, 2: Cancel
        return;
    }

    if (key.vkCode == VK_RETURN || (m_dialogFocus > 0 && key.vkCode == VK_SPACE)) {
        if (m_dialogFocus == 2) {
            // Cancel button
            m_mode = EditorMode::Normal;
            setStatus(L"Cancelled");
            return;
        }

        // Action button or Enter pressed in input box
        if (m_mode == EditorMode::SavePrompt) {
            if (!m_promptInput.empty()) {
                m_doc.setFilePath(m_promptInput);
                std::wstring err;
                if (m_doc.save(m_promptInput, err)) {
                    setStatus(L"Saved " + std::to_wstring(m_doc.lineCount()) + L" lines to " + m_doc.getFileName());
                } else {
                    setStatus(L"Save failed: " + err, Color::Red | Color::BgBlack);
                }
            } else {
                setStatus(L"Save cancelled (empty path)");
            }
        } else if (m_mode == EditorMode::Search) {
            if (!m_promptInput.empty()) {
                m_lastSearchQuery = m_promptInput;
                cmdFindNext(true);
            }
        } else if (m_mode == EditorMode::GoToLinePrompt) {
            if (!m_promptInput.empty()) {
                int line = _wtoi(m_promptInput.c_str());
                int total = static_cast<int>(m_doc.lineCount());
                line = (std::max)(1, (std::min)(total, line));
                m_cursorRow = line - 1;
                m_cursorCol = 0;
                m_preferredCol = 0;
                m_scrollRow = (std::max)(0, m_cursorRow - getEditorHeight() / 2);
                setStatus(L"Jumped to line " + std::to_wstring(line));
            }
        }
        m_mode = EditorMode::Normal;
        return;
    }

    // Input editing if focus is on input field
    if (m_dialogFocus == 0) {
        if (key.vkCode == VK_BACK) {
            if (m_promptCursor > 0) {
                m_promptInput.erase(m_promptCursor - 1, 1);
                m_promptCursor--;
            }
            return;
        }

        if (key.vkCode == VK_DELETE) {
            if (m_promptCursor < static_cast<int>(m_promptInput.size())) {
                m_promptInput.erase(m_promptCursor, 1);
            }
            return;
        }

        if (key.vkCode == VK_LEFT) {
            if (m_promptCursor > 0) m_promptCursor--;
            return;
        }

        if (key.vkCode == VK_RIGHT) {
            if (m_promptCursor < static_cast<int>(m_promptInput.size())) m_promptCursor++;
            return;
        }

        if (key.vkCode == VK_HOME) {
            m_promptCursor = 0;
            return;
        }

        if (key.vkCode == VK_END) {
            m_promptCursor = static_cast<int>(m_promptInput.size());
            return;
        }

        if (!key.ctrl && !key.alt && key.ch >= 32) {
            m_promptInput.insert(m_promptCursor, 1, key.ch);
            m_promptCursor++;
        }
    } else {
        // Left / Right switches between buttons
        if (key.vkCode == VK_LEFT || key.vkCode == VK_RIGHT) {
            m_dialogFocus = (m_dialogFocus == 1) ? 2 : 1;
        }
    }
}

void Editor::handleHelpKey(const KeyEvent& key) {
    (void)key;
    m_mode = EditorMode::Normal;
}

void Editor::handleMouseEvent(const MouseEvent& mouse) {
    int w = m_console.getWidth();
    int h = m_console.getHeight();

    // Wheel scrolling always active
    if (mouse.wheelDelta != 0) {
        m_scrollRow -= mouse.wheelDelta * 3;
        m_scrollRow = (std::max)(0, (std::min)(static_cast<int>(m_doc.lineCount()) - 1, m_scrollRow));
        return;
    }

    if (!mouse.leftButton) {
        return;
    }

    // 1. Title bar interactions (Row 0)
    if (mouse.y == 0) {
        // Right-side close button [ X ]
        if (mouse.x >= w - 7 && mouse.x < w) {
            cmdExit();
            return;
        }
    }

    // 2. Menu bar interactions (Row 1)
    if (mouse.y == 1) {
        for (size_t i = 0; i < m_menus.size(); ++i) {
            int startX = m_menus[i].startX;
            int endX = startX + m_menus[i].width;
            if (mouse.x >= startX && mouse.x <= endX) {
                if (m_activeMenu == static_cast<int>(i)) {
                    m_activeMenu = -1; // Toggle off if clicked again
                } else {
                    m_activeMenu = static_cast<int>(i);
                    m_selectedMenuItem = 0;
                }
                return;
            }
        }
        m_activeMenu = -1;
        return;
    }

    // 3. Dropdown Menu interactions (when active)
    if (m_activeMenu >= 0 && m_activeMenu < static_cast<int>(m_menus.size())) {
        const Menu& menu = m_menus[m_activeMenu];
        int menuW = 24;
        for (const auto& it : menu.items) {
            int len = static_cast<int>(it.label.size() + it.shortcut.size()) + 4;
            if (len > menuW) menuW = len;
        }
        int menuX = menu.startX;
        if (menuX + menuW >= w) menuX = w - menuW - 1;
        int menuY = 2;
        int menuH = static_cast<int>(menu.items.size()) + 2;

        if (mouse.x >= menuX && mouse.x < menuX + menuW &&
            mouse.y >= menuY && mouse.y < menuY + menuH) {
            int itemIdx = mouse.y - menuY - 1;
            if (itemIdx >= 0 && itemIdx < static_cast<int>(menu.items.size())) {
                const auto& item = menu.items[itemIdx];
                if (!item.isSeparator) {
                    if (item.actionId == ACTION_NONE && item.label == L"Cycle Theme") {
                        cycleTheme();
                        m_activeMenu = -1;
                    } else {
                        executeMenuAction(item.actionId);
                    }
                }
            }
            return;
        } else {
            // Click outside dropdown closes it
            m_activeMenu = -1;
        }
    }

    // 4. Modal Dialog interactions
    if (m_mode == EditorMode::AboutModal || m_mode == EditorMode::HelpModal) {
        // Any click outside or on [OK] dismisses modal
        m_mode = EditorMode::Normal;
        return;
    }

    if (m_mode == EditorMode::Search ||
        m_mode == EditorMode::SavePrompt ||
        m_mode == EditorMode::GoToLinePrompt) {
        int boxW = (std::min)(54, w - 4);
        int boxH = 8;
        int boxX = (w - boxW) / 2;
        int boxY = (h - boxH) / 2;

        // Check close button [X] at boxX + boxW - 4, boxY
        if (mouse.y == boxY && mouse.x >= boxX + boxW - 5 && mouse.x < boxX + boxW - 1) {
            m_mode = EditorMode::Normal;
            setStatus(L"Cancelled");
            return;
        }

        // Input field click
        if (mouse.y == boxY + 3 && mouse.x >= boxX + 3 && mouse.x < boxX + boxW - 3) {
            m_dialogFocus = 0;
            int clickedChar = mouse.x - (boxX + 3);
            m_promptCursor = (std::min)(clickedChar, static_cast<int>(m_promptInput.size()));
            return;
        }

        // Buttons click
        int btnY = boxY + 5;
        if (mouse.y == btnY) {
            // Action button around boxX + 6..20
            if (mouse.x >= boxX + 6 && mouse.x <= boxX + 22) {
                m_dialogFocus = 1;
                KeyEvent enterKey;
                enterKey.vkCode = VK_RETURN;
                handleDialogKey(enterKey);
                return;
            }
            // Cancel button around boxX + 26..38
            if (mouse.x >= boxX + 26 && mouse.x <= boxX + 42) {
                m_mode = EditorMode::Normal;
                setStatus(L"Cancelled");
                return;
            }
        }
        return;
    }

    // 5. Scrollbar interactions (Column w - 1, Rows 2 .. h - 2)
    if (mouse.x == w - 1 && mouse.y >= 2 && mouse.y <= h - 2) {
        int startY = 2;
        int endY = h - 2;
        if (mouse.y == startY) {
            // Up arrow
            m_scrollRow = (std::max)(0, m_scrollRow - 1);
        } else if (mouse.y == endY) {
            // Down arrow
            m_scrollRow = (std::min)(static_cast<int>(m_doc.lineCount()) - 1, m_scrollRow + 1);
        } else {
            // Track click: page up or down relative to thumb
            int trackHeight = endY - startY - 1;
            int totalLines = static_cast<int>(m_doc.lineCount());
            int eHeight = getEditorHeight();
            int thumbSize = (std::max)(1, (totalLines > 0) ? (trackHeight * eHeight / totalLines) : trackHeight);
            int maxScroll = (std::max)(1, totalLines - eHeight);
            int thumbStart = startY + 1 + (trackHeight - thumbSize) * std::clamp(m_scrollRow, 0, maxScroll) / maxScroll;

            if (mouse.y < thumbStart) {
                pageUp(false);
            } else if (mouse.y >= thumbStart + thumbSize) {
                pageDown(false);
            }
        }
        return;
    }

    // 6. Text canvas interactions
    int margin = getMarginWidth();
    int topOffset = 2;
    int bottomOffset = h - 1;

    if (mouse.y >= topOffset && mouse.y < bottomOffset && mouse.x >= margin && mouse.x < w - 1) {
        int targetRow = m_scrollRow + (mouse.y - topOffset);
        if (targetRow >= 0 && targetRow < static_cast<int>(m_doc.lineCount())) {
            int targetVCol = m_scrollCol + (mouse.x - margin);
            int targetCol = visualToCharCol(targetRow, targetVCol);
            m_cursorRow = targetRow;
            m_cursorCol = targetCol;
            m_preferredCol = targetCol;
            m_hasSelection = false;
        }
    }
}

void Editor::handleNormalKey(const KeyEvent& key) {
    // Reset quit confirmation if another key pressed
    if (m_quitConfirmPending && !(key.ctrl && key.vkCode == 'Q')) {
        m_quitConfirmPending = false;
    }

    // Alt combinations: Windows 9x menus & theme shortcuts
    if (key.alt && !key.ctrl) {
        switch (key.vkCode) {
            case 'F': m_activeMenu = 0; m_selectedMenuItem = 0; return;
            case 'E': m_activeMenu = 1; m_selectedMenuItem = 0; return;
            case 'S': m_activeMenu = 2; m_selectedMenuItem = 0; return;
            case 'V': m_activeMenu = 3; m_selectedMenuItem = 0; return;
            case 'H': m_activeMenu = 4; m_selectedMenuItem = 0; return;
            case 'T': cycleTheme(); return;
            case '1': setTheme(ThemeType::Win98Dark); return;
            case '2': setTheme(ThemeType::Win95Notepad); return;
            case '3': setTheme(ThemeType::DosEditBlue); return;
            case VK_F4: cmdExit(); return;
            default: break;
        }
    }

    // F10 opens menu bar
    if (key.vkCode == VK_F10) {
        m_activeMenu = 0;
        m_selectedMenuItem = 0;
        return;
    }

    // Ctrl combinations
    if (key.ctrl && !key.alt) {
        switch (key.vkCode) {
            case 'N': executeMenuAction(ACTION_NEW); return;
            case 'O': executeMenuAction(ACTION_OPEN); return;
            case 'S': cmdSave(); return;
            case 'W': cmdSaveAs(); return;
            case 'Q': cmdExit(); return;
            case 'Z': performUndo(); return;
            case 'Y': performRedo(); return;
            case 'C': copyToClipboard(); return;
            case 'X': cutToClipboard(); return;
            case 'V': pasteFromClipboard(); return;
            case 'A': selectAll(); return;
            case 'D': {
                recordUndo();
                m_doc.deleteLine(m_cursorRow, m_cursorCol);
                m_preferredCol = m_cursorCol;
                setStatus(L"Deleted line");
                return;
            }
            case 'F': cmdFind(); return;
            case 'G': cmdGoToLine(); return;
            case 'H': m_mode = EditorMode::HelpModal; return;
            case VK_HOME: moveToDocStart(key.shift); return;
            case VK_END: moveToDocEnd(key.shift); return;
            case VK_LEFT: moveWordLeft(key.shift); return;
            case VK_RIGHT: moveWordRight(key.shift); return;
            case VK_UP:
                m_scrollRow = (std::max)(0, m_scrollRow - 1);
                return;
            case VK_DOWN:
                m_scrollRow = (std::min)(static_cast<int>(m_doc.lineCount()) - 1, m_scrollRow + 1);
                return;
            default:
                break;
        }
    }

    // Function keys
    switch (key.vkCode) {
        case VK_F1:
            m_mode = EditorMode::HelpModal;
            return;
        case VK_F3:
            cmdFindNext(!key.shift);
            return;
        case VK_ESCAPE:
            m_hasSelection = false;
            m_lastSearchQuery.clear();
            setStatus(L"");
            return;
        case VK_UP:
            moveCursor(-1, 0, key.shift);
            return;
        case VK_DOWN:
            moveCursor(1, 0, key.shift);
            return;
        case VK_LEFT:
            moveCursor(0, -1, key.shift);
            return;
        case VK_RIGHT:
            moveCursor(0, 1, key.shift);
            return;
        case VK_HOME:
            moveToLineStart(key.shift);
            return;
        case VK_END:
            moveToLineEnd(key.shift);
            return;
        case VK_PRIOR: // Page Up
            pageUp(key.shift);
            return;
        case VK_NEXT: // Page Down
            pageDown(key.shift);
            return;
        case VK_RETURN: {
            if (m_hasSelection) deleteSelection();
            recordUndo(false);
            const std::wstring& line = m_doc.getLine(m_cursorRow);
            if (m_autoPairsEnabled && m_cursorCol > 0 && m_cursorCol < static_cast<int>(line.size())) {
                wchar_t prevChar = line[m_cursorCol - 1];
                wchar_t nextChar = line[m_cursorCol];
                if ((prevChar == L'{' && nextChar == L'}') ||
                    (prevChar == L'(' && nextChar == L')') ||
                    (prevChar == L'[' && nextChar == L']')) {
                    m_doc.insertExpandedBrackets(m_cursorRow, m_cursorCol);
                    m_preferredCol = m_cursorCol;
                    m_isTypingGroup = false;
                    return;
                }
            }
            m_doc.insertNewline(m_cursorRow, m_cursorCol, true);
            m_preferredCol = m_cursorCol;
            m_isTypingGroup = false;
            return;
        }
        case VK_BACK: {
            if (m_hasSelection) {
                deleteSelection();
            } else {
                recordUndo(false);
                if (m_autoPairsEnabled && m_doc.isBetweenPair(m_cursorRow, m_cursorCol)) {
                    m_doc.deleteChar(m_cursorRow, m_cursorCol);
                    m_doc.backspace(m_cursorRow, m_cursorCol);
                } else {
                    m_doc.backspace(m_cursorRow, m_cursorCol);
                }
                m_preferredCol = m_cursorCol;
            }
            m_isTypingGroup = false;
            return;
        }
        case VK_DELETE: {
            if (m_hasSelection) {
                deleteSelection();
            } else {
                recordUndo(false);
                m_doc.deleteChar(m_cursorRow, m_cursorCol);
                m_preferredCol = m_cursorCol;
            }
            m_isTypingGroup = false;
            return;
        }
        case VK_TAB: {
            if (key.shift) {
                recordUndo(false);
                m_doc.unindentLine(m_cursorRow, m_cursorCol);
                m_preferredCol = m_cursorCol;
            } else {
                if (m_hasSelection) deleteSelection();
                recordUndo(false);
                m_doc.insertText(m_cursorRow, m_cursorCol, L"    ");
                m_preferredCol = m_cursorCol;
            }
            m_isTypingGroup = false;
            return;
        }
        default:
            break;
    }

    // Printable character entry
    if (!key.ctrl && !key.alt && key.ch >= 32) {
        wchar_t ch = key.ch;
        wchar_t closePair = m_autoPairsEnabled ? getClosingPair(ch) : 0;

        // 1. Auto-wrap selection if pair typed
        if (m_hasSelection && closePair != 0) {
            recordUndo(false);
            int sR, sC, eR, eC;
            getSelectionBounds(sR, sC, eR, eC);
            m_doc.wrapSelection(sR, sC, eR, eC, ch, closePair);
            m_selAnchorRow = sR;
            m_selAnchorCol = sC;
            m_cursorRow = eR;
            m_cursorCol = (sR == eR) ? (eC + 2) : (eC + 1);
            m_preferredCol = m_cursorCol;
            m_isTypingGroup = false;
            return;
        }

        if (m_hasSelection) {
            deleteSelection();
        }

        const std::wstring& line = m_doc.getLine(m_cursorRow);

        // 2. Skip-over matching closing character right ahead
        if (m_autoPairsEnabled && isClosingChar(ch) &&
            m_cursorCol < static_cast<int>(line.size()) && line[m_cursorCol] == ch) {
            m_cursorCol++;
            m_preferredCol = m_cursorCol;
            m_isTypingGroup = false;
            return;
        }

        // 3. Auto-close opening pair
        if (closePair != 0) {
            bool shouldAutoClose = true;

            // Don't auto-close single quote inside words (it's, don't)
            if (ch == L'\'') {
                if (m_cursorCol > 0 && iswalnum(line[m_cursorCol - 1])) {
                    shouldAutoClose = false;
                }
            }

            // Don't auto-close after backslash
            if (m_cursorCol > 0 && line[m_cursorCol - 1] == L'\\') {
                shouldAutoClose = false;
            }

            // Don't auto-close immediately before alphanumeric identifier
            if (m_cursorCol < static_cast<int>(line.size()) && iswalnum(line[m_cursorCol])) {
                shouldAutoClose = false;
            }

            if (shouldAutoClose) {
                recordUndo(true);
                m_doc.insertPair(m_cursorRow, m_cursorCol, ch, closePair);
                m_preferredCol = m_cursorCol;
                return;
            }
        }

        // 4. Default character insertion
        recordUndo(true);
        m_doc.insertChar(m_cursorRow, m_cursorCol, ch);
        m_preferredCol = m_cursorCol;
    }
}

void Editor::handleEvent(const AppEvent& event) {
    if (event.type == EventType::Resize) {
        adjustScroll();
        return;
    }

    if (event.type == EventType::Mouse) {
        handleMouseEvent(event.mouse);
        return;
    }

    if (event.type == EventType::Key) {
        if (m_activeMenu >= 0) {
            handleMenuKey(event.key);
            return;
        }

        switch (m_mode) {
            case EditorMode::Normal:
                handleNormalKey(event.key);
                break;
            case EditorMode::Search:
            case EditorMode::SavePrompt:
            case EditorMode::GoToLinePrompt:
                handleDialogKey(event.key);
                break;
            case EditorMode::HelpModal:
            case EditorMode::AboutModal:
                handleHelpKey(event.key);
                break;
        }
    }
}

void Editor::renderTopBar() {
    int w = m_console.getWidth();
    ThemeColors tc = getThemeColors(m_currentTheme);

    // Title bar background
    m_console.fillRect(0, 0, w, 1, L' ', tc.titleBarActive);

    // Left Application Icon & Title
    std::wstring icon = L" [■] ";
    std::wstring title = L"WinEdit 98 - " + m_doc.getFileName();
    if (m_doc.isModified()) {
        title += L" *";
    }

    m_console.setString(0, 0, icon, tc.titleBarActive);
    m_console.setString(static_cast<int>(icon.size()), 0, title, tc.titleBarActive);

    // Close button on the right: [ X ]
    std::wstring closeBtn = L" [ X ] ";
    int bx = w - static_cast<int>(closeBtn.size());
    if (bx > static_cast<int>(icon.size() + title.size())) {
        m_console.setString(bx, 0, closeBtn, tc.titleBarCloseBtn);
    }
}

void Editor::renderMenuBar() {
    int w = m_console.getWidth();
    ThemeColors tc = getThemeColors(m_currentTheme);

    // Fill menu bar row
    m_console.fillRect(0, 1, w, 1, L' ', tc.menuBar);

    for (size_t i = 0; i < m_menus.size(); ++i) {
        const Menu& menu = m_menus[i];
        int startX = menu.startX;
        bool isActive = (m_activeMenu == static_cast<int>(i));

        WORD itemAttr = isActive ? tc.menuDropdownSelected : tc.menuBar;
        WORD hotkeyAttr = isActive ? tc.menuDropdownSelected : tc.menuBarHotkey;

        m_console.setChar(startX, 1, L' ', itemAttr);
        for (size_t c = 0; c < menu.title.size(); ++c) {
            wchar_t ch = menu.title[c];
            bool isHotkey = (towupper(ch) == towupper(menu.hotkeyChar));
            m_console.setChar(startX + 1 + static_cast<int>(c), 1, ch, isHotkey ? hotkeyAttr : itemAttr);
        }
        m_console.setChar(startX + 1 + static_cast<int>(menu.title.size()), 1, L' ', itemAttr);
    }
}

void Editor::renderLines() {
    int margin = getMarginWidth();
    int eHeight = getEditorHeight();
    int eWidth = getEditorWidth();
    int totalLines = static_cast<int>(m_doc.lineCount());
    ThemeColors tc = getThemeColors(m_currentTheme);

    for (int y = 0; y < eHeight; ++y) {
        int screenY = 2 + y; // Row 0 is Title, Row 1 is Menu Bar
        int fileRow = m_scrollRow + y;

        if (fileRow < totalLines) {
            const std::wstring& line = m_doc.getLine(fileRow);
            bool isCurrent = (fileRow == m_cursorRow);

            // Render margin (line numbers)
            if (m_showLineNumbers) {
                std::wstringstream ss;
                int numWidth = margin - 3;
                ss << std::setw(numWidth) << (fileRow + 1) << L" │ ";
                m_console.setString(0, screenY, ss.str().substr(0, margin - 1),
                                    isCurrent ? tc.marginCurrent : tc.margin);
                m_console.setChar(margin - 2, screenY, L'\u2502', tc.marginSep);
                m_console.setChar(margin - 1, screenY, L' ', tc.editorBg);
            } else {
                m_console.setChar(0, screenY, L' ', tc.editorBg);
            }

            // Expand tabs to visual columns
            std::vector<wchar_t> visualChars;
            std::vector<int> charIndices;
            for (size_t i = 0; i < line.size(); ++i) {
                if (line[i] == L'\t') {
                    int spaces = 4 - (static_cast<int>(visualChars.size()) % 4);
                    for (int s = 0; s < spaces; ++s) {
                        visualChars.push_back(L' ');
                        charIndices.push_back(static_cast<int>(i));
                    }
                } else {
                    visualChars.push_back(line[i]);
                    charIndices.push_back(static_cast<int>(i));
                }
            }

            // Mark search matches
            std::vector<bool> isSearchMatch(line.size(), false);
            if (!m_lastSearchQuery.empty()) {
                std::wstring lineLower = line;
                std::wstring qLower = m_lastSearchQuery;
                for (wchar_t& ch : lineLower) ch = towlower(ch);
                for (wchar_t& ch : qLower) ch = towlower(ch);

                size_t pos = 0;
                while ((pos = lineLower.find(qLower, pos)) != std::wstring::npos) {
                    for (size_t k = 0; k < qLower.size() && (pos + k) < line.size(); ++k) {
                        isSearchMatch[pos + k] = true;
                    }
                    pos += qLower.size();
                }
            }

            // Syntax highlighting passing current theme
            std::vector<WORD> syntaxAttrs;
            m_syntax.highlightLine(line, fileRow, m_language, syntaxAttrs, m_currentTheme);

            // Render visible characters
            for (int vx = 0; vx < eWidth; ++vx) {
                int totalVCol = m_scrollCol + vx;
                int screenX = margin + vx;

                if (totalVCol < static_cast<int>(visualChars.size())) {
                    wchar_t ch = visualChars[totalVCol];
                    int charIdx = charIndices[totalVCol];

                    WORD attr = tc.textDefault;
                    if (charIdx < static_cast<int>(syntaxAttrs.size())) {
                        attr = syntaxAttrs[charIdx];
                    }
                    if (charIdx < static_cast<int>(isSearchMatch.size()) && isSearchMatch[charIdx]) {
                        attr = tc.searchMatch;
                    } else if (isCellSelected(fileRow, charIdx)) {
                        attr = tc.selection;
                    }

                    m_console.setChar(screenX, screenY, ch, attr);
                } else {
                    WORD attr = tc.editorBg;
                    if (m_hasSelection && isCellSelected(fileRow, static_cast<int>(line.size()))) {
                        attr = tc.selection;
                    }
                    m_console.setChar(screenX, screenY, L' ', attr);
                }
            }
        } else {
            // Beyond document end
            m_console.fillRect(0, screenY, margin + eWidth, 1, L' ', tc.editorBg);
            m_console.setChar(1, screenY, L'~', tc.margin);
        }
    }
}

void Editor::renderScrollbar() {
    int w = m_console.getWidth();
    int h = m_console.getHeight();
    int sx = w - 1;
    int startY = 2;
    int endY = h - 2;
    int trackHeight = endY - startY - 1;
    ThemeColors tc = getThemeColors(m_currentTheme);

    // Top scroll button ▲
    m_console.setChar(sx, startY, L'\u25B2', tc.scrollbarArrow);

    // Bottom scroll button ▼
    m_console.setChar(sx, endY, L'\u25BC', tc.scrollbarArrow);

    if (trackHeight <= 0) return;

    int totalLines = static_cast<int>(m_doc.lineCount());
    int eHeight = getEditorHeight();

    int thumbSize = (std::max)(1, (totalLines > 0) ? (trackHeight * eHeight / totalLines) : trackHeight);
    int maxScroll = (std::max)(1, totalLines - eHeight);
    int thumbStart = startY + 1 + (trackHeight - thumbSize) * std::clamp(m_scrollRow, 0, maxScroll) / maxScroll;
    int thumbEnd = thumbStart + thumbSize;

    for (int y = startY + 1; y < endY; ++y) {
        if (y >= thumbStart && y < thumbEnd) {
            m_console.setChar(sx, y, L'\u2588', tc.scrollbarThumb);
        } else {
            m_console.setChar(sx, y, L'\u2591', tc.scrollbarTrack);
        }
    }
}

void Editor::renderStatusBar() {
    int w = m_console.getWidth();
    int y = m_console.getHeight() - 1;
    ThemeColors tc = getThemeColors(m_currentTheme);

    m_console.fillRect(0, y, w, 1, L' ', tc.statusBar);

    // Format Panes
    int totalLines = static_cast<int>(m_doc.lineCount());
    int percent = (totalLines > 0) ? ((m_cursorRow + 1) * 100 / totalLines) : 100;
    std::wstring langName = SyntaxHighlighter::getLanguageName(m_language);

    std::wstringstream rightPanels;
    rightPanels << L" [ Ln " << (m_cursorRow + 1) << L", Col " << (m_cursorCol + 1) << L" ] "
                << L"│ [ " << percent << L"% ] "
                << L"│ [ " << langName << L" ] "
                << L"│ [ INS ] "
                << L"│ [ CRLF ] ";
    std::wstring rightStr = rightPanels.str();

    // Left message panel
    std::wstring leftMsg;
    if (!m_statusMessage.empty()) {
        leftMsg = L" " + m_statusMessage;
    } else {
        leftMsg = L" Ready";
        if (m_doc.isModified()) {
            leftMsg += L" (Modified)";
        }
    }

    int rightX = w - static_cast<int>(rightStr.size());
    if (rightX > 10) {
        int leftWidth = rightX - 2;
        if (static_cast<int>(leftMsg.size()) > leftWidth) {
            leftMsg = leftMsg.substr(0, leftWidth);
        }
        std::wstring sunkenLeft = L"[" + leftMsg + std::wstring((std::max)(0, leftWidth - static_cast<int>(leftMsg.size()) - 1), L' ') + L"]";
        m_console.setString(0, y, sunkenLeft, tc.statusBarPanel);
        m_console.setChar(leftWidth + 1, y, L'\u2502', tc.statusBarBorder);
        m_console.setString(rightX, y, rightStr, tc.statusBar);
    } else {
        m_console.setString(0, y, leftMsg, tc.statusBar);
    }
}

void Editor::renderDropDownMenu() {
    if (m_activeMenu < 0 || m_activeMenu >= static_cast<int>(m_menus.size())) {
        return;
    }

    const Menu& menu = m_menus[m_activeMenu];
    int scrW = m_console.getWidth();
    ThemeColors tc = getThemeColors(m_currentTheme);

    int menuW = 24;
    for (const auto& item : menu.items) {
        int len = static_cast<int>(item.label.size() + item.shortcut.size()) + 4;
        if (len > menuW) menuW = len;
    }

    int boxX = menu.startX;
    if (boxX + menuW >= scrW) {
        boxX = scrW - menuW - 1;
    }
    int boxY = 2;
    int boxH = static_cast<int>(menu.items.size()) + 2;

    // Outer borders
    m_console.fillRect(boxX, boxY, menuW, boxH, L' ', tc.menuDropdown);

    // Draw single-line border
    m_console.setChar(boxX, boxY, L'\u250C', tc.menuDropdownBorder);
    m_console.setChar(boxX + menuW - 1, boxY, L'\u2510', tc.menuDropdownBorder);
    m_console.setChar(boxX, boxY + boxH - 1, L'\u2514', tc.menuDropdownBorder);
    m_console.setChar(boxX + menuW - 1, boxY + boxH - 1, L'\u2518', tc.menuDropdownBorder);

    for (int x = boxX + 1; x < boxX + menuW - 1; ++x) {
        m_console.setChar(x, boxY, L'\u2500', tc.menuDropdownBorder);
        m_console.setChar(x, boxY + boxH - 1, L'\u2500', tc.menuDropdownBorder);
    }
    for (int y = boxY + 1; y < boxY + boxH - 1; ++y) {
        m_console.setChar(boxX, y, L'\u2502', tc.menuDropdownBorder);
        m_console.setChar(boxX + menuW - 1, y, L'\u2502', tc.menuDropdownBorder);
    }

    // Drop shadow on right and bottom
    for (int y = boxY + 1; y <= boxY + boxH; ++y) {
        m_console.setChar(boxX + menuW, y, L' ', tc.menuDropdownShadow);
    }
    for (int x = boxX + 1; x <= boxX + menuW; ++x) {
        m_console.setChar(x, boxY + boxH, L' ', tc.menuDropdownShadow);
    }

    // Render items
    for (size_t i = 0; i < menu.items.size(); ++i) {
        const MenuItem& item = menu.items[i];
        int itemY = boxY + 1 + static_cast<int>(i);

        if (item.isSeparator) {
            m_console.setChar(boxX, itemY, L'\u251C', tc.menuDropdownBorder);
            for (int x = boxX + 1; x < boxX + menuW - 1; ++x) {
                m_console.setChar(x, itemY, L'\u2500', tc.menuDropdownBorder);
            }
            m_console.setChar(boxX + menuW - 1, itemY, L'\u2524', tc.menuDropdownBorder);
        } else {
            bool isSelected = (m_selectedMenuItem == static_cast<int>(i));
            WORD itemAttr = isSelected ? tc.menuDropdownSelected : tc.menuDropdown;
            WORD hotkeyAttr = isSelected ? tc.menuDropdownSelected : tc.menuDropdownHotkey;

            m_console.fillRect(boxX + 1, itemY, menuW - 2, 1, L' ', itemAttr);

            // Item label
            int lx = boxX + 2;
            for (size_t c = 0; c < item.label.size(); ++c) {
                wchar_t ch = item.label[c];
                bool isHotkey = (towupper(ch) == towupper(item.hotkeyChar));
                m_console.setChar(lx + static_cast<int>(c), itemY, ch, isHotkey ? hotkeyAttr : itemAttr);
            }

            // Item shortcut right-aligned
            if (!item.shortcut.empty()) {
                int sx = boxX + menuW - 2 - static_cast<int>(item.shortcut.size());
                m_console.setString(sx, itemY, item.shortcut, itemAttr);
            }
        }
    }
}

void Editor::renderInputDialog(const std::wstring& title, const std::wstring& label,
                              const std::wstring& actionBtn, const std::wstring& cancelBtn) {
    int scrW = m_console.getWidth();
    int scrH = m_console.getHeight();
    ThemeColors tc = getThemeColors(m_currentTheme);

    int boxW = (std::min)(54, scrW - 4);
    int boxH = 8;
    int boxX = (scrW - boxW) / 2;
    int boxY = (scrH - boxH) / 2;

    // Body fill
    m_console.fillRect(boxX, boxY, boxW, boxH, L' ', tc.dialogBody);

    // Title bar with icon & close button
    m_console.fillRect(boxX, boxY, boxW, 1, L' ', tc.dialogTitle);
    m_console.setString(boxX + 1, boxY, L"■ " + title, tc.dialogTitle);
    m_console.setString(boxX + boxW - 5, boxY, L"[ X ]", tc.titleBarCloseBtn);

    // Borders & Drop shadow
    for (int y = boxY + 1; y < boxY + boxH; ++y) {
        m_console.setChar(boxX, y, L'\u2502', tc.dialogBorder);
        m_console.setChar(boxX + boxW - 1, y, L'\u2502', tc.dialogBorder);
        // Shadow right
        m_console.setChar(boxX + boxW, y, L' ', Color::DarkGray | Color::BgBlack);
    }
    for (int x = boxX; x < boxX + boxW; ++x) {
        m_console.setChar(x, boxY + boxH - 1, L'\u2500', tc.dialogBorder);
        // Shadow bottom
        m_console.setChar(x + 1, boxY + boxH, L' ', Color::DarkGray | Color::BgBlack);
    }
    m_console.setChar(boxX, boxY + boxH - 1, L'\u2514', tc.dialogBorder);
    m_console.setChar(boxX + boxW - 1, boxY + boxH - 1, L'\u2518', tc.dialogBorder);

    // Label
    m_console.setString(boxX + 3, boxY + 2, label, tc.dialogBody);

    // Sunken text input field
    int inputW = boxW - 6;
    m_console.fillRect(boxX + 3, boxY + 3, inputW, 1, L' ', tc.dialogInput);
    m_console.setString(boxX + 3, boxY + 3, m_promptInput.substr(0, inputW), tc.dialogInput);

    // Push buttons: Action and Cancel
    std::wstring b1 = L"[  " + actionBtn + L"  ]";
    std::wstring b2 = L"[  " + cancelBtn + L"  ]";
    int btn1X = boxX + 6;
    int btn2X = boxX + boxW - static_cast<int>(b2.size()) - 6;

    m_console.setString(btn1X, boxY + 5, b1, (m_dialogFocus == 1) ? tc.dialogButtonFocus : tc.dialogButton);
    m_console.setString(btn2X, boxY + 5, b2, (m_dialogFocus == 2) ? tc.dialogButtonFocus : tc.dialogButton);
}

void Editor::renderAboutModal() {
    int scrW = m_console.getWidth();
    int scrH = m_console.getHeight();
    ThemeColors tc = getThemeColors(m_currentTheme);

    int boxW = (std::min)(60, scrW - 4);
    int boxH = 15;
    int boxX = (scrW - boxW) / 2;
    int boxY = (scrH - boxH) / 2;

    m_console.fillRect(boxX, boxY, boxW, boxH, L' ', tc.dialogBody);

    // Title bar
    m_console.fillRect(boxX, boxY, boxW, 1, L' ', tc.dialogTitle);
    m_console.setString(boxX + 1, boxY, L"■ About WinEdit 98", tc.dialogTitle);
    m_console.setString(boxX + boxW - 5, boxY, L"[ X ]", tc.titleBarCloseBtn);

    // Borders & Shadow
    for (int y = boxY + 1; y < boxY + boxH; ++y) {
        m_console.setChar(boxX, y, L'\u2502', tc.dialogBorder);
        m_console.setChar(boxX + boxW - 1, y, L'\u2502', tc.dialogBorder);
        m_console.setChar(boxX + boxW, y, L' ', Color::DarkGray | Color::BgBlack);
    }
    for (int x = boxX; x < boxX + boxW; ++x) {
        m_console.setChar(x, boxY + boxH - 1, L'\u2500', tc.dialogBorder);
        m_console.setChar(x + 1, boxY + boxH, L' ', Color::DarkGray | Color::BgBlack);
    }
    m_console.setChar(boxX, boxY + boxH - 1, L'\u2514', tc.dialogBorder);
    m_console.setChar(boxX + boxW - 1, boxY + boxH - 1, L'\u2518', tc.dialogBorder);

    // Content
    m_console.setString(boxX + 3, boxY + 2, std::wstring(L"[■] ") + Version::FullName + L" - 32-Bit Text Editor", tc.dialogBody);
    m_console.setString(boxX + 7, boxY + 3, std::wstring(L"Build ") + Version::RetroBuild, tc.dialogBody);
    m_console.setString(boxX + 7, boxY + 4, L"Zaloopa", tc.dialogBody);

    m_console.setString(boxX + 3, boxY + 6, L"Features:", tc.dialogBody);
    m_console.setString(boxX + 5, boxY + 7, L"• Syntax Highlighting (C++, Python, Lua, JSON, MD)", tc.dialogBody);
    m_console.setString(boxX + 5, boxY + 8, L"• Auto-Closing Pairs & Skip-Over: (), [], {}, \"\", ''", tc.dialogBody);
    m_console.setString(boxX + 5, boxY + 9, L"• Windows 9x Dropdown Menus, Scrollbar & Status Bar", tc.dialogBody);
    m_console.setString(boxX + 5, boxY + 10, L"• 3 Themes: Win98 Dark, Win95 Notepad, MS-DOS Edit", tc.dialogBody);

    // OK Button
    std::wstring okBtn = L"[    OK    ]";
    int bx = boxX + (boxW - static_cast<int>(okBtn.size())) / 2;
    m_console.setString(bx, boxY + 12, okBtn, tc.dialogButtonFocus);
}

void Editor::renderHelpModal() {
    int scrW = m_console.getWidth();
    int scrH = m_console.getHeight();
    ThemeColors tc = getThemeColors(m_currentTheme);

    int boxW = (std::min)(68, scrW - 4);
    int boxH = (std::min)(22, scrH - 4);
    int boxX = (scrW - boxW) / 2;
    int boxY = (scrH - boxH) / 2;

    m_console.fillRect(boxX, boxY, boxW, boxH, L' ', tc.dialogBody);

    // Title bar
    m_console.fillRect(boxX, boxY, boxW, 1, L' ', tc.dialogTitle);
    m_console.setString(boxX + 1, boxY, L"■ WinEdit 98 Help Topics & Shortcuts", tc.dialogTitle);
    m_console.setString(boxX + boxW - 5, boxY, L"[ X ]", tc.titleBarCloseBtn);

    // Borders & Shadow
    for (int y = boxY + 1; y < boxY + boxH; ++y) {
        m_console.setChar(boxX, y, L'\u2502', tc.dialogBorder);
        m_console.setChar(boxX + boxW - 1, y, L'\u2502', tc.dialogBorder);
        m_console.setChar(boxX + boxW, y, L' ', Color::DarkGray | Color::BgBlack);
    }
    for (int x = boxX; x < boxX + boxW; ++x) {
        m_console.setChar(x, boxY + boxH - 1, L'\u2500', tc.dialogBorder);
        m_console.setChar(x + 1, boxY + boxH, L' ', Color::DarkGray | Color::BgBlack);
    }
    m_console.setChar(boxX, boxY + boxH - 1, L'\u2514', tc.dialogBorder);
    m_console.setChar(boxX + boxW - 1, boxY + boxH - 1, L'\u2518', tc.dialogBorder);

    std::vector<std::pair<std::wstring, bool>> helpLines = {
        { L"Menu & Navigation:", true },
        { L"  Alt+F/E/S/V/H     Open File / Edit / Search / View / Help Menu", false },
        { L"  Alt+T / Alt+1/2/3 Cycle themes / Win98 Dark / Notepad / DOS Edit", false },
        { L"  Arrow Keys        Move cursor / Line start / Line end", false },
        { L"  Ctrl + Left/Right Skip word left / right", false },
        { L"  PageUp / PageDown Scroll view by page", false },
        { L"", false },
        { L"Editing & Clipboard:", true },
        { L"  Shift + Arrows    Select text range", false },
        { L"  Ctrl + C / X / V  Copy / Cut / Paste", false },
        { L"  Ctrl + A / D      Select All / Delete line", false },
        { L"  Ctrl + Z / Y      Undo / Redo edits", false },
        { L"  (), [], {}, \"\", '' Auto-close pair, skip-over, and bracket expand", false },
        { L"", false },
        { L"File & Search:", true },
        { L"  Ctrl + S / Ctrl+W Save file / Save As...", false },
        { L"  Ctrl + F / F3     Find text / Find Next (Shift+F3 Previous)", false },
        { L"  Ctrl + G          Go to line number", false },
        { L"  Ctrl + Q / Alt+F4 Exit WinEdit 98", false }
    };

    int startY = boxY + 2;
    for (size_t i = 0; i < helpLines.size() && (startY + static_cast<int>(i)) < (boxY + boxH - 2); ++i) {
        WORD attr = helpLines[i].second ? (Color::DarkBlue | Color::BgGray) : tc.dialogBody;
        m_console.setString(boxX + 3, startY + static_cast<int>(i), helpLines[i].first, attr, boxW - 6);
    }

    std::wstring closeBtn = L"[   Close   ]";
    int bx = boxX + (boxW - static_cast<int>(closeBtn.size())) / 2;
    m_console.setString(bx, boxY + boxH - 2, closeBtn, tc.dialogButtonFocus);
}

void Editor::render() {
    ThemeColors tc = getThemeColors(m_currentTheme);
    m_console.clear(tc.editorBg);

    renderTopBar();
    renderMenuBar();
    renderLines();
    renderScrollbar();
    renderStatusBar();

    // Render open dropdown menu
    if (m_activeMenu >= 0) {
        renderDropDownMenu();
        m_console.setCursor(0, 0, false);
    } else if (m_mode == EditorMode::Search) {
        renderInputDialog(L"Find", L"Find what:", L"Find Next", L"Cancel");
        int boxW = (std::min)(54, m_console.getWidth() - 4);
        int boxX = (m_console.getWidth() - boxW) / 2;
        int boxY = (m_console.getHeight() - 8) / 2;
        if (m_dialogFocus == 0) {
            m_console.setCursor(boxX + 3 + m_promptCursor, boxY + 3, true);
        } else {
            m_console.setCursor(0, 0, false);
        }
    } else if (m_mode == EditorMode::SavePrompt) {
        renderInputDialog(L"Save As", L"File name:", L"Save", L"Cancel");
        int boxW = (std::min)(54, m_console.getWidth() - 4);
        int boxX = (m_console.getWidth() - boxW) / 2;
        int boxY = (m_console.getHeight() - 8) / 2;
        if (m_dialogFocus == 0) {
            m_console.setCursor(boxX + 3 + m_promptCursor, boxY + 3, true);
        } else {
            m_console.setCursor(0, 0, false);
        }
    } else if (m_mode == EditorMode::GoToLinePrompt) {
        renderInputDialog(L"Go To Line", m_promptLabel, L"OK", L"Cancel");
        int boxW = (std::min)(54, m_console.getWidth() - 4);
        int boxX = (m_console.getWidth() - boxW) / 2;
        int boxY = (m_console.getHeight() - 8) / 2;
        if (m_dialogFocus == 0) {
            m_console.setCursor(boxX + 3 + m_promptCursor, boxY + 3, true);
        } else {
            m_console.setCursor(0, 0, false);
        }
    } else if (m_mode == EditorMode::HelpModal) {
        renderHelpModal();
        m_console.setCursor(0, 0, false);
    } else if (m_mode == EditorMode::AboutModal) {
        renderAboutModal();
        m_console.setCursor(0, 0, false);
    } else {
        // Normal cursor in editor canvas
        int vCol = charToVisualCol(m_cursorRow, m_cursorCol);
        int sx = getMarginWidth() + (vCol - m_scrollCol);
        int sy = 2 + (m_cursorRow - m_scrollRow);
        m_console.setCursor(sx, sy, true);
    }

    m_console.flush();
}

void Editor::run() {
    while (m_running) {
        adjustScroll();
        updateSyntax();
        render();

        AppEvent event;
        if (m_console.readEvent(event, 500)) {
            handleEvent(event);
        }
    }

    m_console.shutdown();
}
