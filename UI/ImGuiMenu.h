#pragma once

#ifndef IMGUI_MENU_H
#define IMGUI_MENU_H

#include <Windows.h>
#include <string>
#include <functional>
#include <vector>

// ImGui will be included separately
// Download from: https://github.com/ocornut/imgui

namespace MemLib {

    /**
     * ImGuiMenu - Popular game hacking menu system
     * Easy to use, widely adopted in cheat community
     * 
     * Features:
     * - Checkboxes, sliders, buttons
     * - Tabs and windows
     * - Color pickers
     * - Input fields
     * - Auto-integrates with DirectX hook
     */
    class ImGuiMenu {
    public:
        // Menu style
        enum Style {
            STYLE_DARK,    // Dark theme (default)
            STYLE_LIGHT,   // Light theme
            STYLE_CLASSIC, // Classic ImGui
            STYLE_CUSTOM   // User-defined
        };

        // ===== Initialization =====

        // Initialize ImGui (auto-hooks DirectX)
        static bool Initialize();

        // Shutdown ImGui
        static void Shutdown();

        // Check if initialized
        static bool IsInitialized();

        // ===== Menu Control =====

        // Show/hide menu (toggle with key)
        static void SetVisible(bool visible);
        static bool IsVisible();
        static void Toggle();

        // Set toggle key (default: INSERT)
        static void SetToggleKey(int vkey);

        // Set menu style
        static void SetStyle(Style style);

        // ===== Main Menu =====

        // Begin main menu window
        static bool BeginMainMenu(const char* title = "Cheat Menu");
        static void EndMainMenu();

        // ===== Tabs =====

        // Begin tab bar
        static bool BeginTabBar(const char* id);
        static void EndTabBar();

        // Begin tab
        static bool BeginTab(const char* label);
        static void EndTab();

        // ===== Widgets =====

        // Checkbox
        static bool Checkbox(const char* label, bool* value);

        // Button
        static bool Button(const char* label);

        // Slider (int)
        static bool SliderInt(const char* label, int* value, int min, int max);

        // Slider (float)
        static bool SliderFloat(const char* label, float* value, float min, float max);

        // Combo box
        static bool Combo(const char* label, int* current, const char* const items[], int itemsCount);

        // Input text
        static bool InputText(const char* label, char* buffer, size_t bufferSize);

        // Input float
        static bool InputFloat(const char* label, float* value);

        // Input int
        static bool InputInt(const char* label, int* value);

        // Color picker
        static bool ColorPicker(const char* label, float color[4]);

        // ===== Text =====

        // Text label
        static void Text(const char* format, ...);

        // Text (colored)
        static void TextColored(float r, float g, float b, float a, const char* format, ...);

        // Separator
        static void Separator();

        // Spacing
        static void Spacing();

        // ===== Layout =====

        // Same line (put next widget on same line)
        static void SameLine();

        // Columns
        static void Columns(int count);
        static void NextColumn();

        // ===== Custom Drawing =====

        // Get draw list for custom drawing
        static void* GetDrawList();

        // Draw line
        static void DrawLine(float x1, float y1, float x2, float y2, unsigned int color, float thickness = 1.0f);

        // Draw rect
        static void DrawRect(float x, float y, float w, float h, unsigned int color, float thickness = 1.0f);

        // Draw filled rect
        static void DrawRectFilled(float x, float y, float w, float h, unsigned int color);

        // Draw text
        static void DrawText(float x, float y, unsigned int color, const char* text);

        // ===== Example Menus =====

        // Create example aimbot menu
        static void ExampleAimbotMenu(bool& aimbotEnabled, float& aimbotFOV, int& aimbotKey);

        // Create example ESP menu
        static void ExampleESPMenu(bool& espEnabled, bool& espBoxes, bool& espNames, float espColor[4]);

        // Create example misc menu
        static void ExampleMiscMenu(bool& noRecoil, bool& speedhack, float& speedMultiplier);

    private:
        static inline bool s_initialized = false;
        static inline bool s_visible = false;
        static inline int s_toggleKey = VK_INSERT;
        static inline Style s_style = STYLE_DARK;

        // ImGui context (will be created during initialization)
        static inline void* s_imguiContext = nullptr;

        // DirectX state
        static inline void* s_device = nullptr;

        // Input handling
        static void ProcessInput();
        static bool IsKeyPressed(int vkey);

        // Rendering
        static void Render();

        // Styles
        static void ApplyDarkStyle();
        static void ApplyLightStyle();
        static void ApplyClassicStyle();
    };

} // namespace MemLib

#endif // IMGUI_MENU_H
