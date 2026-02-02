#include "IO.h"
#include <thread>
#include <chrono>
#include <cmath>
#include <vector>

namespace WarlockLib {

    // ===== Keyboard Input =====

    void IO::SendKey(int vkey) {
        SendKeyDown(vkey);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        SendKeyUp(vkey);
    }

    void IO::SendKeyDown(int vkey) {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(vkey);
        SendInput(1, &input, sizeof(INPUT));
    }

    void IO::SendKeyUp(int vkey) {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(vkey);
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    void IO::SendText(const std::string& text) {
        for (char c : text) {
            SHORT vk = VkKeyScan(c);
            BYTE keyCode = LOBYTE(vk);
            BYTE shiftState = HIBYTE(vk);

            if (shiftState & 1) {
                SendKeyDown(VK_SHIFT);
            }

            SendKey(keyCode);

            if (shiftState & 1) {
                SendKeyUp(VK_SHIFT);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    bool IO::IsKeyPressed(int vkey) {
        return (GetAsyncKeyState(vkey) & 0x8000) != 0;
    }

    // ===== Mouse Input =====

    void IO::MoveMouse(int x, int y) {
        SetCursorPos(x, y);
    }

    void IO::ClickMouse(int x, int y) {
        MoveMouse(x, y);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        MouseDown(x, y, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        MouseUp(x, y, false);
    }

    void IO::ClickRight(int x, int y) {
        MoveMouse(x, y);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        MouseDown(x, y, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        MouseUp(x, y, true);
    }

    void IO::MouseDown(int x, int y, bool rightButton) {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        
        if (rightButton) {
            input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        } else {
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        }
        
        SendInput(1, &input, sizeof(INPUT));
    }

    void IO::MouseUp(int x, int y, bool rightButton) {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        
        if (rightButton) {
            input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        } else {
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        }
        
        SendInput(1, &input, sizeof(INPUT));
    }

    void IO::GetMousePosition(int& x, int& y) {
        POINT pt;
        GetCursorPos(&pt);
        x = pt.x;
        y = pt.y;
    }

    // ===== Screen Reading =====

    COLORREF IO::GetPixelColor(int x, int y) {
        HDC hdcScreen = GetDC(nullptr);
        if (!hdcScreen) {
            return 0;
        }

        COLORREF color = ::GetPixel(hdcScreen, x, y);
        ReleaseDC(nullptr, hdcScreen);

        return color;
    }

    bool IO::FindColor(COLORREF color, int& outX, int& outY, int tolerance) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        
        return FindColorInRegion(color, 0, 0, screenWidth, screenHeight, outX, outY, tolerance);
    }

    bool IO::FindColorInRegion(COLORREF color, int x, int y, int width, int height, int& outX, int& outY, int tolerance) {
        HDC hdcScreen = GetDC(nullptr);
        if (!hdcScreen) {
            return false;
        }

        for (int py = y; py < y + height; py++) {
            for (int px = x; px < x + width; px++) {
                COLORREF pixelColor = ::GetPixel(hdcScreen, px, py);
                
                if (ColorMatch(color, pixelColor, tolerance)) {
                    outX = px;
                    outY = py;
                    ReleaseDC(nullptr, hdcScreen);
                    return true;
                }
            }
        }

        ReleaseDC(nullptr, hdcScreen);
        return false;
    }

    bool IO::CaptureScreen(const std::string& filename) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        
        return CaptureRegion(0, 0, screenWidth, screenHeight, filename);
    }

    bool IO::CaptureRegion(int x, int y, int width, int height, const std::string& filename) {
        HDC hdcScreen = GetDC(nullptr);
        if (!hdcScreen) {
            return false;
        }

        HDC hdcMemory = CreateCompatibleDC(hdcScreen);
        if (!hdcMemory) {
            ReleaseDC(nullptr, hdcScreen);
            return false;
        }

        HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
        if (!hBitmap) {
            DeleteDC(hdcMemory);
            ReleaseDC(nullptr, hdcScreen);
            return false;
        }

        HGDIOBJ oldBitmap = SelectObject(hdcMemory, hBitmap);
        BitBlt(hdcMemory, 0, 0, width, height, hdcScreen, x, y, SRCCOPY);

        // Save bitmap to file (simplified - real implementation would save as BMP/PNG)
        BITMAP bmp;
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        BITMAPFILEHEADER bfh = {};
        BITMAPINFOHEADER bih = {};

        bih.biSize = sizeof(BITMAPINFOHEADER);
        bih.biWidth = bmp.bmWidth;
        bih.biHeight = bmp.bmHeight;
        bih.biPlanes = 1;
        bih.biBitCount = 32;
        bih.biCompression = BI_RGB;

        DWORD dwBmpSize = ((bmp.bmWidth * bih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
        
        bfh.bfType = 0x4D42; // "BM"
        bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bfh.bfSize = bfh.bfOffBits + dwBmpSize;

        std::vector<BYTE> buffer(dwBmpSize);
        GetDIBits(hdcScreen, hBitmap, 0, bmp.bmHeight, buffer.data(), (BITMAPINFO*)&bih, DIB_RGB_COLORS);

        HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &written, nullptr);
            WriteFile(hFile, &bih, sizeof(BITMAPINFOHEADER), &written, nullptr);
            WriteFile(hFile, buffer.data(), dwBmpSize, &written, nullptr);
            CloseHandle(hFile);
        }

        SelectObject(hdcMemory, oldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMemory);
        ReleaseDC(nullptr, hdcScreen);

        return true;
    }

    int IO::GetScreenWidth() {
        return GetSystemMetrics(SM_CXSCREEN);
    }

    int IO::GetScreenHeight() {
        return GetSystemMetrics(SM_CYSCREEN);
    }

    std::vector<COLORREF> IO::GetScreenPixels(int x, int y, int width, int height) {
        std::vector<COLORREF> pixels;
        pixels.reserve(width * height);

        HDC hdcScreen = GetDC(nullptr);
        if (!hdcScreen) {
            return pixels;
        }

        for (int py = y; py < y + height; py++) {
            for (int px = x; px < x + width; px++) {
                COLORREF color = ::GetPixel(hdcScreen, px, py);
                pixels.push_back(color);
            }
        }

        ReleaseDC(nullptr, hdcScreen);
        return pixels;
    }

    // ===== Utility =====

    bool IO::ColorMatch(COLORREF color1, COLORREF color2, int tolerance) {
        if (tolerance == 0) {
            return color1 == color2;
        }

        int r1 = GetRValue(color1), g1 = GetGValue(color1), b1 = GetBValue(color1);
        int r2 = GetRValue(color2), g2 = GetGValue(color2), b2 = GetBValue(color2);

        int dr = r1 - r2;
        int dg = g1 - g2;
        int db = b1 - b2;

        int distance = static_cast<int>(std::sqrt(dr * dr + dg * dg + db * db));
        return distance <= tolerance;
    }

    COLORREF IO::RGB(unsigned char r, unsigned char g, unsigned char b) {
        return (COLORREF)((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16);
    }

} // namespace WarlockLib
