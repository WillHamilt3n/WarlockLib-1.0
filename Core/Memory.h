#pragma once

#ifndef MEMORY_H
#define MEMORY_H

#include <Windows.h>
#include <vector>
#include <optional>
#include <thread>
#include <atomic>
#include <map>
#include <mutex>
#include <functional>

namespace WarlockLib {

    // Forward declaration
    class Process;

    /**
     * Memory class - Complete memory manipulation toolkit
     * - Basic read/write operations
     * - Memory freezing (continuous writing)
     * - Function hooking (inline, VTable, trampolines)
     * - Code patching (NOP, restore)
     */
    class Memory {
    public:
        // ===== Basic Memory Operations =====

        // External memory operations (requires Process handle)
        template<typename T>
        static std::optional<T> Read(HANDLE processHandle, uintptr_t address) {
            T value{};
            SIZE_T bytesRead;
            if (ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), &bytesRead)) {
                if (bytesRead == sizeof(T)) {
                    return value;
                }
            }
            return std::nullopt;
        }

        template<typename T>
        static bool Write(HANDLE processHandle, uintptr_t address, const T& value) {
            SIZE_T bytesWritten;
            return WriteProcessMemory(processHandle, reinterpret_cast<LPVOID>(address), &value, sizeof(T), &bytesWritten)
                && bytesWritten == sizeof(T);
        }

        // Read array of bytes
        static std::vector<byte> ReadBytes(HANDLE processHandle, uintptr_t address, size_t size);
        static bool WriteBytes(HANDLE processHandle, uintptr_t address, const std::vector<byte>& bytes);

        // Internal memory operations (current process)
        template<typename T>
        static T ReadInternal(uintptr_t address) {
            return *reinterpret_cast<T*>(address);
        }

        template<typename T>
        static void WriteInternal(uintptr_t address, const T& value) {
            *reinterpret_cast<T*>(address) = value;
        }

        // Protected memory operations
        template<typename T>
        static bool WriteProtected(HANDLE processHandle, uintptr_t address, const T& value) {
            DWORD oldProtection;
            if (VirtualProtectEx(processHandle, reinterpret_cast<LPVOID>(address), sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtection)) {
                bool result = Write(processHandle, address, value);
                VirtualProtectEx(processHandle, reinterpret_cast<LPVOID>(address), sizeof(T), oldProtection, &oldProtection);
                return result;
            }
            return false;
        }

        template<typename T>
        static bool WriteProtectedInternal(uintptr_t address, const T& value) {
            DWORD oldProtection;
            if (VirtualProtect(reinterpret_cast<LPVOID>(address), sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtection)) {
                WriteInternal(address, value);
                VirtualProtect(reinterpret_cast<LPVOID>(address), sizeof(T), oldProtection, &oldProtection);
                return true;
            }
            return false;
        }

        // Pointer chain following
        static std::optional<uintptr_t> FollowPointerChain(HANDLE processHandle, uintptr_t baseAddress, const std::vector<uintptr_t>& offsets);
        static uintptr_t FollowPointerChainInternal(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets);

        // Memory region information
        static std::optional<MEMORY_BASIC_INFORMATION> QueryMemory(HANDLE processHandle, uintptr_t address);
        static std::vector<MEMORY_BASIC_INFORMATION> GetMemoryRegions(HANDLE processHandle);

        // Allocation/Deallocation
        static uintptr_t AllocateMemory(HANDLE processHandle, size_t size, DWORD protection = PAGE_EXECUTE_READWRITE);
        static bool FreeMemory(HANDLE processHandle, uintptr_t address);

        // ===== Memory Freezing (God Mode, Infinite Ammo, etc.) =====

        struct FrozenValue {
            uintptr_t address;
            std::vector<byte> value;
            size_t size;
            DWORD intervalMs;
            HANDLE processHandle;
            bool useDriver;
            void* driverPtr;
        };

        // Freeze a value at an address (continuously writes it)
        template<typename T>
        static bool Freeze(uintptr_t address, const T& value, DWORD intervalMs = 100, HANDLE processHandle = nullptr) {
            std::lock_guard<std::mutex> lock(s_freezerMutex);

            FrozenValue frozen{};
            frozen.address = address;
            frozen.value.resize(sizeof(T));
            memcpy(frozen.value.data(), &value, sizeof(T));
            frozen.size = sizeof(T);
            frozen.intervalMs = intervalMs;
            frozen.processHandle = processHandle;
            frozen.useDriver = false;
            frozen.driverPtr = nullptr;

            s_frozenValues[address] = frozen;

            if (!s_freezerRunning) {
                StartFreezer();
            }

            return true;
        }

        // Unfreeze a specific address
        static bool Unfreeze(uintptr_t address);

        // Unfreeze all addresses
        static void UnfreezeAll();

        // Check if address is frozen
        static bool IsFrozen(uintptr_t address);

        // Get count of frozen addresses
        static size_t GetFrozenCount();

        // ===== Function Hooking =====

        // Inline/Detour Hook
        class InlineHook {
        public:
            InlineHook(uintptr_t targetFunction, uintptr_t hookFunction);
            ~InlineHook();

            bool Install();
            bool Uninstall();
            bool IsInstalled() const { return m_installed; }

            uintptr_t GetOriginalFunction() const { return m_originalFunction; }
            uintptr_t GetTrampoline() const { return m_trampoline; }

        private:
            uintptr_t m_targetFunction;
            uintptr_t m_hookFunction;
            uintptr_t m_trampoline;
            uintptr_t m_originalFunction;
            std::vector<byte> m_originalBytes;
            bool m_installed;

            bool CreateTrampoline();
            std::vector<byte> CreateJump(uintptr_t from, uintptr_t to);
        };

        // VTable Hook
        class VTableHook {
        public:
            VTableHook(void* instance);
            ~VTableHook();

            bool HookFunction(size_t index, uintptr_t hookFunction);
            bool UnhookFunction(size_t index);
            void UnhookAll();

            uintptr_t GetOriginalFunction(size_t index) const;

        private:
            void* m_instance;
            uintptr_t* m_originalVTable;
            uintptr_t* m_newVTable;
            size_t m_vTableSize;
            std::vector<size_t> m_hookedIndices;

            size_t GetVTableSize();
        };

        // Simple function pointer hook
        template<typename T>
        static bool HookFunctionPointer(T** functionPtr, T* hookFunction, T** originalFunction) {
            DWORD oldProtection;
            if (VirtualProtect(functionPtr, sizeof(T*), PAGE_EXECUTE_READWRITE, &oldProtection)) {
                *originalFunction = *functionPtr;
                *functionPtr = hookFunction;
                VirtualProtect(functionPtr, sizeof(T*), oldProtection, &oldProtection);
                return true;
            }
            return false;
        }

        // ===== Code Patching =====

        // NOP (No Operation) bytes - disable code
        static bool Nop(uintptr_t address, size_t count);
        
        // Restore original bytes
        static bool RestoreBytes(uintptr_t address, const std::vector<byte>& originalBytes);

        // Save original bytes before patching
        static std::vector<byte> SaveBytes(uintptr_t address, size_t count);

    private:
        // Freezer internals
        static inline std::map<uintptr_t, FrozenValue> s_frozenValues;
        static inline std::thread s_freezerThread;
        static inline std::atomic<bool> s_freezerRunning{ false };
        static inline std::mutex s_freezerMutex;

        static void StartFreezer();
        static void StopFreezer();
        static void FreezerLoop();
        static void WriteFrozenValue(const FrozenValue& frozen);
    };

} // namespace WarlockLib

#endif // MEMORY_H
