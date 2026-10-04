#pragma once

#include "Common.hpp"
#include "Console.hpp"
#include "Document.hpp"
#include "Syntax.hpp"
#include <string>
#include <vector>

enum class EditorMode {
    Normal,
    Search,
    SavePrompt,
    GoToLinePrompt,
    HelpModal,
    AboutModal
};

struct UndoSnapshot {
    std::vector<std::wstring> lines;
    int cursorRow = 0;
    int cursorCol = 0;
    bool modified = false;
};

class Editor {
public:
    Editor();

    bool init(const std::wstring& initialFile = L"");
    void run();

    // Theme and options accessors
    void setTheme(ThemeType theme);
    ThemeType getTheme() const { return m_currentTheme; }
    void cycleTheme();

private:
    Console m_console;
    Document m_doc;
    SyntaxHighlighter m_syntax;
    Language m_language = Language::None;

    EditorMode m_mode = EditorMode::Normal;
    bool m_running = true;

    // Windows 9x Theme & View options
    ThemeType m_currentTheme = ThemeType::Win98Dark;
    bool m_showLineNumbers = true;
    bool m_autoPairsEnabled = true;

    // Windows 9x Menus
    std::vector<Menu> m_menus;
    int m_activeMenu = -1; // -1 if closed, 0..N-1 when opened
    int m_selectedMenuItem = 0;

    // Viewport and cursor
    int m_cursorRow = 0;
    int m_cursorCol = 0;
    int m_preferredCol = 0;
    int m_scrollRow = 0;
    int m_scrollCol = 0;

    // Selection
    bool m_hasSelection = false;
    int m_selAnchorRow = 0;
    int m_selAnchorCol = 0;

    // Undo / Redo history
    std::vector<UndoSnapshot> m_undoStack;
    std::vector<UndoSnapshot> m_redoStack;
    bool m_isTypingGroup = false;

    // Search
    std::wstring m_lastSearchQuery;

    // Dialog state (Find, Save As, Go to line)
    std::wstring m_promptLabel;
    std::wstring m_promptInput;
    int m_promptCursor = 0;
    int m_dialogFocus = 0; // 0 = input box, 1 = Action button (e.g. OK/Save/Find), 2 = Cancel

    // Status message
    std::wstring m_statusMessage;
    WORD m_statusColor = Color::Green | Color::BgBlack;
    DWORD m_statusTimestamp = 0;
    bool m_quitConfirmPending = false;

    // Layout
    int getMarginWidth() const;
    int getEditorHeight() const;
    int getEditorWidth() const;

    // Visual column helpers (tab awareness)
    int charToVisualCol(int row, int charCol) const;
    int visualToCharCol(int row, int visualCol) const;

    // Viewport adjustment
    void adjustScroll();
    void updateSyntax();

    // Undo helpers
    void recordUndo(bool isTyping = false);
    void performUndo();
    void performRedo();

    // Selection helpers
    void getSelectionBounds(int& startRow, int& startCol, int& endRow, int& endCol) const;
    bool isCellSelected(int row, int col) const;
    void deleteSelection();
    void selectAll();
    void copyToClipboard();
    void cutToClipboard();
    void pasteFromClipboard();

    // Navigation
    void moveCursor(int dRow, int dCol, bool keepSelection = false);
    void moveWordLeft(bool keepSelection = false);
    void moveWordRight(bool keepSelection = false);
    void moveToLineStart(bool keepSelection = false);
    void moveToLineEnd(bool keepSelection = false);
    void pageUp(bool keepSelection = false);
    void pageDown(bool keepSelection = false);
    void moveToDocStart(bool keepSelection = false);
    void moveToDocEnd(bool keepSelection = false);

    // Menus
    void initMenus();
    void executeMenuAction(int actionId);

    // Event handling
    void handleEvent(const AppEvent& event);
    void handleNormalKey(const KeyEvent& key);
    void handleMenuKey(const KeyEvent& key);
    void handleDialogKey(const KeyEvent& key);
    void handleHelpKey(const KeyEvent& key);
    void handleMouseEvent(const MouseEvent& mouse);

    // Rendering - Windows 9x components
    void render();
    void renderTopBar();
    void renderMenuBar();
    void renderLines();
    void renderScrollbar();
    void renderStatusBar();
    void renderDropDownMenu();
    void renderInputDialog(const std::wstring& title, const std::wstring& label,
                            const std::wstring& actionBtn, const std::wstring& cancelBtn = L"Cancel");
    void renderAboutModal();
    void renderHelpModal();

    // Messaging
    void setStatus(const std::wstring& msg, WORD color = 0);

    // Commands
    void cmdSave();
    void cmdSaveAs();
    void cmdFind();
    void cmdFindNext(bool forward = true);
    void cmdGoToLine();
    void cmdExit();
};
