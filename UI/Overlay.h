#pragma once

#ifndef OVERLAY_H
#define OVERLAY_H

#include <Windows.h>
#include <string>
#include <vector>
#include <functional>
#include <d3d9.h>
#include <d3d11.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3d11.lib")

namespace MemLib {

    /**
     * Overlay - DirectX overlay for ESP (Extra Sensory Perception)
     * Supports both D3D9 and D3D11
     * 
     * Features:
     * - Draw lines, boxes, circles, text
     * - Hook Present/EndScene
     * - FPS counter
     * - Multiple colors and styles
     */
    class Overlay {
    public:
        // Color structure
        struct Color {
            float r, g, b, a;
            
            Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
                : r(red), g(green), b(blue), a(alpha) {}
            
            // Common colors
            static Color White()   { return Color(1.0f, 1.0f, 1.0f, 1.0f); }
            static Color Black()   { return Color(0.0f, 0.0f, 0.0f, 1.0f); }
            static Color Red()     { return Color(1.0f, 0.0f, 0.0f, 1.0f); }
            static Color Green()   { return Color(0.0f, 1.0f, 0.0f, 1.0f); }
            static Color Blue()    { return Color(0.0f, 0.0f, 1.0f, 1.0f); }
            static Color Yellow()  { return Color(1.0f, 1.0f, 0.0f, 1.0f); }
            static Color Cyan()    { return Color(0.0f, 1.0f, 1.0f, 1.0f); }
            static Color Magenta() { return Color(1.0f, 0.0f, 1.0f, 1.0f); }
        };

        // DirectX version
        enum DXVersion {
            DX_AUTO = 0,    // Auto-detect
            DX_D3D9 = 9,    // Direct3D 9
            DX_D3D11 = 11   // Direct3D 11
        };

        // ===== Initialization =====

        // Initialize overlay (hooks DirectX)
        static bool Initialize(DXVersion version = DX_AUTO);

        // Shutdown overlay
        static void Shutdown();

        // Check if initialized
        static bool IsInitialized();

        // Get DirectX version being used
        static DXVersion GetVersion();

        // ===== Drawing Functions =====

        // Draw line
        static void DrawLine(float x1, float y1, float x2, float y2, const Color& color, float thickness = 1.0f);

        // Draw box (outline)
        static void DrawBox(float x, float y, float width, float height, const Color& color, float thickness = 1.0f);

        // Draw filled box
        static void DrawFilledBox(float x, float y, float width, float height, const Color& color);

        // Draw circle (outline)
        static void DrawCircle(float x, float y, float radius, const Color& color, int segments = 32, float thickness = 1.0f);

        // Draw filled circle
        static void DrawFilledCircle(float x, float y, float radius, const Color& color, int segments = 32);

        // Draw text
        static void DrawText(float x, float y, const std::string& text, const Color& color, int fontSize = 16);

        // Draw 3D box (world to screen conversion)
        static void Draw3DBox(const float worldPos[3], float width, float height, float depth, const Color& color);

        // ===== ESP Helpers =====

        // Draw ESP box around entity
        static void DrawESPBox(float screenX, float screenY, float width, float height, const Color& color, bool filled = false);

        // Draw health bar
        static void DrawHealthBar(float x, float y, float width, float height, float healthPercent, const Color& bgColor, const Color& fgColor);

        // Draw crosshair
        static void DrawCrosshair(float x, float y, float size, const Color& color, float thickness = 2.0f);

        // Draw distance text
        static void DrawDistance(float x, float y, float distance, const Color& color);

        // ===== Utility =====

        // World to screen conversion
        static bool WorldToScreen(const float worldPos[3], float& screenX, float& screenY);

        // Set view matrix for 3D drawing
        static void SetViewMatrix(const float matrix[16]);

        // Get screen dimensions
        static void GetScreenSize(int& width, int& height);

        // Enable/disable overlay rendering
        static void SetEnabled(bool enabled);
        static bool IsEnabled();

        // FPS counter
        static void DrawFPS(float x, float y, const Color& color);
        static float GetFPS();

        // ===== Custom Drawing =====

        // Register custom draw callback
        using DrawCallback = std::function<void()>;
        static void RegisterDrawCallback(DrawCallback callback);
        static void ClearDrawCallbacks();

    private:
        static inline bool s_initialized = false;
        static inline bool s_enabled = true;
        static inline DXVersion s_version = DX_AUTO;
        
        // DirectX 9
        static inline IDirect3DDevice9* s_d3d9Device = nullptr;
        static inline ID3DXFont* s_d3d9Font = nullptr;
        static inline ID3DXLine* s_d3d9Line = nullptr;
        
        // DirectX 11
        static inline ID3D11Device* s_d3d11Device = nullptr;
        static inline ID3D11DeviceContext* s_d3d11Context = nullptr;
        
        // View matrix for 3D
        static inline float s_viewMatrix[16] = { 0 };
        
        // Screen size
        static inline int s_screenWidth = 1920;
        static inline int s_screenHeight = 1080;
        
        // FPS tracking
        static inline float s_fps = 0.0f;
        static inline DWORD s_lastFrameTime = 0;
        static inline int s_frameCount = 0;
        
        // Custom callbacks
        static inline std::vector<DrawCallback> s_drawCallbacks;
        
        // Internal initialization
        static bool InitializeD3D9();
        static bool InitializeD3D11();
        static DXVersion DetectDXVersion();
        
        // Hooking
        static bool HookPresent();
        static bool HookEndScene();
        
        // Hook handlers
        static HRESULT WINAPI PresentHook(IDirect3DDevice9* device, CONST RECT* src, CONST RECT* dest, HWND window, CONST RGNDATA* dirtyRegion);
        static HRESULT WINAPI EndSceneHook(IDirect3DDevice9* device);
        static HRESULT WINAPI Present11Hook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
        
        // Drawing internals
        static void RenderD3D9();
        static void RenderD3D11();
        
        // Helper functions
        static void UpdateFPS();
        static D3DCOLOR ColorToD3D9(const Color& color);
    };

} // namespace MemLib

#endif // OVERLAY_H
