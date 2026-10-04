#include "Syntax.hpp"
#include <cwctype>
#include <algorithm>

SyntaxHighlighter::SyntaxHighlighter() {
    initKeywordSets();
}

void SyntaxHighlighter::initKeywordSets() {
    // C / C++ Keywords
    m_cppKeywords = {
        L"auto", L"break", L"case", L"catch", L"class", L"const", L"constexpr",
        L"continue", L"default", L"delete", L"do", L"else", L"enum", L"explicit",
        L"export", L"extern", L"false", L"for", L"friend", L"goto", L"if", L"inline",
        L"mutable", L"namespace", L"new", L"noexcept", L"nullptr", L"operator",
        L"private", L"protected", L"public", L"register", L"reinterpret_cast",
        L"return", L"sizeof", L"static", L"static_assert", L"static_cast",
        L"struct", L"switch", L"template", L"this", L"thread_local", L"throw",
        L"true", L"try", L"typedef", L"typeid", L"typename", L"union", L"using",
        L"virtual", L"volatile", L"while", L"override", L"final", L"concept", L"requires"
    };

    // C / C++ Types
    m_cppTypes = {
        L"bool", L"char", L"char8_t", L"char16_t", L"char32_t", L"double", L"float",
        L"int", L"long", L"short", L"signed", L"unsigned", L"void", L"wchar_t",
        L"size_t", L"int8_t", L"int16_t", L"int32_t", L"int64_t",
        L"uint8_t", L"uint16_t", L"uint32_t", L"uint64_t",
        L"intptr_t", L"uintptr_t", L"ptrdiff_t",
        L"string", L"wstring", L"vector", L"map", L"unordered_map", L"set",
        L"unordered_set", L"pair", L"tuple", L"unique_ptr", L"shared_ptr", L"weak_ptr",
        L"HANDLE", L"DWORD", L"WORD", L"BYTE", L"BOOL", L"HWND", L"HDC", L"LPARAM",
        L"WPARAM", L"LRESULT", L"SHORT", L"CHAR_INFO", L"COORD", L"SMALL_RECT"
    };

    // Python Keywords
    m_pyKeywords = {
        L"and", L"as", L"assert", L"async", L"await", L"break", L"class", L"continue",
        L"def", L"del", L"elif", L"else", L"except", L"finally", L"for", L"from",
        L"global", L"if", L"import", L"in", L"is", L"lambda", L"nonlocal", L"not",
        L"or", L"pass", L"raise", L"return", L"try", L"while", L"with", L"yield",
        L"match", L"case"
    };

    // Python Builtins & Types
    m_pyBuiltins = {
        L"True", L"False", L"None", L"self", L"cls", L"int", L"float", L"str",
        L"bool", L"list", L"dict", L"set", L"tuple", L"bytes", L"print", L"len",
        L"range", L"enumerate", L"zip", L"map", L"filter", L"open", L"isinstance",
        L"type", L"super", L"any", L"all", L"min", L"max", L"sum"
    };

    // JSON Constants
    m_jsonConstants = {
        L"true", L"false", L"null"
    };

    // Lua Keywords
    m_luaKeywords = {
        L"and", L"break", L"do", L"else", L"elseif", L"end",
        L"false", L"for", L"function", L"goto", L"if", L"in",
        L"local", L"nil", L"not", L"or", L"repeat", L"return",
        L"then", L"true", L"until", L"while"
    };

    // Lua Built-ins & Standard Libraries
    m_luaBuiltins = {
        L"assert", L"collectgarbage", L"dofile", L"error", L"getmetatable",
        L"ipairs", L"load", L"loadfile", L"next", L"pairs", L"pcall",
        L"print", L"rawequal", L"rawget", L"rawlen", L"rawset", L"select",
        L"setmetatable", L"tonumber", L"tostring", L"type", L"warn", L"xpcall",
        L"_G", L"_VERSION", L"self",
        L"string", L"table", L"math", L"io", L"os", L"debug", L"package", L"coroutine", L"utf8"
    };
}

Language SyntaxHighlighter::detectLanguage(const std::wstring& filename) {
    size_t dotPos = filename.find_last_of(L'.');
    if (dotPos == std::wstring::npos) {
        return Language::None;
    }

    std::wstring ext = filename.substr(dotPos + 1);
    for (wchar_t& ch : ext) {
        ch = towlower(ch);
    }

    if (ext == L"cpp" || ext == L"hpp" || ext == L"c" || ext == L"h" ||
        ext == L"cc" || ext == L"cxx" || ext == L"hh" || ext == L"hxx" ||
        ext == L"inl" || ext == L"cu") {
        return Language::Cpp;
    }

    if (ext == L"py" || ext == L"pyw" || ext == L"pyx") {
        return Language::Python;
    }

    if (ext == L"json" || ext == L"jsonc" || ext == L"geojson") {
        return Language::Json;
    }

    if (ext == L"md" || ext == L"markdown") {
        return Language::Markdown;
    }

    if (ext == L"lua") {
        return Language::Lua;
    }

    return Language::None;
}

std::wstring SyntaxHighlighter::getLanguageName(Language lang) {
    switch (lang) {
        case Language::Cpp: return L"C++";
        case Language::Python: return L"Python";
        case Language::Json: return L"JSON";
        case Language::Markdown: return L"Markdown";
        case Language::Lua: return L"Lua";
        default: return L"Plain Text";
    }
}

struct SyntaxPalette {
    WORD normal;
    WORD keyword;
    WORD type;
    WORD preproc;
    WORD stringLit;
    WORD number;
    WORD comment;
    WORD decorator;
    WORD header;
    WORD list;
    WORD code;
    WORD key;
};

static SyntaxPalette getSyntaxPalette(ThemeType theme) {
    SyntaxPalette sp{};
    if (theme == ThemeType::Win95Notepad) {
        sp.normal    = Color::Black | Color::BgWhite;
        sp.keyword   = Color::DarkBlue | Color::BgWhite;
        sp.type      = Color::DarkGreen | Color::BgWhite;
        sp.preproc   = Color::DarkMagenta | Color::BgWhite;
        sp.stringLit = Color::DarkRed | Color::BgWhite;
        sp.number    = Color::DarkCyan | Color::BgWhite;
        sp.comment   = Color::DarkGray | Color::BgWhite;
        sp.decorator = Color::DarkMagenta | Color::BgWhite;
        sp.header    = Color::DarkBlue | Color::BgWhite;
        sp.list      = Color::DarkGreen | Color::BgWhite;
        sp.code      = Color::DarkRed | Color::BgWhite;
        sp.key       = Color::DarkBlue | Color::BgWhite;
    } else if (theme == ThemeType::DosEditBlue) {
        sp.normal    = Color::White | Color::BgDarkBlue;
        sp.keyword   = Color::Yellow | Color::BgDarkBlue;
        sp.type      = Color::Green | Color::BgDarkBlue;
        sp.preproc   = Color::Magenta | Color::BgDarkBlue;
        sp.stringLit = Color::Cyan | Color::BgDarkBlue;
        sp.number    = Color::White | Color::BgDarkBlue;
        sp.comment   = Color::DarkGray | Color::BgDarkBlue;
        sp.decorator = Color::Magenta | Color::BgDarkBlue;
        sp.header    = Color::Yellow | Color::BgDarkBlue;
        sp.list      = Color::Cyan | Color::BgDarkBlue;
        sp.code      = Color::Green | Color::BgDarkBlue;
        sp.key       = Color::Yellow | Color::BgDarkBlue;
    } else { // Win98Dark
        sp.normal    = Color::White | Color::BgBlack;
        sp.keyword   = Color::Cyan | Color::BgBlack;
        sp.type      = Color::Green | Color::BgBlack;
        sp.preproc   = Color::Magenta | Color::BgBlack;
        sp.stringLit = Color::Yellow | Color::BgBlack;
        sp.number    = Color::DarkCyan | Color::BgBlack;
        sp.comment   = Color::DarkGray | Color::BgBlack;
        sp.decorator = Color::Magenta | Color::BgBlack;
        sp.header    = Color::Yellow | Color::BgBlack;
        sp.list      = Color::Cyan | Color::BgBlack;
        sp.code      = Color::Green | Color::BgBlack;
        sp.key       = Color::Cyan | Color::BgBlack;
    }
    return sp;
}

void SyntaxHighlighter::updateStates(const std::vector<std::wstring>& lines, Language lang) {
    m_lineStates.resize(lines.size() + 1, LineState::Normal);
    m_lineStates[0] = LineState::Normal;

    LineState currentState = LineState::Normal;
    std::vector<WORD> dummyAttrs;

    for (size_t i = 0; i < lines.size(); ++i) {
        m_lineStates[i] = currentState;
        LineState endState = currentState;

        if (lang == Language::Cpp) {
            highlightCpp(lines[i], currentState, dummyAttrs, endState, ThemeType::Win98Dark);
        } else if (lang == Language::Python) {
            highlightPython(lines[i], currentState, dummyAttrs, endState, ThemeType::Win98Dark);
        } else if (lang == Language::Lua) {
            highlightLua(lines[i], currentState, dummyAttrs, endState, ThemeType::Win98Dark);
        } else {
            endState = LineState::Normal;
        }

        currentState = endState;
    }
    m_lineStates[lines.size()] = currentState;
}

void SyntaxHighlighter::highlightLine(const std::wstring& line, size_t lineIndex, Language lang,
                                     std::vector<WORD>& outAttrs, ThemeType theme) {
    SyntaxPalette pal = getSyntaxPalette(theme);
    outAttrs.assign(line.size(), pal.normal);
    if (line.empty() || lang == Language::None) {
        return;
    }

    LineState startState = LineState::Normal;
    if (lineIndex < m_lineStates.size()) {
        startState = m_lineStates[lineIndex];
    }

    LineState dummyEnd = LineState::Normal;
    switch (lang) {
        case Language::Cpp:
            highlightCpp(line, startState, outAttrs, dummyEnd, theme);
            break;
        case Language::Python:
            highlightPython(line, startState, outAttrs, dummyEnd, theme);
            break;
        case Language::Json:
            highlightJson(line, outAttrs, theme);
            break;
        case Language::Markdown:
            highlightMarkdown(line, outAttrs, theme);
            break;
        case Language::Lua:
            highlightLua(line, startState, outAttrs, dummyEnd, theme);
            break;
        default:
            break;
    }
}

void SyntaxHighlighter::highlightCpp(const std::wstring& line, LineState startState,
                                    std::vector<WORD>& outAttrs, LineState& endState, ThemeType theme) {
    int n = static_cast<int>(line.size());
    SyntaxPalette pal = getSyntaxPalette(theme);
    outAttrs.assign(n, pal.normal);

    const WORD colNormal   = pal.normal;
    const WORD colKeyword  = pal.keyword;
    const WORD colType     = pal.type;
    const WORD colPreproc  = pal.preproc;
    const WORD colString   = pal.stringLit;
    const WORD colNumber   = pal.number;
    const WORD colComment  = pal.comment;

    int i = 0;
    endState = startState;

    // Handle multiline comment continuation from previous line
    if (startState == LineState::MultiComment) {
        size_t closePos = line.find(L"*/");
        if (closePos == std::wstring::npos) {
            std::fill(outAttrs.begin(), outAttrs.end(), colComment);
            endState = LineState::MultiComment;
            return;
        } else {
            for (size_t c = 0; c <= closePos + 1; ++c) {
                outAttrs[c] = colComment;
            }
            i = static_cast<int>(closePos + 2);
            endState = LineState::Normal;
        }
    }

    // Check preprocessor directive
    int firstNonSpace = 0;
    while (firstNonSpace < n && iswspace(line[firstNonSpace])) {
        firstNonSpace++;
    }
    if (firstNonSpace < n && line[firstNonSpace] == L'#') {
        for (int c = firstNonSpace; c < n; ++c) {
            outAttrs[c] = colPreproc;
        }
        return;
    }

    while (i < n) {
        // Single-line comment
        if (line[i] == L'/' && i + 1 < n && line[i + 1] == L'/') {
            for (int c = i; c < n; ++c) {
                outAttrs[c] = colComment;
            }
            break;
        }

        // Multi-line comment start
        if (line[i] == L'/' && i + 1 < n && line[i + 1] == L'*') {
            size_t closePos = line.find(L"*/", i + 2);
            if (closePos == std::wstring::npos) {
                for (int c = i; c < n; ++c) {
                    outAttrs[c] = colComment;
                }
                endState = LineState::MultiComment;
                break;
            } else {
                for (size_t c = i; c <= closePos + 1; ++c) {
                    outAttrs[c] = colComment;
                }
                i = static_cast<int>(closePos + 2);
                continue;
            }
        }

        // Strings and character literals
        if (line[i] == L'"' || line[i] == L'\'') {
            wchar_t quote = line[i];
            outAttrs[i] = colString;
            i++;
            while (i < n) {
                outAttrs[i] = colString;
                if (line[i] == L'\\' && i + 1 < n) {
                    i++;
                    outAttrs[i] = colString;
                } else if (line[i] == quote) {
                    i++;
                    break;
                }
                i++;
            }
            continue;
        }

        // Numbers
        if (iswdigit(line[i]) && (i == 0 || (!iswalnum(line[i - 1]) && line[i - 1] != L'_'))) {
            while (i < n && (iswalnum(line[i]) || line[i] == L'.' || line[i] == L'_')) {
                outAttrs[i] = colNumber;
                i++;
            }
            continue;
        }

        // Identifiers (keywords, types, identifiers)
        if (iswalpha(line[i]) || line[i] == L'_') {
            int start = i;
            while (i < n && (iswalnum(line[i]) || line[i] == L'_')) {
                i++;
            }
            std::wstring word = line.substr(start, i - start);
            WORD attr = colNormal;
            if (m_cppKeywords.count(word)) {
                attr = colKeyword;
            } else if (m_cppTypes.count(word)) {
                attr = colType;
            }

            for (int c = start; c < i; ++c) {
                outAttrs[c] = attr;
            }
            continue;
        }

        outAttrs[i] = colNormal;
        i++;
    }
}

void SyntaxHighlighter::highlightPython(const std::wstring& line, LineState startState,
                                       std::vector<WORD>& outAttrs, LineState& endState, ThemeType theme) {
    int n = static_cast<int>(line.size());
    SyntaxPalette pal = getSyntaxPalette(theme);
    outAttrs.assign(n, pal.normal);

    const WORD colNormal   = pal.normal;
    const WORD colKeyword  = pal.keyword;
    const WORD colType     = pal.type;
    const WORD colString   = pal.stringLit;
    const WORD colNumber   = pal.number;
    const WORD colComment  = pal.comment;
    const WORD colDecorator= pal.decorator;

    int i = 0;
    endState = startState;

    // Check multiline triple quote continuation
    if (startState == LineState::TripleDoubleString || startState == LineState::TripleSingleString) {
        const std::wstring closeSeq = (startState == LineState::TripleDoubleString) ? L"\"\"\"" : L"'''";
        size_t closePos = line.find(closeSeq);
        if (closePos == std::wstring::npos) {
            std::fill(outAttrs.begin(), outAttrs.end(), colString);
            return;
        } else {
            for (size_t c = 0; c <= closePos + 2; ++c) {
                outAttrs[c] = colString;
            }
            i = static_cast<int>(closePos + 3);
            endState = LineState::Normal;
        }
    }

    // Check decorator
    int firstNonSpace = 0;
    while (firstNonSpace < n && iswspace(line[firstNonSpace])) {
        firstNonSpace++;
    }
    if (firstNonSpace < n && line[firstNonSpace] == L'@') {
        for (int c = firstNonSpace; c < n; ++c) {
            outAttrs[c] = colDecorator;
        }
        return;
    }

    while (i < n) {
        // Comment
        if (line[i] == L'#') {
            for (int c = i; c < n; ++c) {
                outAttrs[c] = colComment;
            }
            break;
        }

        // Triple double string
        if (i + 2 < n && line[i] == L'"' && line[i + 1] == L'"' && line[i + 2] == L'"') {
            size_t closePos = line.find(L"\"\"\"", i + 3);
            if (closePos == std::wstring::npos) {
                for (int c = i; c < n; ++c) outAttrs[c] = colString;
                endState = LineState::TripleDoubleString;
                break;
            } else {
                for (size_t c = i; c <= closePos + 2; ++c) outAttrs[c] = colString;
                i = static_cast<int>(closePos + 3);
                continue;
            }
        }

        // Triple single string
        if (i + 2 < n && line[i] == L'\'' && line[i + 1] == L'\'' && line[i + 2] == L'\'') {
            size_t closePos = line.find(L"'''", i + 3);
            if (closePos == std::wstring::npos) {
                for (int c = i; c < n; ++c) outAttrs[c] = colString;
                endState = LineState::TripleSingleString;
                break;
            } else {
                for (size_t c = i; c <= closePos + 2; ++c) outAttrs[c] = colString;
                i = static_cast<int>(closePos + 3);
                continue;
            }
        }

        // Single-line string
        if (line[i] == L'"' || line[i] == L'\'') {
            wchar_t quote = line[i];
            outAttrs[i] = colString;
            i++;
            while (i < n) {
                outAttrs[i] = colString;
                if (line[i] == L'\\' && i + 1 < n) {
                    i++;
                    outAttrs[i] = colString;
                } else if (line[i] == quote) {
                    i++;
                    break;
                }
                i++;
            }
            continue;
        }

        // Numbers
        if (iswdigit(line[i]) && (i == 0 || (!iswalnum(line[i - 1]) && line[i - 1] != L'_'))) {
            while (i < n && (iswalnum(line[i]) || line[i] == L'.' || line[i] == L'_')) {
                outAttrs[i] = colNumber;
                i++;
            }
            continue;
        }

        // Identifiers
        if (iswalpha(line[i]) || line[i] == L'_') {
            int start = i;
            while (i < n && (iswalnum(line[i]) || line[i] == L'_')) {
                i++;
            }
            std::wstring word = line.substr(start, i - start);
            WORD attr = colNormal;
            if (m_pyKeywords.count(word)) {
                attr = colKeyword;
            } else if (m_pyBuiltins.count(word)) {
                attr = colType;
            }

            for (int c = start; c < i; ++c) {
                outAttrs[c] = attr;
            }
            continue;
        }

        outAttrs[i] = colNormal;
        i++;
    }
}

void SyntaxHighlighter::highlightJson(const std::wstring& line, std::vector<WORD>& outAttrs, ThemeType theme) {
    int n = static_cast<int>(line.size());
    SyntaxPalette pal = getSyntaxPalette(theme);
    outAttrs.assign(n, pal.normal);

    const WORD colNormal   = pal.normal;
    const WORD colKey      = pal.key;
    const WORD colString   = pal.stringLit;
    const WORD colNumber   = pal.number;
    const WORD colConst    = pal.preproc;

    int i = 0;
    while (i < n) {
        if (line[i] == L'"') {
            int start = i;
            i++;
            while (i < n) {
                if (line[i] == L'\\' && i + 1 < n) {
                    i += 2;
                } else if (line[i] == L'"') {
                    i++;
                    break;
                } else {
                    i++;
                }
            }

            // Look ahead to check if this string is a key (followed by ':')
            int k = i;
            while (k < n && iswspace(line[k])) k++;
            bool isKey = (k < n && line[k] == L':');

            WORD attr = isKey ? colKey : colString;
            for (int c = start; c < i; ++c) {
                outAttrs[c] = attr;
            }
            continue;
        }

        if (iswdigit(line[i]) || (line[i] == L'-' && i + 1 < n && iswdigit(line[i + 1]))) {
            while (i < n && (iswalnum(line[i]) || line[i] == L'.' || line[i] == L'-' || line[i] == L'+')) {
                outAttrs[i] = colNumber;
                i++;
            }
            continue;
        }

        if (iswalpha(line[i])) {
            int start = i;
            while (i < n && iswalpha(line[i])) {
                i++;
            }
            std::wstring word = line.substr(start, i - start);
            WORD attr = m_jsonConstants.count(word) ? colConst : colNormal;
            for (int c = start; c < i; ++c) {
                outAttrs[c] = attr;
            }
            continue;
        }

        outAttrs[i] = colNormal;
        i++;
    }
}

void SyntaxHighlighter::highlightMarkdown(const std::wstring& line, std::vector<WORD>& outAttrs, ThemeType theme) {
    int n = static_cast<int>(line.size());
    SyntaxPalette pal = getSyntaxPalette(theme);
    outAttrs.assign(n, pal.normal);

    const WORD colHeader = pal.header;
    const WORD colList   = pal.list;
    const WORD colCode   = pal.code;

    int firstNonSpace = 0;
    while (firstNonSpace < n && iswspace(line[firstNonSpace])) {
        firstNonSpace++;
    }
    if (firstNonSpace >= n) return;

    // Headers (# ...)
    if (line[firstNonSpace] == L'#') {
        for (int c = firstNonSpace; c < n; ++c) {
            outAttrs[c] = colHeader;
        }
        return;
    }

    // List bullets (- or * or numbered list)
    if (line[firstNonSpace] == L'-' || line[firstNonSpace] == L'*') {
        outAttrs[firstNonSpace] = colList;
    }

    // Inline code (`...`)
    int i = 0;
    while (i < n) {
        if (line[i] == L'`') {
            size_t closeTick = line.find(L'`', i + 1);
            if (closeTick != std::wstring::npos) {
                for (size_t c = i; c <= closeTick; ++c) {
                    outAttrs[c] = colCode;
                }
                i = static_cast<int>(closeTick + 1);
                continue;
            }
        }
        i++;
    }
}

void SyntaxHighlighter::highlightLua(const std::wstring& line, LineState startState,
                                    std::vector<WORD>& outAttrs, LineState& endState, ThemeType theme) {
    int n = static_cast<int>(line.size());
    SyntaxPalette pal = getSyntaxPalette(theme);
    outAttrs.assign(n, pal.normal);

    const WORD colNormal   = pal.normal;
    const WORD colKeyword  = pal.keyword;
    const WORD colType     = pal.type;
    const WORD colString   = pal.stringLit;
    const WORD colNumber   = pal.number;
    const WORD colComment  = pal.comment;

    int i = 0;
    endState = startState;

    // Check multiline comment continuation (--[[ ... ]])
    if (startState == LineState::LuaMultiComment) {
        size_t closePos = line.find(L"]]");
        if (closePos == std::wstring::npos) {
            std::fill(outAttrs.begin(), outAttrs.end(), colComment);
            endState = LineState::LuaMultiComment;
            return;
        } else {
            for (size_t c = 0; c <= closePos + 1; ++c) {
                outAttrs[c] = colComment;
            }
            i = static_cast<int>(closePos + 2);
            endState = LineState::Normal;
        }
    }

    // Check multiline raw string continuation ([[ ... ]])
    if (startState == LineState::LuaMultiString) {
        size_t closePos = line.find(L"]]");
        if (closePos == std::wstring::npos) {
            std::fill(outAttrs.begin(), outAttrs.end(), colString);
            endState = LineState::LuaMultiString;
            return;
        } else {
            for (size_t c = 0; c <= closePos + 1; ++c) {
                outAttrs[c] = colString;
            }
            i = static_cast<int>(closePos + 2);
            endState = LineState::Normal;
        }
    }

    while (i < n) {
        // Comments: -- or --[[
        if (line[i] == L'-' && i + 1 < n && line[i + 1] == L'-') {
            // Check for multiline comment --[[
            if (i + 3 < n && line[i + 2] == L'[' && line[i + 3] == L'[') {
                size_t closePos = line.find(L"]]", i + 4);
                if (closePos == std::wstring::npos) {
                    for (int c = i; c < n; ++c) {
                        outAttrs[c] = colComment;
                    }
                    endState = LineState::LuaMultiComment;
                    break;
                } else {
                    for (size_t c = i; c <= closePos + 1; ++c) {
                        outAttrs[c] = colComment;
                    }
                    i = static_cast<int>(closePos + 2);
                    continue;
                }
            } else {
                // Single-line comment
                for (int c = i; c < n; ++c) {
                    outAttrs[c] = colComment;
                }
                break;
            }
        }

        // Multiline string literal [[ ... ]]
        if (line[i] == L'[' && i + 1 < n && line[i + 1] == L'[') {
            size_t closePos = line.find(L"]]", i + 2);
            if (closePos == std::wstring::npos) {
                for (int c = i; c < n; ++c) {
                    outAttrs[c] = colString;
                }
                endState = LineState::LuaMultiString;
                break;
            } else {
                for (size_t c = i; c <= closePos + 1; ++c) {
                    outAttrs[c] = colString;
                }
                i = static_cast<int>(closePos + 2);
                continue;
            }
        }

        // Single-line string literals '...' or "..."
        if (line[i] == L'"' || line[i] == L'\'') {
            wchar_t quote = line[i];
            outAttrs[i] = colString;
            i++;
            while (i < n) {
                outAttrs[i] = colString;
                if (line[i] == L'\\' && i + 1 < n) {
                    i++;
                    outAttrs[i] = colString;
                } else if (line[i] == quote) {
                    i++;
                    break;
                }
                i++;
            }
            continue;
        }

        // Numbers (hex, float, decimal)
        if (iswdigit(line[i]) && (i == 0 || (!iswalnum(line[i - 1]) && line[i - 1] != L'_'))) {
            while (i < n && (iswalnum(line[i]) || line[i] == L'.' || line[i] == L'_')) {
                outAttrs[i] = colNumber;
                i++;
            }
            continue;
        }

        // Identifiers (keywords, built-ins, standard libraries)
        if (iswalpha(line[i]) || line[i] == L'_') {
            int start = i;
            while (i < n && (iswalnum(line[i]) || line[i] == L'_')) {
                i++;
            }
            std::wstring word = line.substr(start, i - start);
            WORD attr = colNormal;
            if (m_luaKeywords.count(word)) {
                attr = colKeyword;
            } else if (m_luaBuiltins.count(word)) {
                attr = colType;
            }

            for (int c = start; c < i; ++c) {
                outAttrs[c] = attr;
            }
            continue;
        }

        outAttrs[i] = colNormal;
        i++;
    }
}

