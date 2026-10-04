#include "Document.hpp"
#include <algorithm>
#include <cwctype>

Document::Document() {
    m_lines.push_back(L"");
    m_isNewFile = true;
    m_modified = false;
    updateFileName();
}

void Document::updateFileName() {
    if (m_filePath.empty()) {
        m_fileName = L"[No Name]";
        return;
    }

    size_t lastSlash = m_filePath.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        m_fileName = m_filePath.substr(lastSlash + 1);
    } else {
        m_fileName = m_filePath;
    }
}

void Document::setFilePath(const std::wstring& filePath) {
    m_filePath = filePath;
    updateFileName();
}

const std::wstring& Document::getLine(size_t index) const {
    static const std::wstring empty;
    if (index < m_lines.size()) {
        return m_lines[index];
    }
    return empty;
}

void Document::setLines(const std::vector<std::wstring>& lines) {
    m_lines = lines;
    if (m_lines.empty()) {
        m_lines.push_back(L"");
    }
}

bool Document::load(const std::wstring& filePath, std::wstring& errorMsg) {
    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        errorMsg = L"Failed to open file (Error: " + std::to_wstring(GetLastError()) + L")";
        return false;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        errorMsg = L"Failed to get file size";
        CloseHandle(hFile);
        return false;
    }

    std::string byteBuffer;
    if (fileSize > 0) {
        byteBuffer.resize(fileSize);
        DWORD bytesRead = 0;
        if (!ReadFile(hFile, &byteBuffer[0], fileSize, &bytesRead, NULL)) {
            errorMsg = L"Failed to read file content";
            CloseHandle(hFile);
            return false;
        }
    }
    CloseHandle(hFile);

    m_lines.clear();

    if (byteBuffer.empty()) {
        m_lines.push_back(L"");
    } else {
        // Check for UTF-8 BOM
        size_t offset = 0;
        if (byteBuffer.size() >= 3 &&
            static_cast<unsigned char>(byteBuffer[0]) == 0xEF &&
            static_cast<unsigned char>(byteBuffer[1]) == 0xBB &&
            static_cast<unsigned char>(byteBuffer[2]) == 0xBF) {
            offset = 3;
        }

        int wideLen = MultiByteToWideChar(CP_UTF8, 0, byteBuffer.data() + offset,
                                          static_cast<int>(byteBuffer.size() - offset), NULL, 0);
        std::wstring wideContent;
        if (wideLen > 0) {
            wideContent.resize(wideLen);
            MultiByteToWideChar(CP_UTF8, 0, byteBuffer.data() + offset,
                                static_cast<int>(byteBuffer.size() - offset), &wideContent[0], wideLen);
        }

        // Split into lines
        std::wstring currentLine;
        for (wchar_t ch : wideContent) {
            if (ch == L'\r') {
                continue; // Ignore CR, we treat LF as line separator
            } else if (ch == L'\n') {
                m_lines.push_back(currentLine);
                currentLine.clear();
            } else {
                currentLine.push_back(ch);
            }
        }
        m_lines.push_back(currentLine);
    }

    if (m_lines.empty()) {
        m_lines.push_back(L"");
    }

    m_filePath = filePath;
    updateFileName();
    m_isNewFile = false;
    m_modified = false;
    return true;
}

bool Document::save(const std::wstring& filePath, std::wstring& errorMsg) {
    std::wstring fullText;
    for (size_t i = 0; i < m_lines.size(); ++i) {
        fullText += m_lines[i];
        if (i + 1 < m_lines.size()) {
            fullText += L"\r\n";
        }
    }

    std::string utf8Data;
    if (!fullText.empty()) {
        int utf8Len = WideCharToMultiByte(CP_UTF8, 0, fullText.data(),
                                          static_cast<int>(fullText.size()), NULL, 0, NULL, NULL);
        if (utf8Len > 0) {
            utf8Data.resize(utf8Len);
            WideCharToMultiByte(CP_UTF8, 0, fullText.data(),
                                static_cast<int>(fullText.size()), &utf8Data[0], utf8Len, NULL, NULL);
        }
    }

    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        errorMsg = L"Failed to create file (Error: " + std::to_wstring(GetLastError()) + L")";
        return false;
    }

    DWORD bytesWritten = 0;
    if (!utf8Data.empty()) {
        if (!WriteFile(hFile, utf8Data.data(), static_cast<DWORD>(utf8Data.size()), &bytesWritten, NULL)) {
            errorMsg = L"Failed to write file content";
            CloseHandle(hFile);
            return false;
        }
    }

    CloseHandle(hFile);
    m_filePath = filePath;
    updateFileName();
    m_isNewFile = false;
    m_modified = false;
    return true;
}

void Document::insertChar(int& row, int& col, wchar_t ch) {
    if (row < 0) row = 0;
    if (row >= static_cast<int>(m_lines.size())) row = static_cast<int>(m_lines.size()) - 1;
    if (col < 0) col = 0;
    if (col > static_cast<int>(m_lines[row].size())) col = static_cast<int>(m_lines[row].size());

    m_lines[row].insert(m_lines[row].begin() + col, ch);
    col++;
    m_modified = true;
}

void Document::insertNewline(int& row, int& col, bool autoIndent) {
    if (row < 0) row = 0;
    if (row >= static_cast<int>(m_lines.size())) row = static_cast<int>(m_lines.size()) - 1;
    if (col < 0) col = 0;
    if (col > static_cast<int>(m_lines[row].size())) col = static_cast<int>(m_lines[row].size());

    std::wstring indent;
    if (autoIndent) {
        for (wchar_t ch : m_lines[row]) {
            if (ch == L' ' || ch == L'\t') {
                indent.push_back(ch);
            } else {
                break;
            }
        }
        // Don't auto-indent more than the cursor position
        if (static_cast<int>(indent.size()) > col) {
            indent = indent.substr(0, col);
        }
    }

    std::wstring rightPart = indent + m_lines[row].substr(col);
    m_lines[row] = m_lines[row].substr(0, col);

    m_lines.insert(m_lines.begin() + row + 1, rightPart);
    row++;
    col = static_cast<int>(indent.size());
    m_modified = true;
}

void Document::backspace(int& row, int& col) {
    if (col > 0) {
        if (col <= static_cast<int>(m_lines[row].size())) {
            m_lines[row].erase(m_lines[row].begin() + col - 1);
            col--;
            m_modified = true;
        }
    } else if (row > 0) {
        // Join with previous line
        col = static_cast<int>(m_lines[row - 1].size());
        m_lines[row - 1] += m_lines[row];
        m_lines.erase(m_lines.begin() + row);
        row--;
        m_modified = true;
    }
}

void Document::deleteChar(int& row, int& col) {
    if (row < 0 || row >= static_cast<int>(m_lines.size())) return;

    if (col < static_cast<int>(m_lines[row].size())) {
        m_lines[row].erase(m_lines[row].begin() + col);
        m_modified = true;
    } else if (row + 1 < static_cast<int>(m_lines.size())) {
        // Merge next line into current
        m_lines[row] += m_lines[row + 1];
        m_lines.erase(m_lines.begin() + row + 1);
        m_modified = true;
    }
}

void Document::deleteLine(int& row, int& col) {
    if (m_lines.empty()) return;
    if (row < 0 || row >= static_cast<int>(m_lines.size())) return;

    if (m_lines.size() == 1) {
        m_lines[0].clear();
        col = 0;
    } else {
        m_lines.erase(m_lines.begin() + row);
        if (row >= static_cast<int>(m_lines.size())) {
            row = static_cast<int>(m_lines.size()) - 1;
        }
        if (col > static_cast<int>(m_lines[row].size())) {
            col = static_cast<int>(m_lines[row].size());
        }
    }
    m_modified = true;
}

void Document::duplicateLine(int& row, int& col) {
    (void)col;
    if (row < 0 || row >= static_cast<int>(m_lines.size())) return;
    m_lines.insert(m_lines.begin() + row + 1, m_lines[row]);
    row++;
    m_modified = true;
}

void Document::insertText(int& row, int& col, const std::wstring& text) {
    if (text.empty()) return;

    for (wchar_t ch : text) {
        if (ch == L'\n') {
            insertNewline(row, col, false);
        } else if (ch != L'\r') {
            insertChar(row, col, ch);
        }
    }
    m_modified = true;
}

void Document::indentLine(int row) {
    if (row >= 0 && row < static_cast<int>(m_lines.size())) {
        m_lines[row].insert(0, L"    ");
        m_modified = true;
    }
}

void Document::unindentLine(int row, int& col) {
    if (row < 0 || row >= static_cast<int>(m_lines.size())) return;

    int spaces = 0;
    while (spaces < 4 && spaces < static_cast<int>(m_lines[row].size()) && m_lines[row][spaces] == L' ') {
        spaces++;
    }

    if (spaces > 0) {
        m_lines[row].erase(0, spaces);
        col = (std::max)(0, col - spaces);
        m_modified = true;
    }
}

void Document::insertPair(int& row, int& col, wchar_t openChar, wchar_t closeChar) {
    if (row < 0) row = 0;
    if (row >= static_cast<int>(m_lines.size())) row = static_cast<int>(m_lines.size()) - 1;
    if (col < 0) col = 0;
    if (col > static_cast<int>(m_lines[row].size())) col = static_cast<int>(m_lines[row].size());

    m_lines[row].insert(m_lines[row].begin() + col, closeChar);
    m_lines[row].insert(m_lines[row].begin() + col, openChar);
    col++; // Cursor between open and close characters
    m_modified = true;
}

void Document::insertExpandedBrackets(int& row, int& col) {
    if (row < 0 || row >= static_cast<int>(m_lines.size())) return;
    std::wstring line = m_lines[row];
    if (col <= 0 || col > static_cast<int>(line.size())) return;

    // Detect base indent of the current line
    std::wstring baseIndent;
    for (wchar_t ch : line) {
        if (ch == L' ' || ch == L'\t') {
            baseIndent.push_back(ch);
        } else {
            break;
        }
    }

    std::wstring leftPart = line.substr(0, col);
    std::wstring rightPart = line.substr(col);

    // New middle line: baseIndent + 4 spaces
    std::wstring middleLine = baseIndent + L"    ";
    // Bottom line: baseIndent + rightPart
    std::wstring bottomLine = baseIndent + rightPart;

    m_lines[row] = leftPart;
    m_lines.insert(m_lines.begin() + row + 1, middleLine);
    m_lines.insert(m_lines.begin() + row + 2, bottomLine);

    row++;
    col = static_cast<int>(middleLine.size());
    m_modified = true;
}

void Document::wrapSelection(int startRow, int startCol, int endRow, int endCol,
                             wchar_t openChar, wchar_t closeChar) {
    if (startRow > endRow || (startRow == endRow && startCol > endCol)) {
        std::swap(startRow, endRow);
        std::swap(startCol, endCol);
    }
    if (startRow < 0 || endRow >= static_cast<int>(m_lines.size())) return;

    if (startRow == endRow) {
        int sc = (std::min)(startCol, static_cast<int>(m_lines[startRow].size()));
        int ec = (std::min)(endCol, static_cast<int>(m_lines[startRow].size()));
        m_lines[startRow].insert(m_lines[startRow].begin() + ec, closeChar);
        m_lines[startRow].insert(m_lines[startRow].begin() + sc, openChar);
    } else {
        int sc = (std::min)(startCol, static_cast<int>(m_lines[startRow].size()));
        int ec = (std::min)(endCol, static_cast<int>(m_lines[endRow].size()));
        m_lines[endRow].insert(m_lines[endRow].begin() + ec, closeChar);
        m_lines[startRow].insert(m_lines[startRow].begin() + sc, openChar);
    }
    m_modified = true;
}

bool Document::isBetweenPair(int row, int col) const {
    if (row < 0 || row >= static_cast<int>(m_lines.size())) return false;
    const std::wstring& line = m_lines[row];
    if (col <= 0 || col >= static_cast<int>(line.size())) return false;
    return isMatchingPair(line[col - 1], line[col]);
}

std::wstring Document::getSelectedText(int startRow, int startCol, int endRow, int endCol) const {
    if (startRow > endRow || (startRow == endRow && startCol > endCol)) {
        std::swap(startRow, endRow);
        std::swap(startCol, endCol);
    }

    if (startRow < 0) startRow = 0;
    if (endRow >= static_cast<int>(m_lines.size())) endRow = static_cast<int>(m_lines.size()) - 1;

    if (startRow == endRow) {
        const std::wstring& line = m_lines[startRow];
        int sc = (std::min)(startCol, static_cast<int>(line.size()));
        int ec = (std::min)(endCol, static_cast<int>(line.size()));
        return line.substr(sc, ec - sc);
    }

    std::wstring result;
    const std::wstring& firstLine = m_lines[startRow];
    int sc = (std::min)(startCol, static_cast<int>(firstLine.size()));
    result += firstLine.substr(sc) + L"\n";

    for (int r = startRow + 1; r < endRow; ++r) {
        result += m_lines[r] + L"\n";
    }

    const std::wstring& lastLine = m_lines[endRow];
    int ec = (std::min)(endCol, static_cast<int>(lastLine.size()));
    result += lastLine.substr(0, ec);

    return result;
}

void Document::deleteRange(int startRow, int startCol, int endRow, int endCol) {
    if (startRow > endRow || (startRow == endRow && startCol > endCol)) {
        std::swap(startRow, endRow);
        std::swap(startCol, endCol);
    }

    if (startRow < 0) startRow = 0;
    if (endRow >= static_cast<int>(m_lines.size())) endRow = static_cast<int>(m_lines.size()) - 1;

    if (startRow == endRow) {
        int sc = (std::min)(startCol, static_cast<int>(m_lines[startRow].size()));
        int ec = (std::min)(endCol, static_cast<int>(m_lines[startRow].size()));
        m_lines[startRow].erase(sc, ec - sc);
    } else {
        int sc = (std::min)(startCol, static_cast<int>(m_lines[startRow].size()));
        int ec = (std::min)(endCol, static_cast<int>(m_lines[endRow].size()));

        std::wstring merged = m_lines[startRow].substr(0, sc) + m_lines[endRow].substr(ec);
        m_lines.erase(m_lines.begin() + startRow + 1, m_lines.begin() + endRow + 1);
        m_lines[startRow] = merged;
    }

    if (m_lines.empty()) {
        m_lines.push_back(L"");
    }
    m_modified = true;
}

static std::wstring toLower(const std::wstring& s) {
    std::wstring result = s;
    for (wchar_t& ch : result) {
        ch = towlower(ch);
    }
    return result;
}

bool Document::findNext(const std::wstring& query, int startRow, int startCol,
                        int& foundRow, int& foundCol, bool caseSensitive) const {
    if (query.empty() || m_lines.empty()) return false;

    std::wstring needle = caseSensitive ? query : toLower(query);
    int total = static_cast<int>(m_lines.size());

    // Search rest of startRow
    if (startRow >= 0 && startRow < total) {
        std::wstring line = caseSensitive ? m_lines[startRow] : toLower(m_lines[startRow]);
        if (startCol < static_cast<int>(line.size())) {
            size_t pos = line.find(needle, startCol);
            if (pos != std::wstring::npos) {
                foundRow = startRow;
                foundCol = static_cast<int>(pos);
                return true;
            }
        }
    }

    // Search following lines
    for (int r = startRow + 1; r < total; ++r) {
        std::wstring line = caseSensitive ? m_lines[r] : toLower(m_lines[r]);
        size_t pos = line.find(needle);
        if (pos != std::wstring::npos) {
            foundRow = r;
            foundCol = static_cast<int>(pos);
            return true;
        }
    }

    // Wrap around from beginning to startRow
    for (int r = 0; r <= startRow && r < total; ++r) {
        std::wstring line = caseSensitive ? m_lines[r] : toLower(m_lines[r]);
        size_t maxPos = (r == startRow) ? startCol : std::wstring::npos;
        size_t pos = line.find(needle);
        if (pos != std::wstring::npos && (maxPos == std::wstring::npos || pos < maxPos)) {
            foundRow = r;
            foundCol = static_cast<int>(pos);
            return true;
        }
    }

    return false;
}

bool Document::findPrev(const std::wstring& query, int startRow, int startCol,
                        int& foundRow, int& foundCol, bool caseSensitive) const {
    if (query.empty() || m_lines.empty()) return false;

    std::wstring needle = caseSensitive ? query : toLower(query);
    int total = static_cast<int>(m_lines.size());

    // Search startRow backwards from startCol - 1
    if (startRow >= 0 && startRow < total && startCol > 0) {
        std::wstring line = caseSensitive ? m_lines[startRow] : toLower(m_lines[startRow]);
        size_t pos = line.rfind(needle, (std::max)(0, startCol - 1));
        if (pos != std::wstring::npos) {
            foundRow = startRow;
            foundCol = static_cast<int>(pos);
            return true;
        }
    }

    // Search preceding lines
    for (int r = startRow - 1; r >= 0; --r) {
        std::wstring line = caseSensitive ? m_lines[r] : toLower(m_lines[r]);
        size_t pos = line.rfind(needle);
        if (pos != std::wstring::npos) {
            foundRow = r;
            foundCol = static_cast<int>(pos);
            return true;
        }
    }

    // Wrap around from bottom to startRow
    for (int r = total - 1; r >= startRow; --r) {
        std::wstring line = caseSensitive ? m_lines[r] : toLower(m_lines[r]);
        size_t minPos = (r == startRow) ? startCol : 0;
        size_t pos = line.rfind(needle);
        if (pos != std::wstring::npos && (r > startRow || pos > minPos)) {
            foundRow = r;
            foundCol = static_cast<int>(pos);
            return true;
        }
    }

    return false;
}
