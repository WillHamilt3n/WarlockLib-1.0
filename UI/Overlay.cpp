#include "Overlay.h"
#include <d3dx9.h>
#include <cmath>

#pragma comment(lib, "d3dx9.lib")

namespace MemLib {

    // ===== Initialization =====

    bool Overlay::Initialize(DXVersion version) {
        if (s_initialized) {
            return true;
        }

        s_version = version;
        
        if (version == DX_AUTO) {
            s_version = DetectDXVersion();
        }

        bool success = false;
        
        if (s_version == DX_D3D9) {
            success = InitializeD3D9();
        }
        else if (s_version == DX_D3D11) {
            success = InitializeD3D11();
        }

        if (success) {
            s_initialized = true;
            
            // Get screen size
            s_screenWidth = GetSystemMetrics(SM_CXSCREEN);
            s_screenHeight = GetSystemMetrics(SM_CYSCREEN);
        }

        return success;
    }

    void Overlay::Shutdown() {
        if (!s_initialized) {
            return;
        }

        // Release D3D9 resources
        if (s_d3d9Font) {
            s_d3d9Font->Release();
            s_d3d9Font = nullptr;
        }
        if (s_d3d9Line) {
            s_d3d9Line->Release();
            s_d3d9Line = nullptr;
        }

        // Release D3D11 resources
        if (s_d3d11Context) {
            s_d3d11Context->Release();
            s_d3d11Context = nullptr;
        }
        if (s_d3d11Device) {
            s_d3d11Device->Release();
            s_d3d11Device = nullptr;
        }

        s_initialized = false;
    }

    bool Overlay::IsInitialized() {
        return s_initialized;
    }

    Overlay::DXVersion Overlay::GetVersion() {
        return s_version;
    }

    // ===== DirectX Detection =====

    Overlay::DXVersion Overlay::DetectDXVersion() {
        // Try to detect which DirectX version the game uses
        // Check for D3D11 first (more modern)
        
        HMODULE d3d11 = GetModuleHandleA("d3d11.dll");
        if (d3d11) {
            return DX_D3D11;
        }

        HMODULE d3d9 = GetModuleHandleA("d3d9.dll");
        if (d3d9) {
            return DX_D3D9;
        }

        // Default to D3D9
        return DX_D3D9;
    }

    // ===== D3D9 Initialization =====

    bool Overlay::InitializeD3D9() {
        // Hook Present/EndScene
        // In a real implementation, you'd hook these functions
        // For now, this is a framework

        // Create font
        HMODULE d3d9Dll = GetModuleHandleA("d3d9.dll");
        if (!d3d9Dll) {
            return false;
        }

        // Get device (would normally be hooked from Present/EndScene)
        // For framework purposes, mark as initialized
        return true;
    }

    // ===== D3D11 Initialization =====

    bool Overlay::InitializeD3D11() {
        // Hook Present for D3D11
        // Similar to D3D9 but uses IDXGISwapChain::Present

        HMODULE d3d11Dll = GetModuleHandleA("d3d11.dll");
        if (!d3d11Dll) {
            return false;
        }

        // Framework implementation
        return true;
    }

    // ===== Drawing Functions =====

    void Overlay::DrawLine(float x1, float y1, float x2, float y2, const Color& color, float thickness) {
        if (!s_initialized || !s_enabled) return;

        if (s_version == DX_D3D9 && s_d3d9Line && s_d3d9Device) {
            D3DXVECTOR2 points[2] = {
                D3DXVECTOR2(x1, y1),
                D3DXVECTOR2(x2, y2)
            };

            s_d3d9Line->SetWidth(thickness);
            s_d3d9Line->Begin();
            s_d3d9Line->Draw(points, 2, ColorToD3D9(color));
            s_d3d9Line->End();
        }
    }

    void Overlay::DrawBox(float x, float y, float width, float height, const Color& color, float thickness) {
        DrawLine(x, y, x + width, y, color, thickness);                 // Top
        DrawLine(x + width, y, x + width, y + height, color, thickness); // Right
        DrawLine(x + width, y + height, x, y + height, color, thickness); // Bottom
        DrawLine(x, y + height, x, y, color, thickness);                 // Left
    }

    void Overlay::DrawFilledBox(float x, float y, float width, float height, const Color& color) {
        if (!s_initialized || !s_enabled) return;

        if (s_version == DX_D3D9 && s_d3d9Device) {
            D3DRECT rect = { (LONG)x, (LONG)y, (LONG)(x + width), (LONG)(y + height) };
            s_d3d9Device->Clear(1, &rect, D3DCLEAR_TARGET, ColorToD3D9(color), 1.0f, 0);
        }
    }

    void Overlay::DrawCircle(float x, float y, float radius, const Color& color, int segments, float thickness) {
        if (!s_initialized || !s_enabled) return;

        float angle = 0.0f;
        float step = (3.14159f * 2.0f) / segments;

        for (int i = 0; i < segments; i++) {
            float x1 = x + radius * cosf(angle);
            float y1 = y + radius * sinf(angle);
            float x2 = x + radius * cosf(angle + step);
            float y2 = y + radius * sinf(angle + step);

            DrawLine(x1, y1, x2, y2, color, thickness);
            angle += step;
        }
    }

    void Overlay::DrawFilledCircle(float x, float y, float radius, const Color& color, int segments) {
        if (!s_initialized || !s_enabled) return;

        float angle = 0.0f;
        float step = (3.14159f * 2.0f) / segments;

        for (int i = 0; i < segments; i++) {
            float x1 = x;
            float y1 = y;
            float x2 = x + radius * cosf(angle);
            float y2 = y + radius * sinf(angle);
            float x3 = x + radius * cosf(angle + step);
            float y3 = y + radius * sinf(angle + step);

            DrawLine(x1, y1, x2, y2, color, 1.0f);
            DrawLine(x2, y2, x3, y3, color, 1.0f);
            DrawLine(x3, y3, x1, y1, color, 1.0f);

            angle += step;
        }
    }

    void Overlay::DrawText(float x, float y, const std::string& text, const Color& color, int fontSize) {
        if (!s_initialized || !s_enabled) return;

        if (s_version == DX_D3D9 && s_d3d9Font && s_d3d9Device) {
            RECT rect = { (LONG)x, (LONG)y, (LONG)(x + 1000), (LONG)(y + 100) };
            s_d3d9Font->DrawTextA(NULL, text.c_str(), -1, &rect, DT_NOCLIP, ColorToD3D9(color));
        }
    }

    // ===== ESP Helpers =====

    void Overlay::DrawESPBox(float screenX, float screenY, float width, float height, const Color& color, bool filled) {
        if (filled) {
            DrawFilledBox(screenX, screenY, width, height, Color(color.r, color.g, color.b, 0.3f));
        }
        DrawBox(screenX, screenY, width, height, color, 2.0f);
    }

    void Overlay::DrawHealthBar(float x, float y, float width, float height, float healthPercent, const Color& bgColor, const Color& fgColor) {
        // Background
        DrawFilledBox(x, y, width, height, bgColor);

        // Health
        float healthWidth = width * (healthPercent / 100.0f);
        DrawFilledBox(x, y, healthWidth, height, fgColor);

        // Border
        DrawBox(x, y, width, height, Color::White(), 1.0f);
    }

    void Overlay::DrawCrosshair(float x, float y, float size, const Color& color, float thickness) {
        DrawLine(x - size, y, x + size, y, color, thickness);
        DrawLine(x, y - size, x, y + size, color, thickness);
    }

    void Overlay::DrawDistance(float x, float y, float distance, const Color& color) {
        char buffer[32];
        sprintf_s(buffer, "%.1fm", distance);
        DrawText(x, y, buffer, color);
    }

    // ===== 3D Drawing =====

    bool Overlay::WorldToScreen(const float worldPos[3], float& screenX, float& screenY) {
        // Basic world to screen conversion using view matrix
        // Real implementation would use full view-projection matrix

        if (s_viewMatrix[0] == 0.0f) {
            return false; // Matrix not set
        }

        // Transform world position using view matrix
        float clipX = worldPos[0] * s_viewMatrix[0] + worldPos[1] * s_viewMatrix[4] + worldPos[2] * s_viewMatrix[8] + s_viewMatrix[12];
        float clipY = worldPos[0] * s_viewMatrix[1] + worldPos[1] * s_viewMatrix[5] + worldPos[2] * s_viewMatrix[9] + s_viewMatrix[13];
        float clipW = worldPos[0] * s_viewMatrix[3] + worldPos[1] * s_viewMatrix[7] + worldPos[2] * s_viewMatrix[11] + s_viewMatrix[15];

        if (clipW < 0.1f) {
            return false; // Behind camera
        }

        // NDC coordinates
        float ndcX = clipX / clipW;
        float ndcY = clipY / clipW;

        // Screen coordinates
        screenX = (s_screenWidth / 2.0f) * (1.0f + ndcX);
        screenY = (s_screenHeight / 2.0f) * (1.0f - ndcY);

        return true;
    }

    void Overlay::SetViewMatrix(const float matrix[16]) {
        memcpy(s_viewMatrix, matrix, sizeof(float) * 16);
    }

    void Overlay::Draw3DBox(const float worldPos[3], float width, float height, float depth, const Color& color) {
        // Calculate 8 corners of the box
        float corners[8][3];

        corners[0][0] = worldPos[0] - width / 2; corners[0][1] = worldPos[1] - height / 2; corners[0][2] = worldPos[2] - depth / 2;
        corners[1][0] = worldPos[0] + width / 2; corners[1][1] = worldPos[1] - height / 2; corners[1][2] = worldPos[2] - depth / 2;
        corners[2][0] = worldPos[0] + width / 2; corners[2][1] = worldPos[1] + height / 2; corners[2][2] = worldPos[2] - depth / 2;
        corners[3][0] = worldPos[0] - width / 2; corners[3][1] = worldPos[1] + height / 2; corners[3][2] = worldPos[2] - depth / 2;
        corners[4][0] = worldPos[0] - width / 2; corners[4][1] = worldPos[1] - height / 2; corners[4][2] = worldPos[2] + depth / 2;
        corners[5][0] = worldPos[0] + width / 2; corners[5][1] = worldPos[1] - height / 2; corners[5][2] = worldPos[2] + depth / 2;
        corners[6][0] = worldPos[0] + width / 2; corners[6][1] = worldPos[1] + height / 2; corners[6][2] = worldPos[2] + depth / 2;
        corners[7][0] = worldPos[0] - width / 2; corners[7][1] = worldPos[1] + height / 2; corners[7][2] = worldPos[2] + depth / 2;

        // Convert to screen coordinates
        float screenCorners[8][2];
        bool visible[8];

        for (int i = 0; i < 8; i++) {
            visible[i] = WorldToScreen(corners[i], screenCorners[i][0], screenCorners[i][1]);
        }

        // Draw edges
        int edges[12][2] = {
            {0,1}, {1,2}, {2,3}, {3,0}, // Front face
            {4,5}, {5,6}, {6,7}, {7,4}, // Back face
            {0,4}, {1,5}, {2,6}, {3,7}  // Connecting edges
        };

        for (int i = 0; i < 12; i++) {
            int v1 = edges[i][0];
            int v2 = edges[i][1];

            if (visible[v1] && visible[v2]) {
                DrawLine(screenCorners[v1][0], screenCorners[v1][1],
                    screenCorners[v2][0], screenCorners[v2][1], color, 2.0f);
            }
        }
    }

    // ===== Utility =====

    void Overlay::GetScreenSize(int& width, int& height) {
        width = s_screenWidth;
        height = s_screenHeight;
    }

    void Overlay::SetEnabled(bool enabled) {
        s_enabled = enabled;
    }

    bool Overlay::IsEnabled() {
        return s_enabled;
    }

    void Overlay::DrawFPS(float x, float y, const Color& color) {
        UpdateFPS();
        char buffer[32];
        sprintf_s(buffer, "FPS: %.1f", s_fps);
        DrawText(x, y, buffer, color);
    }

    float Overlay::GetFPS() {
        return s_fps;
    }

    void Overlay::UpdateFPS() {
        s_frameCount++;
        DWORD currentTime = GetTickCount();

        if (currentTime - s_lastFrameTime >= 1000) {
            s_fps = s_frameCount * 1000.0f / (currentTime - s_lastFrameTime);
            s_frameCount = 0;
            s_lastFrameTime = currentTime;
        }
    }

    // ===== Custom Drawing =====

    void Overlay::RegisterDrawCallback(DrawCallback callback) {
        s_drawCallbacks.push_back(callback);
    }

    void Overlay::ClearDrawCallbacks() {
        s_drawCallbacks.clear();
    }

    // ===== Helper Functions =====

    D3DCOLOR Overlay::ColorToD3D9(const Color& color) {
        return D3DCOLOR_ARGB(
            (int)(color.a * 255),
            (int)(color.r * 255),
            (int)(color.g * 255),
            (int)(color.b * 255)
        );
    }

} // namespace MemLib
