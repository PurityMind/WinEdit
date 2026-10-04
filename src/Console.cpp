#include "Console.hpp"
#include <algorithm>

Console::Console() = default;

Console::~Console() {
    shutdown();
}

bool Console::init() {
    if (m_initialized) return true;

    // Direct console handles
    m_hIn = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (m_hIn == INVALID_HANDLE_VALUE) {
        m_hIn = GetStdHandle(STD_INPUT_HANDLE);
    }

    m_hOrigOut = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (m_hOrigOut == INVALID_HANDLE_VALUE) {
        m_hOrigOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    if (m_hIn == INVALID_HANDLE_VALUE || m_hOrigOut == INVALID_HANDLE_VALUE) {
        return false;
    }

    GetConsoleMode(m_hIn, &m_origInMode);
    GetConsoleMode(m_hOrigOut, &m_origOutMode);

    // Disable line buffering, echo, and quick edit (which freezes console on click)
    // Enable window events and mouse events
    DWORD newInMode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    SetConsoleMode(m_hIn, newInMode);

    // Create alternate screen buffer for seamless teardown
    m_hScreen = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CONSOLE_TEXTMODE_BUFFER,
        NULL
    );

    if (m_hScreen != INVALID_HANDLE_VALUE) {
        SetConsoleActiveScreenBuffer(m_hScreen);
    } else {
        m_hScreen = m_hOrigOut;
    }

    // Determine initial dimensions
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(m_hScreen, &csbi)) {
        m_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        m_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        m_width = 80;
        m_height = 25;
    }

    resizeBuffer(m_width, m_height);
    m_initialized = true;
    return true;
}

void Console::shutdown() {
    if (!m_initialized) return;

    setCursor(0, 0, true);

    if (m_hScreen != INVALID_HANDLE_VALUE && m_hScreen != m_hOrigOut) {
        SetConsoleActiveScreenBuffer(m_hOrigOut);
        CloseHandle(m_hScreen);
        m_hScreen = INVALID_HANDLE_VALUE;
    }

    if (m_hIn != INVALID_HANDLE_VALUE) {
        SetConsoleMode(m_hIn, m_origInMode);
        CloseHandle(m_hIn);
        m_hIn = INVALID_HANDLE_VALUE;
    }

    if (m_hOrigOut != INVALID_HANDLE_VALUE) {
        SetConsoleMode(m_hOrigOut, m_origOutMode);
        CloseHandle(m_hOrigOut);
        m_hOrigOut = INVALID_HANDLE_VALUE;
    }

    m_initialized = false;
}

void Console::resizeBuffer(int newW, int newH) {
    m_width = (std::max)(newW, 20);
    m_height = (std::max)(newH, 10);

    COORD coord = { static_cast<SHORT>(m_width), static_cast<SHORT>(m_height) };
    SetConsoleScreenBufferSize(m_hScreen, coord);

    m_buffer.resize(m_width * m_height);
    clear();
}

bool Console::checkResize() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(m_hScreen, &csbi)) {
        int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        if (w != m_width || h != m_height) {
            resizeBuffer(w, h);
            return true;
        }
    }
    return false;
}

void Console::clear(WORD attr) {
    CHAR_INFO ci;
    ci.Char.UnicodeChar = L' ';
    ci.Attributes = attr;
    std::fill(m_buffer.begin(), m_buffer.end(), ci);
}

void Console::setChar(int x, int y, wchar_t ch, WORD attr) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;
    int idx = y * m_width + x;
    m_buffer[idx].Char.UnicodeChar = ch;
    m_buffer[idx].Attributes = attr;
}

void Console::setString(int x, int y, const std::wstring& str, WORD attr, int maxLen) {
    if (y < 0 || y >= m_height) return;
    int limit = (maxLen >= 0) ? (std::min)(static_cast<int>(str.size()), maxLen)
                              : static_cast<int>(str.size());
    for (int i = 0; i < limit; ++i) {
        int targetX = x + i;
        if (targetX < 0) continue;
        if (targetX >= m_width) break;
        int idx = y * m_width + targetX;
        m_buffer[idx].Char.UnicodeChar = str[i];
        m_buffer[idx].Attributes = attr;
    }
}

void Console::fillRect(int x, int y, int w, int h, wchar_t ch, WORD attr) {
    for (int row = y; row < y + h; ++row) {
        if (row < 0 || row >= m_height) continue;
        for (int col = x; col < x + w; ++col) {
            if (col < 0 || col >= m_width) continue;
            int idx = row * m_width + col;
            m_buffer[idx].Char.UnicodeChar = ch;
            m_buffer[idx].Attributes = attr;
        }
    }
}

void Console::drawBox(int x, int y, int w, int h, const std::wstring& title,
                      WORD borderAttr, WORD fillAttr, WORD titleAttr) {
    if (w < 2 || h < 2) return;

    // Fill interior
    fillRect(x + 1, y + 1, w - 2, h - 2, L' ', fillAttr);

    // Box corners
    setChar(x, y, L'\u250C', borderAttr);                 // ┌
    setChar(x + w - 1, y, L'\u2510', borderAttr);         // ┐
    setChar(x, y + h - 1, L'\u2514', borderAttr);         // └
    setChar(x + w - 1, y + h - 1, L'\u2518', borderAttr); // ┘

    // Horizontal borders
    for (int c = x + 1; c < x + w - 1; ++c) {
        setChar(c, y, L'\u2500', borderAttr);
        setChar(c, y + h - 1, L'\u2500', borderAttr);
    }

    // Vertical borders
    for (int r = y + 1; r < y + h - 1; ++r) {
        setChar(x, r, L'\u2502', borderAttr);
        setChar(x + w - 1, r, L'\u2502', borderAttr);
    }

    // Title if provided
    if (!title.empty() && w > 4) {
        std::wstring formattedTitle = L" " + title + L" ";
        int titleX = x + (w - static_cast<int>(formattedTitle.size())) / 2;
        if (titleX < x + 1) titleX = x + 1;
        setString(titleX, y, formattedTitle, titleAttr, w - 2);
    }
}

void Console::flush() {
    if (!m_initialized) return;
    COORD bufferSize = { static_cast<SHORT>(m_width), static_cast<SHORT>(m_height) };
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, 0, static_cast<SHORT>(m_width - 1), static_cast<SHORT>(m_height - 1) };
    WriteConsoleOutputW(m_hScreen, m_buffer.data(), bufferSize, bufferCoord, &writeRegion);
}

void Console::setCursor(int x, int y, bool visible) {
    if (!m_initialized) return;

    CONSOLE_CURSOR_INFO cci;
    cci.dwSize = 25;
    cci.bVisible = visible ? TRUE : FALSE;
    SetConsoleCursorInfo(m_hScreen, &cci);

    if (visible && x >= 0 && x < m_width && y >= 0 && y < m_height) {
        COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
        SetConsoleCursorPosition(m_hScreen, pos);
    }
}

bool Console::readEvent(AppEvent& outEvent, DWORD timeoutMs) {
    outEvent = AppEvent{};

    while (true) {
        DWORD waitRes = WaitForSingleObject(m_hIn, timeoutMs);
        if (waitRes != WAIT_OBJECT_0) {
            return false;
        }

        DWORD numRead = 0;
        INPUT_RECORD record;
        if (!ReadConsoleInputW(m_hIn, &record, 1, &numRead) || numRead == 0) {
            return false;
        }

        if (record.EventType == WINDOW_BUFFER_SIZE_EVENT) {
            checkResize();
            outEvent.type = EventType::Resize;
            outEvent.newWidth = m_width;
            outEvent.newHeight = m_height;
            return true;
        }

        if (record.EventType == KEY_EVENT) {
            const KEY_EVENT_RECORD& ker = record.Event.KeyEvent;
            if (!ker.bKeyDown) {
                continue; // Skip key release
            }

            // Skip bare modifiers
            switch (ker.wVirtualKeyCode) {
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT:
                case VK_CONTROL:
                case VK_LCONTROL:
                case VK_RCONTROL:
                case VK_MENU: // Alt
                case VK_LMENU:
                case VK_RMENU:
                case VK_CAPITAL:
                case VK_LWIN:
                case VK_RWIN:
                case VK_NUMLOCK:
                case VK_SCROLL:
                    continue;
                default:
                    break;
            }

            outEvent.type = EventType::Key;
            outEvent.key.vkCode = ker.wVirtualKeyCode;
            outEvent.key.ch = ker.uChar.UnicodeChar;
            outEvent.key.ctrl = (ker.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
            outEvent.key.alt = (ker.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0;
            outEvent.key.shift = (ker.dwControlKeyState & SHIFT_PRESSED) != 0;
            outEvent.key.repeatCount = ker.wRepeatCount;
            return true;
        }

        if (record.EventType == MOUSE_EVENT) {
            const MOUSE_EVENT_RECORD& mer = record.Event.MouseEvent;
            bool isWheel = (mer.dwEventFlags & MOUSE_WHEELED) != 0;
            bool isClick = (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;

            if (isWheel || isClick) {
                outEvent.type = EventType::Mouse;
                outEvent.mouse.x = mer.dwMousePosition.X;
                outEvent.mouse.y = mer.dwMousePosition.Y;
                outEvent.mouse.leftButton = isClick;
                if (isWheel) {
                    short delta = static_cast<short>(HIWORD(mer.dwButtonState));
                    outEvent.mouse.wheelDelta = (delta > 0) ? 1 : -1;
                }
                return true;
            }
            continue;
        }
    }
}
