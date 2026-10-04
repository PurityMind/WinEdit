#pragma once

#include "Common.hpp"
#include <vector>
#include <string>

class Console {
public:
    Console();
    ~Console();

    // Initializes console mode and creates alternate screen buffer
    bool init();

    // Restores original console state
    void shutdown();

    // Dimensions
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    // Screen buffer drawing
    void clear(WORD attr = Color::Gray | Color::BgBlack);
    void setChar(int x, int y, wchar_t ch, WORD attr);
    void setString(int x, int y, const std::wstring& str, WORD attr, int maxLen = -1);
    void fillRect(int x, int y, int w, int h, wchar_t ch, WORD attr);
    void drawBox(int x, int y, int w, int h, const std::wstring& title,
                 WORD borderAttr, WORD fillAttr, WORD titleAttr);

    // Blits backbuffer to the console screen
    void flush();

    // Cursor visibility and position
    void setCursor(int x, int y, bool visible = true);

    // Read input events (blocking or with timeout)
    bool readEvent(AppEvent& outEvent, DWORD timeoutMs = INFINITE);

    // Checks and updates dimensions if window resized
    bool checkResize();

private:
    HANDLE m_hIn = INVALID_HANDLE_VALUE;
    HANDLE m_hOrigOut = INVALID_HANDLE_VALUE;
    HANDLE m_hScreen = INVALID_HANDLE_VALUE;

    DWORD m_origInMode = 0;
    DWORD m_origOutMode = 0;
    bool m_initialized = false;

    int m_width = 80;
    int m_height = 25;
    std::vector<CHAR_INFO> m_buffer;

    void resizeBuffer(int newW, int newH);
};
