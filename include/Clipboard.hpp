#pragma once

#include "Common.hpp"
#include <string>

namespace Clipboard {
    // Copies Unicode text to the Windows system clipboard
    bool SetText(const std::wstring& text);

    // Retrieves Unicode text from the Windows system clipboard
    bool GetText(std::wstring& outText);
}
