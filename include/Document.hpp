#pragma once

#include "Common.hpp"
#include <vector>
#include <string>

class Document {
public:
    Document();

    // File Operations
    bool load(const std::wstring& filePath, std::wstring& errorMsg);
    bool save(const std::wstring& filePath, std::wstring& errorMsg);

    // Document state
    bool isModified() const { return m_modified; }
    void setModified(bool modified) { m_modified = modified; }
    bool isNewFile() const { return m_isNewFile; }
    const std::wstring& getFilePath() const { return m_filePath; }
    const std::wstring& getFileName() const { return m_fileName; }
    void setFilePath(const std::wstring& filePath);

    // Line queries
    size_t lineCount() const { return m_lines.size(); }
    const std::wstring& getLine(size_t index) const;
    const std::vector<std::wstring>& getLines() const { return m_lines; }
    void setLines(const std::vector<std::wstring>& lines);

    // Text editing
    void insertChar(int& row, int& col, wchar_t ch);
    void insertNewline(int& row, int& col, bool autoIndent = true);
    void backspace(int& row, int& col);
    void deleteChar(int& row, int& col);
    void deleteLine(int& row, int& col);
    void duplicateLine(int& row, int& col);
    void insertText(int& row, int& col, const std::wstring& text);
    void indentLine(int row);
    void unindentLine(int row, int& col);

    // Auto-pairing and bracket expansion
    void insertPair(int& row, int& col, wchar_t openChar, wchar_t closeChar);
    void insertExpandedBrackets(int& row, int& col);
    void wrapSelection(int startRow, int startCol, int endRow, int endCol, wchar_t openChar, wchar_t closeChar);
    bool isBetweenPair(int row, int col) const;

    // Selection range operations
    std::wstring getSelectedText(int startRow, int startCol, int endRow, int endCol) const;
    void deleteRange(int startRow, int startCol, int endRow, int endCol);

    // Search operations
    bool findNext(const std::wstring& query, int startRow, int startCol,
                  int& foundRow, int& foundCol, bool caseSensitive = false) const;
    bool findPrev(const std::wstring& query, int startRow, int startCol,
                  int& foundRow, int& foundCol, bool caseSensitive = false) const;

private:
    std::vector<std::wstring> m_lines;
    std::wstring m_filePath;
    std::wstring m_fileName = L"[No Name]";
    bool m_modified = false;
    bool m_isNewFile = true;

    void updateFileName();
};
