#pragma once

#ifndef IO_H
#define IO_H

#include <Windows.h>
#include <string>

namespace WarlockLib {

    /**
     * IO - Input/Output operations
     * Combines keyboard/mouse input simulation and screen reading
     */
    class IO {
    public:
        // ===== Keyboard Input =====
        
        // Send a single key press
        static void SendKey(int vkey);
        
        // Send key down
        static void SendKeyDown(int vkey);
        
        // Send key up
        static void SendKeyUp(int vkey);
        
        // Type text
        static void SendText(const std::string& text);
        
        // Check if key is pressed
        static bool IsKeyPressed(int vkey);
        
        // ===== Mouse Input =====
        
        // Move mouse to position
        static void MoveMouse(int x, int y);
        
        // Click mouse at position
        static void ClickMouse(int x, int y);
        
        // Click right mouse button
        static void ClickRight(int x, int y);
        
        // Mouse down
        static void MouseDown(int x, int y, bool rightButton = false);
        
        // Mouse up
        static void MouseUp(int x, int y, bool rightButton = false);
        
        // Get mouse position
        static void GetMousePosition(int& x, int& y);
        
        // ===== Screen Reading =====
        
        // Get pixel color at position
        static COLORREF GetPixelColor(int x, int y);
        
        // Find color on screen
        static bool FindColor(COLORREF color, int& outX, int& outY, int tolerance = 0);
        
        // Find color in region
        static bool FindColorInRegion(COLORREF color, int x, int y, int width, int height, int& outX, int& outY, int tolerance = 0);
        
        // Capture screenshot
        static bool CaptureScreen(const std::string& filename);
        
        // Capture region
        static bool CaptureRegion(int x, int y, int width, int height, const std::string& filename);
        
        // Get screen dimensions
        static int GetScreenWidth();
        static int GetScreenHeight();
        
        // Get pixel data (for custom analysis)
        static std::vector<COLORREF> GetScreenPixels(int x, int y, int width, int height);
        
        // ===== Utility =====
        
        // Check if colors match within tolerance
        static bool ColorMatch(COLORREF color1, COLORREF color2, int tolerance = 0);
        
        // Convert RGB to COLORREF
        static COLORREF RGB(unsigned char r, unsigned char g, unsigned char b);
    };

} // namespace WarlockLib

#endif // IO_H
