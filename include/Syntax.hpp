#pragma once

#include "Common.hpp"
#include <string>
#include <vector>
#include <unordered_set>

enum class Language {
    None,
    Cpp,
    Python,
    Json,
    Markdown,
    Lua
};

enum class LineState : uint8_t {
    Normal = 0,
    MultiComment = 1,       // /* ... */ in C++
    TripleDoubleString = 2, // """ in Python
    TripleSingleString = 3, // ''' in Python
    LuaMultiComment = 4,    // --[[ ... ]] in Lua
    LuaMultiString = 5      // [[ ... ]] in Lua
};

class SyntaxHighlighter {
public:
    SyntaxHighlighter();

    // Detects language from filename or extension
    static Language detectLanguage(const std::wstring& filename);
    static std::wstring getLanguageName(Language lang);

    // Re-evaluates multiline states across the document
    void updateStates(const std::vector<std::wstring>& lines, Language lang);

    // Highlights a single line into character attributes
    void highlightLine(const std::wstring& line, size_t lineIndex, Language lang,
                       std::vector<WORD>& outAttrs, ThemeType theme = ThemeType::Win98Dark);

private:
    std::vector<LineState> m_lineStates;

    // Keyword & Type lookup sets
    std::unordered_set<std::wstring> m_cppKeywords;
    std::unordered_set<std::wstring> m_cppTypes;
    std::unordered_set<std::wstring> m_pyKeywords;
    std::unordered_set<std::wstring> m_pyBuiltins;
    std::unordered_set<std::wstring> m_jsonConstants;
    std::unordered_set<std::wstring> m_luaKeywords;
    std::unordered_set<std::wstring> m_luaBuiltins;

    void initKeywordSets();

    void highlightCpp(const std::wstring& line, LineState startState,
                      std::vector<WORD>& outAttrs, LineState& endState, ThemeType theme);
    void highlightPython(const std::wstring& line, LineState startState,
                         std::vector<WORD>& outAttrs, LineState& endState, ThemeType theme);
    void highlightJson(const std::wstring& line,
                       std::vector<WORD>& outAttrs, ThemeType theme);
    void highlightMarkdown(const std::wstring& line,
                           std::vector<WORD>& outAttrs, ThemeType theme);
    void highlightLua(const std::wstring& line, LineState startState,
                      std::vector<WORD>& outAttrs, LineState& endState, ThemeType theme);
};
