#pragma once

#ifndef SIMPLE_CHEAT_H
#define SIMPLE_CHEAT_H

#include "MemoryLib.h"
#include <memory>

namespace WarlockLib {

    /**
     * SimpleCheat - One-liner functions for common cheat operations
     * Perfect for beginners who want quick results
     * 
     * Note: WarlockLib class name kept for backwards compatibility
     */
    class WarlockLib {
    private:
        static inline std::unique_ptr<Process> s_activeProcess = nullptr;
        static inline HANDLE s_activeHandle = nullptr;
        static inline std::unique_ptr<Driver> s_activeDriver = nullptr;
        static inline std::unique_ptr<Freezer> s_freezer = nullptr;
        static inline bool s_useDriver = false;
        static inline bool s_useDMA = false;
        static inline DWORD s_dmaProcessId = 0;

    public:
        // ===== Driver Mode =====

        // Attach using kernel driver (bypasses anti-cheat)
        static bool AttachWithDriver(const std::wstring& processName, const std::wstring& driverName) {
            s_activeDriver = std::make_unique<Driver>();
            if (!s_activeDriver->Connect(driverName)) {
                s_activeDriver.reset();
                return false;
            }

            // Find process ID
            auto pid = Process::FindProcessId(processName);
            if (!pid.has_value()) {
                s_activeDriver.reset();
                return false;
            }

            s_activeDriver->SetTargetProcess(pid.value());
            s_useDriver = true;
            return true;
        }

        // ===== One-Liner Process Attachment =====
        
        // Attach to process and return true if successful
        static bool Attach(const std::wstring& processName) {
            s_activeProcess = std::make_unique<Process>();
            if (s_activeProcess->Attach(processName)) {
                s_activeHandle = s_activeProcess->GetHandle();
                return true;
            }
            s_activeProcess.reset();
            s_activeHandle = nullptr;
            return false;
        }

        // Detach from process
        static void Detach() {
            s_activeProcess.reset();
            s_activeHandle = nullptr;
            s_activeDriver.reset();
            s_freezer.reset();
            s_useDriver = false;
            s_useDMA = false;
            s_dmaProcessId = 0;
        }

        // ===== DMA Mode (Hardware-level access) =====

        // Attach using DMA (hardware bypass - completely undetectable!)
        static bool AttachWithDMA(const std::wstring& processName) {
            if (!DMA::Initialize()) {
                return false;
            }

            s_dmaProcessId = DMA::FindProcess(processName);
            if (s_dmaProcessId == 0) {
                return false;
            }

            s_useDMA = true;
            return true;
        }

        // Check if using DMA mode
        static bool IsUsingDMA() {
            return s_useDMA;
        }

        // ===== One-Liner Memory Operations =====

        // Read any type from address in attached process
        template<typename T>
        static T Read(uintptr_t address) {
            if (s_useDriver && s_activeDriver) {
                auto result = s_activeDriver->Read<T>(address);
                return result.value_or(T{});
            }
            auto result = Memory::Read<T>(s_activeHandle, address);
            return result.value_or(T{});
        }

        // Write any type to address in attached process
        template<typename T>
        static bool Write(uintptr_t address, T value) {
            if (s_useDriver && s_activeDriver) {
                return s_activeDriver->Write<T>(address, value);
            }
            return Memory::Write<T>(s_activeHandle, address, value);
        }

        // Read from module + offset
        template<typename T>
        static T ReadModule(const std::wstring& moduleName, uintptr_t offset) {
            // DMA mode
            if (s_useDMA) {
                auto baseAddr = DMA::GetModuleBase(s_dmaProcessId, moduleName);
                if (baseAddr.has_value()) {
                    auto result = DMA::Read<T>(s_dmaProcessId, baseAddr.value() + offset);
                    return result.value_or(T{});
                }
                return T{};
            }

            // Driver mode
            if (s_useDriver && s_activeDriver) {
                uintptr_t base = GetModuleBase(moduleName);
                auto result = s_activeDriver->Read<T>(base + offset);
                return result.value_or(T{});
            }

            // User mode
            if (s_activeProcess) {
                uintptr_t base = GetModuleBase(moduleName);
                auto result = Memory::Read<T>(s_activeHandle, base + offset);
                return result.value_or(T{});
            }

            return T{};
        }

        // Write to module + offset
        template<typename T>
        static bool WriteModule(const std::wstring& moduleName, uintptr_t offset, T value) {
            // DMA mode
            if (s_useDMA) {
                auto baseAddr = DMA::GetModuleBase(s_dmaProcessId, moduleName);
                if (baseAddr.has_value()) {
                    return DMA::Write<T>(s_dmaProcessId, baseAddr.value() + offset, value);
                }
                return false;
            }

            // Driver mode
            if (s_useDriver && s_activeDriver) {
                uintptr_t base = GetModuleBase(moduleName);
                return s_activeDriver->Write<T>(base + offset, value);
            }

            // User mode
            if (s_activeProcess) {
                uintptr_t base = GetModuleBase(moduleName);
                return Memory::Write<T>(s_activeHandle, base + offset, value);
            }

            return false;
        }

        // ===== One-Liner Pointer Chains =====

        // Follow pointer chain and read final value
        template<typename T>
        static T ReadPointer(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets) {
            auto finalAddr = Memory::FollowPointerChain(s_activeHandle, baseAddress, offsets);
            if (finalAddr.has_value()) {
                return Read<T>(finalAddr.value());
            }
            return T{};
        }

        // Follow pointer chain from module base
        template<typename T>
        static T ReadModulePointer(const std::wstring& moduleName, const std::vector<uintptr_t>& offsets) {
            uintptr_t base = s_activeProcess->GetModuleBaseAddress(moduleName);
            return ReadPointer<T>(base, offsets);
        }

        // ===== One-Liner Pattern Scanning =====

        // Find pattern in module
        static uintptr_t FindPattern(const std::wstring& moduleName, const std::string& pattern) {
            PatternScanner scanner(s_activeHandle);
            auto result = scanner.FindPatternInModule(moduleName, pattern);
            return result.value_or(0);
        }

        // Find pattern from module base with size
        static uintptr_t FindPatternInRange(uintptr_t startAddress, size_t size, const std::string& pattern) {
            PatternScanner scanner(s_activeHandle);
            auto result = scanner.FindPattern(startAddress, size, pattern);
            return result.value_or(0);
        }

        // ===== One-Liner Module Info =====

        static uintptr_t GetModuleBase(const std::wstring& moduleName) {
            if (s_useDriver && s_activeDriver) {
                auto base = s_activeDriver->GetModuleBase(moduleName);
                return base.value_or(0);
            }
            return s_activeProcess->GetModuleBaseAddress(moduleName);
        }

        static uintptr_t GetModuleSize(const std::wstring& moduleName) {
            if (s_useDriver && s_activeDriver) {
                auto size = s_activeDriver->GetModuleSize(moduleName);
                return size.value_or(0);
            }
            auto info = s_activeProcess->GetModuleInfo(moduleName);
            return info.has_value() ? info->modBaseSize : 0;
        }

        // ===== Internal (DLL Injection) Helpers =====

        // Read internal memory
        template<typename T>
        static T ReadInternal(uintptr_t address) {
            return Memory::ReadInternal<T>(address);
        }

        // Write internal memory
        template<typename T>
        static void WriteInternal(uintptr_t address, T value) {
            Memory::WriteInternal<T>(address, value);
        }

        // Find pattern internally
        static uintptr_t FindPatternInternal(uintptr_t startAddress, size_t size, const std::string& pattern) {
            auto result = PatternScanner::FindPatternInternal(startAddress, size, pattern);
            return result.value_or(0);
        }

        // ===== One-Liner Hooking =====

        // Create and install inline hook (returns hook object, keep it alive!)
        static std::unique_ptr<Hook::InlineHook> HookFunction(uintptr_t targetAddress, uintptr_t hookAddress) {
            auto hook = std::make_unique<Hook::InlineHook>(targetAddress, hookAddress);
            if (hook->Install()) {
                return hook;
            }
            return nullptr;
        }

        // NOP out bytes at address
        static bool Nop(uintptr_t address, size_t count) {
            return Hook::NopBytes(address, count);
        }

        // ===== Utility Functions =====

        // Allocate memory in target process
        static uintptr_t Alloc(size_t size) {
            return Memory::AllocateMemory(s_activeHandle, size);
        }

        // Free allocated memory
        static bool Free(uintptr_t address) {
            return Memory::FreeMemory(s_activeHandle, address);
        }

        // Check if attached
        static bool IsAttached() {
            return s_activeHandle != nullptr || (s_useDriver && s_activeDriver);
        }

        // Get current process handle
        static HANDLE GetHandle() {
            return s_activeHandle;
        }

        // Get driver instance (if using driver mode)
        static Driver* GetDriver() {
            return s_activeDriver.get();
        }

        // Check if using driver mode
        static bool IsUsingDriver() {
            return s_useDriver;
        }

        // ===== Memory Freezer Functions =====

        // Freeze a value at an address (continuously writes it)
        template<typename T>
        static bool Freeze(uintptr_t address, T value, DWORD intervalMs = 100) {
            if (!s_freezer) {
                s_freezer = std::make_unique<Freezer>();
            }

            if (s_useDriver && s_activeDriver) {
                return s_freezer->Freeze<T>(address, value, intervalMs, nullptr, true, s_activeDriver.get());
            }
            else if (s_activeHandle) {
                return s_freezer->Freeze<T>(address, value, intervalMs, s_activeHandle, false, nullptr);
            }
            else {
                // Internal mode (no handle)
                return s_freezer->Freeze<T>(address, value, intervalMs, nullptr, false, nullptr);
            }
        }

        // Freeze a value at module + offset
        template<typename T>
        static bool FreezeModule(const std::wstring& moduleName, uintptr_t offset, T value, DWORD intervalMs = 100) {
            uintptr_t base = GetModuleBase(moduleName);
            return Freeze<T>(base + offset, value, intervalMs);
        }

        // Unfreeze a specific address
        static bool Unfreeze(uintptr_t address) {
            if (s_freezer) {
                return s_freezer->Unfreeze(address);
            }
            return false;
        }

        // Unfreeze all addresses
        static void UnfreezeAll() {
            if (s_freezer) {
                s_freezer->UnfreezeAll();
            }
        }

        // Check if address is frozen
        static bool IsFrozen(uintptr_t address) {
            if (s_freezer) {
                return s_freezer->IsFrozen(address);
            }
            return false;
        }

        // Get count of frozen values
        static size_t GetFrozenCount() {
            if (s_freezer) {
                return s_freezer->GetFrozenCount();
            }
            return 0;
        }

        // ===== Code Cave Finder Functions =====

        // Find a code cave in a module
        static uintptr_t FindCodeCave(const std::wstring& moduleName, size_t minSize) {
            if (s_activeHandle) {
                auto result = CodeCave::FindInModule(s_activeHandle, moduleName, minSize);
                return result.value_or(0);
            }
            else {
                // Internal mode
                auto result = CodeCave::FindInModuleInternal(moduleName, minSize);
                return result.value_or(0);
            }
        }

        // Find code cave in a memory range
        static uintptr_t FindCodeCaveInRange(uintptr_t startAddress, size_t rangeSize, size_t minSize) {
            if (s_activeHandle) {
                auto result = CodeCave::FindInRange(s_activeHandle, startAddress, rangeSize, minSize);
                return result.value_or(0);
            }
            else {
                auto result = CodeCave::FindInRangeInternal(startAddress, rangeSize, minSize);
                return result.value_or(0);
            }
        }

        // Find all code caves in a module
        static std::vector<uintptr_t> FindAllCodeCaves(const std::wstring& moduleName, size_t minSize, size_t maxResults = 10) {
            if (s_activeHandle) {
                return CodeCave::FindAllInModule(s_activeHandle, moduleName, minSize, maxResults);
            }
            return {};
        }

        // Find code cave near an address (for relative jumps)
        static uintptr_t FindCodeCaveNear(uintptr_t address, size_t minSize, size_t maxDistance = 0x7FFFFFFF) {
            if (s_activeHandle) {
                auto result = CodeCave::FindNear(s_activeHandle, address, minSize, maxDistance);
                return result.value_or(0);
            }
            return 0;
        }

        // Verify if a code cave is still usable
        static bool VerifyCodeCave(uintptr_t address, size_t size) {
            if (s_activeHandle) {
                return CodeCave::VerifyCave(s_activeHandle, address, size);
            }
            else {
                return CodeCave::VerifyCaveInternal(address, size);
            }
        }

        // ===== Input Simulation Functions =====

        // Keyboard
        static bool SendKey(WORD virtualKey) {
            return Input::SendKeyPress(virtualKey);
        }

        static bool SendText(const std::string& text) {
            return Input::SendText(text);
        }

        // Mouse
        static bool MoveMouse(int x, int y) {
            return Input::MoveMouse(x, y);
        }

        static bool ClickMouse(int x = -1, int y = -1) {
            return Input::ClickLeft(x, y);
        }

        static bool ClickRight(int x = -1, int y = -1) {
            return Input::ClickRight(x, y);
        }

        // ===== Screen Reading Functions =====

        // Get pixel color at position
        static COLORREF GetPixelColor(int x, int y) {
            auto color = Screen::GetPixelColor(x, y);
            return color.value_or(0);
        }

        // Check if colors match (with tolerance)
        static bool IsColorMatch(COLORREF color1, COLORREF color2, int tolerance = 10) {
            return Screen::IsColorSimilar(color1, color2, tolerance);
        }

        // Find color on screen
        static bool FindColor(COLORREF targetColor, int& outX, int& outY, int tolerance = 10) {
            auto result = Screen::FindColor(targetColor, tolerance);
            if (result.has_value()) {
                outX = result->x;
                outY = result->y;
                return true;
            }
            return false;
        }

        // Capture screenshot
        static bool CaptureScreen(const std::string& filename) {
            return Screen::CaptureScreen(filename);
        }

        // Get screen size
        static int GetScreenWidth() {
            return Screen::GetScreenWidth();
        }

        static int GetScreenHeight() {
            return Screen::GetScreenHeight();
        }
    };

} // namespace MemLib

#endif // SIMPLE_CHEAT_H
