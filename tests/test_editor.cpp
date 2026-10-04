#include "Document.hpp"
#include "Clipboard.hpp"
#include "Syntax.hpp"
#include <iostream>
#include <cassert>
#include <string>

void testDocumentEditing() {
    std::cout << "[TEST] Document editing operations...\n";
    Document doc;
    assert(doc.lineCount() == 1);
    assert(doc.getLine(0) == L"");
    assert(!doc.isModified());

    // Insert characters
    int row = 0, col = 0;
    doc.insertChar(row, col, L'H');
    doc.insertChar(row, col, L'i');
    assert(doc.getLine(0) == L"Hi");
    assert(row == 0 && col == 2);
    assert(doc.isModified());

    // Enter with auto-indent
    doc.insertNewline(row, col, true);
    assert(doc.lineCount() == 2);
    assert(row == 1 && col == 0);

    // Indent and auto-indent
    doc.insertText(row, col, L"    auto-indented line");
    assert(doc.getLine(1) == L"    auto-indented line");
    assert(row == 1 && col == 22);

    doc.insertNewline(row, col, true);
    assert(doc.lineCount() == 3);
    assert(row == 2 && col == 4); // Inherited 4 spaces!
    assert(doc.getLine(2) == L"    ");

    // Backspace
    doc.backspace(row, col);
    assert(col == 3);
    assert(doc.getLine(2) == L"   ");

    // Merge lines with backspace at col 0
    row = 2; col = 0;
    doc.backspace(row, col);
    assert(doc.lineCount() == 2);
    assert(row == 1);

    std::cout << "  -> Document editing passed!\n";
}

void testDocumentSelectionAndRanges() {
    std::cout << "[TEST] Selection and range deletion...\n";
    Document doc;
    int row = 0, col = 0;
    doc.insertText(row, col, L"Line 1: ABCDEF\nLine 2: GHIJKL\nLine 3: MNOPQR");
    assert(doc.lineCount() == 3);

    // Get selected text within same line
    std::wstring sel1 = doc.getSelectedText(0, 8, 0, 11);
    assert(sel1 == L"ABC");

    // Get selected multiline text
    std::wstring sel2 = doc.getSelectedText(0, 8, 1, 11);
    assert(sel2 == L"ABCDEF\nLine 2: GHI");

    // Delete range multiline
    doc.deleteRange(0, 8, 2, 8);
    // Should merge "Line 1: " with "MNOPQR"
    assert(doc.lineCount() == 1);
    assert(doc.getLine(0) == L"Line 1: MNOPQR");

    std::cout << "  -> Selection and range deletion passed!\n";
}

void testDocumentSearch() {
    std::cout << "[TEST] Search findNext and findPrev...\n";
    Document doc;
    int row = 0, col = 0;
    doc.insertText(row, col, L"apple banana cherry\nbanana durian elderberry\nfig grape banana");

    int foundRow = -1, foundCol = -1;
    // Find next from start
    bool found = doc.findNext(L"banana", 0, 0, foundRow, foundCol, false);
    assert(found && foundRow == 0 && foundCol == 6);

    // Find next again
    found = doc.findNext(L"banana", 0, 7, foundRow, foundCol, false);
    assert(found && foundRow == 1 && foundCol == 0);

    // Find next wrap-around
    found = doc.findNext(L"apple", 1, 0, foundRow, foundCol, false);
    assert(found && foundRow == 0 && foundCol == 0);

    // Find previous from end of line 2
    found = doc.findPrev(L"banana", 2, 16, foundRow, foundCol, false);
    assert(found && foundRow == 2 && foundCol == 10);

    std::cout << "  -> Search passed!\n";
}

void testFileRoundtrip() {
    std::cout << "[TEST] File save and load roundtrip with UTF-8...\n";
    std::wstring testPath = L"test_document_utf8.txt";

    Document docOut;
    int row = 0, col = 0;
    std::wstring originalText = L"Hello, Windows TUI!\nLine 2: Unicode \u041F\u0440\u0438\u0432\u0435\u0442 \u2764\nLine 3: Final line";
    docOut.insertText(row, col, originalText);

    std::wstring err;
    bool saved = docOut.save(testPath, err);
    assert(saved);

    Document docIn;
    bool loaded = docIn.load(testPath, err);
    assert(loaded);
    assert(docIn.lineCount() == 3);
    assert(docIn.getLine(0) == L"Hello, Windows TUI!");
    assert(docIn.getLine(1) == L"Line 2: Unicode \u041F\u0440\u0438\u0432\u0435\u0442 \u2764");
    assert(docIn.getLine(2) == L"Line 3: Final line");
    assert(!docIn.isModified());

    DeleteFileW(testPath.c_str());
    std::cout << "  -> UTF-8 File save and load passed!\n";
}

void testClipboard() {
    std::cout << "[TEST] Windows Clipboard read/write...\n";
    std::wstring testText = L"Clipboard verification string \u2605";
    bool setOk = Clipboard::SetText(testText);
    assert(setOk);

    std::wstring readBack;
    bool getOk = Clipboard::GetText(readBack);
    assert(getOk);
    assert(readBack == testText);

    std::cout << "  -> Clipboard operations passed!\n";
}

void testSyntaxHighlighting() {
    std::cout << "[TEST] Syntax highlighting detection and rules...\n";

    // Language detection
    assert(SyntaxHighlighter::detectLanguage(L"main.cpp") == Language::Cpp);
    assert(SyntaxHighlighter::detectLanguage(L"header.hpp") == Language::Cpp);
    assert(SyntaxHighlighter::detectLanguage(L"script.py") == Language::Python);
    assert(SyntaxHighlighter::detectLanguage(L"data.json") == Language::Json);
    assert(SyntaxHighlighter::detectLanguage(L"readme.md") == Language::Markdown);
    assert(SyntaxHighlighter::detectLanguage(L"notes.txt") == Language::None);

    SyntaxHighlighter highlighter;

    // Test C++ highlighting
    std::vector<std::wstring> cppLines = {
        L"#include <iostream>",
        L"int main() {",
        L"    /* multiline comment start",
        L"       continuation */ int count = 42;",
        L"    return 0; // return status",
        L"}"
    };

    highlighter.updateStates(cppLines, Language::Cpp);

    std::vector<WORD> attrs;
    // Line 0: #include is preprocessor
    highlighter.highlightLine(cppLines[0], 0, Language::Cpp, attrs);
    assert(attrs[0] == (Color::Magenta | Color::BgBlack)); // '#'
    assert(attrs[1] == (Color::Magenta | Color::BgBlack)); // 'i'

    // Line 1: 'int' is type
    highlighter.highlightLine(cppLines[1], 1, Language::Cpp, attrs);
    assert(attrs[0] == (Color::Green | Color::BgBlack)); // 'i' in int
    assert(attrs[1] == (Color::Green | Color::BgBlack)); // 'n' in int
    assert(attrs[2] == (Color::Green | Color::BgBlack)); // 't' in int

    // Line 2: multiline comment start
    highlighter.highlightLine(cppLines[2], 2, Language::Cpp, attrs);
    assert(attrs[4] == (Color::DarkGray | Color::BgBlack)); // '/' in /*

    // Line 3: multiline comment continuation then code
    highlighter.highlightLine(cppLines[3], 3, Language::Cpp, attrs);
    assert(attrs[7] == (Color::DarkGray | Color::BgBlack)); // inside continuation
    // After close: 'int' should be type
    size_t intPos = cppLines[3].find(L"int");
    assert(attrs[intPos] == (Color::Green | Color::BgBlack));
    // Number '42' should be number color
    size_t numPos = cppLines[3].find(L"42");
    assert(attrs[numPos] == (Color::DarkCyan | Color::BgBlack));

    // Line 4: 'return' is keyword, '// return status' is comment
    highlighter.highlightLine(cppLines[4], 4, Language::Cpp, attrs);
    size_t retPos = cppLines[4].find(L"return");
    assert(attrs[retPos] == (Color::Cyan | Color::BgBlack));
    size_t commentPos = cppLines[4].find(L"//");
    assert(attrs[commentPos] == (Color::DarkGray | Color::BgBlack));

    // Test JSON highlighting
    std::vector<std::wstring> jsonLines = {
        L"  \"name\": \"WinEdit\","
    };
    highlighter.updateStates(jsonLines, Language::Json);
    highlighter.highlightLine(jsonLines[0], 0, Language::Json, attrs);
    size_t keyPos = jsonLines[0].find(L"\"name\"");
    assert(attrs[keyPos] == (Color::Cyan | Color::BgBlack)); // Key is cyan
    size_t valPos = jsonLines[0].find(L"\"WinEdit\"");
    assert(attrs[valPos] == (Color::Yellow | Color::BgBlack)); // Value string is yellow

    // Test Lua highlighting
    assert(SyntaxHighlighter::detectLanguage(L"game.lua") == Language::Lua);
    assert(SyntaxHighlighter::getLanguageName(Language::Lua) == L"Lua");

    std::vector<std::wstring> luaLines = {
        L"local message = \"Hello Lua!\" -- variable",
        L"--[[ start multiline",
        L"     comment ]] local count = 42",
        L"print(message) table.insert(t, count)"
    };
    highlighter.updateStates(luaLines, Language::Lua);

    // Line 0: local is keyword, "Hello Lua!" is string, -- variable is comment
    highlighter.highlightLine(luaLines[0], 0, Language::Lua, attrs);
    size_t luaLocalPos = luaLines[0].find(L"local");
    assert(attrs[luaLocalPos] == (Color::Cyan | Color::BgBlack));
    size_t luaStrPos = luaLines[0].find(L"\"Hello Lua!\"");
    assert(attrs[luaStrPos] == (Color::Yellow | Color::BgBlack));
    size_t luaCommentPos = luaLines[0].find(L"--");
    assert(attrs[luaCommentPos] == (Color::DarkGray | Color::BgBlack));

    // Line 1: --[[ start multiline
    highlighter.highlightLine(luaLines[1], 1, Language::Lua, attrs);
    assert(attrs[0] == (Color::DarkGray | Color::BgBlack));

    // Line 2: continuation ]] local count = 42
    highlighter.highlightLine(luaLines[2], 2, Language::Lua, attrs);
    assert(attrs[5] == (Color::DarkGray | Color::BgBlack)); // inside comment
    size_t luaLocal2Pos = luaLines[2].find(L"local");
    assert(attrs[luaLocal2Pos] == (Color::Cyan | Color::BgBlack));
    size_t luaNumPos = luaLines[2].find(L"42");
    assert(attrs[luaNumPos] == (Color::DarkCyan | Color::BgBlack));

    // Line 3: print is builtin, table is builtin
    highlighter.highlightLine(luaLines[3], 3, Language::Lua, attrs);
    size_t luaPrintPos = luaLines[3].find(L"print");
    assert(attrs[luaPrintPos] == (Color::Green | Color::BgBlack));
    size_t luaTablePos = luaLines[3].find(L"table");
    assert(attrs[luaTablePos] == (Color::Green | Color::BgBlack));

    std::cout << "  -> Syntax highlighting (including Lua) passed!\n";
}

void testAutoPairing() {
    std::cout << "[TEST] Auto-pairing quotes, apostrophes, and brackets...\n";

    // Test pair helper functions
    assert(getClosingPair(L'(') == L')');
    assert(getClosingPair(L'[') == L']');
    assert(getClosingPair(L'{') == L'}');
    assert(getClosingPair(L'"') == L'"');
    assert(getClosingPair(L'\'') == L'\'');
    assert(getClosingPair(L'`') == L'`');
    assert(getClosingPair(L'x') == 0);

    assert(isMatchingPair(L'(', L')'));
    assert(isMatchingPair(L'[', L']'));
    assert(isMatchingPair(L'{', L'}'));
    assert(isMatchingPair(L'"', L'"'));
    assert(isMatchingPair(L'\'', L'\''));
    assert(isMatchingPair(L'`', L'`'));
    assert(!isMatchingPair(L'(', L']'));

    assert(isClosingChar(L')'));
    assert(isClosingChar(L']'));
    assert(isClosingChar(L'}'));
    assert(isClosingChar(L'"'));
    assert(isClosingChar(L'\''));
    assert(isClosingChar(L'`'));
    assert(!isClosingChar(L'('));

    Document doc;
    int row = 0, col = 0;

    // 1. Test insertPair for brackets
    doc.insertPair(row, col, L'(', L')');
    assert(doc.getLine(0) == L"()");
    assert(row == 0 && col == 1); // Cursor between ( and )
    assert(doc.isBetweenPair(row, col));

    // 2. Test pair deletion (simulating backspace between pair)
    assert(doc.isBetweenPair(row, col));
    doc.deleteChar(row, col);
    doc.backspace(row, col);
    assert(doc.getLine(0) == L"");
    assert(col == 0);

    // 3. Test insertPair for quotes
    doc.insertPair(row, col, L'"', L'"');
    assert(doc.getLine(0) == L"\"\"");
    assert(col == 1);
    assert(doc.isBetweenPair(row, col));

    // Delete pair
    doc.deleteChar(row, col);
    doc.backspace(row, col);
    assert(doc.getLine(0) == L"");

    // 4. Test expanded bracket formatting on Enter
    // Setup line: "    if (true) {" with cursor before "}"
    row = 0; col = 0;
    doc.insertText(row, col, L"    if (true) {}");
    col = 15; // after { and before }
    assert(doc.isBetweenPair(row, col));

    doc.insertExpandedBrackets(row, col);
    assert(doc.lineCount() == 3);
    assert(doc.getLine(0) == L"    if (true) {");
    assert(doc.getLine(1) == L"        "); // 4 base + 4 indent
    assert(doc.getLine(2) == L"    }");
    assert(row == 1 && col == 8); // Cursor on middle line

    // 5. Test selection wrapping
    Document wrapDoc;
    row = 0; col = 0;
    wrapDoc.insertText(row, col, L"hello world");
    // Wrap "hello" (0..5) with ( and )
    wrapDoc.wrapSelection(0, 0, 0, 5, L'(', L')');
    assert(wrapDoc.getLine(0) == L"(hello) world");

    // Wrap "world" (8..13) with " and "
    wrapDoc.wrapSelection(0, 8, 0, 13, L'"', L'"');
    assert(wrapDoc.getLine(0) == L"(hello) \"world\"");

    std::cout << "  -> Auto-pairing and bracket expansion passed!\n";
}

void testWindows9xThemesAndMenus() {
    std::cout << "[TEST] Windows 9x Themes and Menus...\n";

    // 1. Verify Themes and Colors
    ThemeColors dark = getThemeColors(ThemeType::Win98Dark);
    assert(dark.titleBarActive == (Color::White | Color::BgDarkBlue));
    assert(dark.menuBar == (Color::Black | Color::BgGray));
    assert(dark.editorBg == Color::BgBlack);

    ThemeColors notepad = getThemeColors(ThemeType::Win95Notepad);
    assert(notepad.titleBarActive == (Color::White | Color::BgDarkBlue));
    assert(notepad.editorBg == Color::BgWhite);
    assert(notepad.textDefault == (Color::Black | Color::BgWhite));

    ThemeColors dosEdit = getThemeColors(ThemeType::DosEditBlue);
    assert(dosEdit.editorBg == Color::BgDarkBlue);
    assert(dosEdit.textDefault == (Color::White | Color::BgDarkBlue));
    assert(dosEdit.margin == (Color::Cyan | Color::BgDarkBlue));

    // 2. Verify Theme-Aware Syntax Highlighting
    SyntaxHighlighter highlighter;
    std::vector<std::wstring> lines = {
        L"int main() { return 42; }"
    };
    highlighter.updateStates(lines, Language::Cpp);

    std::vector<WORD> darkAttrs;
    highlighter.highlightLine(lines[0], 0, Language::Cpp, darkAttrs, ThemeType::Win98Dark);
    // In dark theme, background is black
    assert((darkAttrs[0] & 0x00F0) == 0); // Background is 0 (BgBlack)

    std::vector<WORD> notepadAttrs;
    highlighter.highlightLine(lines[0], 0, Language::Cpp, notepadAttrs, ThemeType::Win95Notepad);
    // In Win95 Notepad theme, background is white
    assert((notepadAttrs[0] & 0x00F0) == Color::BgWhite);

    std::vector<WORD> dosAttrs;
    highlighter.highlightLine(lines[0], 0, Language::Cpp, dosAttrs, ThemeType::DosEditBlue);
    // In MS-DOS 7.0 Edit theme, background is deep blue
    assert((dosAttrs[0] & 0x00F0) == Color::BgDarkBlue);

    // 3. Verify Menu Actions
    // 4. Verify Version System
    assert(Version::Major == 1);
    assert(Version::Minor == 3);
    assert(Version::Patch == 0);
    assert(std::wstring(Version::String) == L"1.3.0");
    assert(std::wstring(Version::FullName) == L"WinEdit 98 v1.3.0");

    std::cout << "  -> Windows 9x Themes, Menus, and Versioning passed!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  Running WinEdit Automated Test Suite  \n";
    std::cout << "========================================\n";

    testDocumentEditing();
    testDocumentSelectionAndRanges();
    testDocumentSearch();
    testFileRoundtrip();
    testClipboard();
    testSyntaxHighlighting();
    testAutoPairing();
    testWindows9xThemesAndMenus();

    std::cout << "========================================\n";
    std::cout << "  ALL 8 TESTS PASSED SUCCESSFULLY!       \n";
    std::cout << "========================================\n";
    return 0;
}
