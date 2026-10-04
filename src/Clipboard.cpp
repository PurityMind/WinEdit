#include "Clipboard.hpp"
#include <vector>

namespace Clipboard {

bool SetText(const std::wstring& text) {
    if (!OpenClipboard(NULL)) {
        return false;
    }
    EmptyClipboard();

    // Convert standalone \n to \r\n for standard Windows clipboard consumers
    std::wstring normalized;
    normalized.reserve(text.size() + 32);
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == L'\n' && (i == 0 || text[i - 1] != L'\r')) {
            normalized.push_back(L'\r');
        }
        normalized.push_back(text[i]);
    }

    size_t byteCount = (normalized.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, byteCount);
    if (!hMem) {
        CloseClipboard();
        return false;
    }

    wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
    if (!pMem) {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }

    memcpy(pMem, normalized.c_str(), byteCount);
    GlobalUnlock(hMem);

    if (!SetClipboardData(CF_UNICODETEXT, hMem)) {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}

bool GetText(std::wstring& outText) {
    outText.clear();
    if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        return false;
    }
    if (!OpenClipboard(NULL)) {
        return false;
    }

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (!hData) {
        CloseClipboard();
        return false;
    }

    const wchar_t* pText = static_cast<const wchar_t*>(GlobalLock(hData));
    if (!pText) {
        CloseClipboard();
        return false;
    }

    // Convert \r\n to \n for clean internal document handling
    std::wstring raw(pText);
    GlobalUnlock(hData);
    CloseClipboard();

    outText.reserve(raw.size());
    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == L'\r') {
            if (i + 1 < raw.size() && raw[i + 1] == L'\n') {
                continue; // Skip \r in \r\n
            }
            outText.push_back(L'\n');
        } else {
            outText.push_back(raw[i]);
        }
    }

    return true;
}

} // namespace Clipboard
