#pragma once

#ifndef XAML_MENU_H
#define XAML_MENU_H

#include <Windows.h>
#include <string>
#include <functional>
#include <map>
#include <vector>

// For WPF/UWP support
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>

namespace MemLib {

    /**
     * XAMLMenu - Modern XAML-based cheat UI
     * Uses WPF/UWP for beautiful, native Windows UI
     * 
     * Features:
     * - Modern, native Windows look
     * - Can be overlay or separate window
     * - Data binding support
     * - Animations and transitions
     * - Touch support
     */
    class XAMLMenu {
    public:
        // Window mode
        enum WindowMode {
            MODE_OVERLAY,   // Transparent overlay on game
            MODE_WINDOW,    // Separate window
            MODE_BORDERLESS // Borderless window
        };

        // Control types
        enum ControlType {
            CONTROL_CHECKBOX,
            CONTROL_SLIDER,
            CONTROL_BUTTON,
            CONTROL_TEXTBOX,
            CONTROL_COMBOBOX,
            CONTROL_TOGGLE,
            CONTROL_COLORPICKER
        };

        // Control definition
        struct Control {
            std::string id;
            ControlType type;
            std::string label;
            void* value;           // Pointer to bool/int/float/string
            float min = 0.0f;      // For sliders
            float max = 100.0f;    // For sliders
            std::vector<std::string> items; // For comboboxSo

            std::function<void()> onClick; // For buttons
        };

        // Tab definition
        struct Tab {
            std::string name;
            std::vector<Control> controls;
        };

        // ===== Initialization =====

        // Initialize XAML menu
        static bool Initialize(WindowMode mode = MODE_OVERLAY);

        // Shutdown menu
        static void Shutdown();

        // Check if initialized
        static bool IsInitialized();

        // ===== Window Control =====

        // Show/hide menu
        static void SetVisible(bool visible);
        static bool IsVisible();
        static void Toggle();

        // Set window size
        static void SetSize(int width, int height);

        // Set window position
        static void SetPosition(int x, int y);

        // Set window opacity
        static void SetOpacity(float opacity);

        // ===== Menu Building =====

        // Add tab
        static void AddTab(const std::string& name);

        // Set current tab
        static void SetCurrentTab(const std::string& name);

        // Add checkbox
        static void AddCheckbox(const std::string& label, bool* value);

        // Add slider (int)
        static void AddSliderInt(const std::string& label, int* value, int min, int max);

        // Add slider (float)
        static void AddSliderFloat(const std::string& label, float* value, float min, float max);

        // Add button
        static void AddButton(const std::string& label, std::function<void()> onClick);

        // Add text input
        static void AddTextInput(const std::string& label, std::string* value);

        // Add combo box
        static void AddComboBox(const std::string& label, int* selected, const std::vector<std::string>& items);

        // Add toggle switch
        static void AddToggle(const std::string& label, bool* value);

        // Add color picker
        static void AddColorPicker(const std::string& label, float color[4]);

        // Add separator
        static void AddSeparator();

        // Add text label
        static void AddLabel(const std::string& text);

        // ===== Styling =====

        // Set theme (Acrylic, Dark, Light)
        static void SetTheme(const std::string& theme);

        // Set accent color
        static void SetAccentColor(unsigned char r, unsigned char g, unsigned char b);

        // Enable acrylic blur
        static void SetAcrylicBlur(bool enabled);

        // ===== Example Menus =====

        // Create example cheat menu
        static void CreateExampleMenu();

        // Aimbot tab
        static void CreateAimbotTab(bool& enabled, float& fov, int& key, bool& smoothing);

        // ESP tab
        static void CreateESPTab(bool& enabled, bool& boxes, bool& names, bool& health);

        // Misc tab
        static void CreateMiscTab(bool& noRecoil, bool& speedhack, float& speed);

        // Settings tab
        static void CreateSettingsTab();

    private:
        static inline bool s_initialized = false;
        static inline bool s_visible = false;
        static inline WindowMode s_mode = MODE_OVERLAY;
        
        static inline HWND s_window = nullptr;
        static inline int s_width = 400;
        static inline int s_height = 600;
        static inline float s_opacity = 0.95f;

        static inline std::vector<Tab> s_tabs;
        static inline std::string s_currentTab;

        // Window proc
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

        // Rendering
        static void Render();
        static void RenderTab(const Tab& tab);
        static void RenderControl(const Control& control);

        // Input handling
        static void ProcessInput();

        // XAML helpers
        static void* CreateXAMLElement(ControlType type);
        static void UpdateXAMLElement(void* element, const Control& control);
    };

} // namespace MemLib

#endif // XAML_MENU_H
