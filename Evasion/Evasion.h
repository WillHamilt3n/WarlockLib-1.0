#pragma once

#ifndef EVASION_H
#define EVASION_H

#include <Windows.h>
#include <string>
#include <optional>

namespace WarlockLib {

    /**
     * Evasion - Anti-cheat and detection evasion techniques
     * Combines PEB hiding, thread hiding, syscalls, and obfuscation
     */
    class Evasion {
    public:
        // ===== PEB Hiding =====
        
        // Hide module from PEB (prevents FindModule detection)
        static bool HideModuleFromPEB(const std::wstring& moduleName);
        
        // Hide current module
        static bool HideCurrentModule();
        
        // Unlink module from PEB lists
        static bool UnlinkModuleFromPEB(HMODULE hModule);
        
        // ===== Thread Hiding =====
        
        // Hide current thread from debuggers
        static bool HideCurrentThread();
        
        // Hide specific thread
        static bool HideThread(HANDLE hThread);
        
        // ===== String Obfuscation =====
        
        // Encrypt string at compile time
        static std::string EncryptString(const std::string& str, BYTE key);
        
        // Decrypt string
        static std::string DecryptString(const std::string& encrypted, BYTE key);
        
        // XOR encrypt/decrypt
        static void XORCrypt(BYTE* data, size_t size, BYTE key);
        
        // ===== Detection Checks =====
        
        // Check if running under debugger
        static bool IsDebuggerPresent();
        
        // Check if being analyzed (Cheat Engine, x64dbg)
        static bool IsBeingAnalyzed();
        
        // Check if running in virtual machine
        static bool IsVirtualMachine();
        
        // Check for specific tools
        static bool IsCheatEngineRunning();
        static bool IsX64DbgRunning();
        static bool IsIDARunning();
        
        // ===== Direct Syscalls =====
        
        // Initialize syscall interface
        static bool InitializeSyscalls();
        
        // Cleanup syscalls
        static void ShutdownSyscalls();
        
        // Check if syscalls are initialized
        static bool AreSyscallsInitialized();
        
        // Read memory via syscall (bypasses hooks)
        template<typename T>
        static std::optional<T> SyscallRead(HANDLE processHandle, uintptr_t address) {
            if (!AreSyscallsInitialized()) {
                return std::nullopt;
            }
            
            T value{};
            SIZE_T bytesRead = 0;
            
            if (NT_ReadVirtualMemory(processHandle, reinterpret_cast<PVOID>(address), &value, sizeof(T), &bytesRead)) {
                if (bytesRead == sizeof(T)) {
                    return value;
                }
            }
            
            return std::nullopt;
        }
        
        // Write memory via syscall
        template<typename T>
        static bool SyscallWrite(HANDLE processHandle, uintptr_t address, const T& value) {
            if (!AreSyscallsInitialized()) {
                return false;
            }
            
            SIZE_T bytesWritten = 0;
            return NT_WriteVirtualMemory(processHandle, reinterpret_cast<PVOID>(address), 
                                        const_cast<T*>(&value), sizeof(T), &bytesWritten) 
                   && bytesWritten == sizeof(T);
        }
        
        // Allocate memory via syscall
        static uintptr_t SyscallAllocate(HANDLE processHandle, size_t size, DWORD protection = PAGE_EXECUTE_READWRITE);
        
        // Free memory via syscall
        static bool SyscallFree(HANDLE processHandle, uintptr_t address);
        
        // ===== Process Protection =====
        
        // Set process as critical (BSOD on termination)
        static bool SetProcessCritical(bool critical);
        
        // Elevate process privileges
        static bool ElevatePrivileges();
        
    private:
        static inline bool s_syscallsInitialized = false;
        static inline HMODULE s_ntdll = nullptr;
        
        // Syscall function pointers
        static inline void* s_NtReadVirtualMemory = nullptr;
        static inline void* s_NtWriteVirtualMemory = nullptr;
        static inline void* s_NtAllocateVirtualMemory = nullptr;
        static inline void* s_NtFreeVirtualMemory = nullptr;
        
        // Syscall wrappers
        static bool NT_ReadVirtualMemory(HANDLE processHandle, PVOID baseAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesRead);
        static bool NT_WriteVirtualMemory(HANDLE processHandle, PVOID baseAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesWritten);
        
        // Helper functions
        static bool FindSyscallNumbers();
        static void* GetNtFunction(const char* functionName);
    };

} // namespace WarlockLib

#endif // EVASION_H
