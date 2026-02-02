#include "Evasion.h"
#include <TlHelp32.h>
#include <winternl.h>

namespace WarlockLib {

    // ===== PEB Hiding =====

    bool Evasion::HideModuleFromPEB(const std::wstring& moduleName) {
        HMODULE hModule = GetModuleHandleW(moduleName.c_str());
        if (!hModule) {
            return false;
        }
        
        return UnlinkModuleFromPEB(hModule);
    }

    bool Evasion::HideCurrentModule() {
        HMODULE hModule = GetModuleHandleW(nullptr);
        return UnlinkModuleFromPEB(hModule);
    }

    bool Evasion::UnlinkModuleFromPEB(HMODULE hModule) {
        #ifdef _WIN64
        PPEB peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
        #else
        PPEB peb = reinterpret_cast<PPEB>(__readfsdword(0x30));
        #endif

        if (!peb) {
            return false;
        }

        PPEB_LDR_DATA ldr = peb->Ldr;
        if (!ldr) {
            return false;
        }

        // Traverse module lists and unlink
        PLIST_ENTRY currentEntry = ldr->InLoadOrderModuleList.Flink;
        
        while (currentEntry != &ldr->InLoadOrderModuleList) {
            PLDR_DATA_TABLE_ENTRY dataTableEntry = CONTAINING_RECORD(currentEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
            
            if (dataTableEntry->DllBase == hModule) {
                // Unlink from InLoadOrderModuleList
                dataTableEntry->InLoadOrderLinks.Flink->Blink = dataTableEntry->InLoadOrderLinks.Blink;
                dataTableEntry->InLoadOrderLinks.Blink->Flink = dataTableEntry->InLoadOrderLinks.Flink;
                
                // Unlink from InMemoryOrderModuleList
                dataTableEntry->InMemoryOrderLinks.Flink->Blink = dataTableEntry->InMemoryOrderLinks.Blink;
                dataTableEntry->InMemoryOrderLinks.Blink->Flink = dataTableEntry->InMemoryOrderLinks.Flink;
                
                // Unlink from InInitializationOrderModuleList
                dataTableEntry->InInitializationOrderLinks.Flink->Blink = dataTableEntry->InInitializationOrderLinks.Blink;
                dataTableEntry->InInitializationOrderLinks.Blink->Flink = dataTableEntry->InInitializationOrderLinks.Flink;
                
                return true;
            }
            
            currentEntry = currentEntry->Flink;
        }

        return false;
    }

    // ===== Thread Hiding =====

    bool Evasion::HideCurrentThread() {
        return HideThread(GetCurrentThread());
    }

    bool Evasion::HideThread(HANDLE hThread) {
        typedef NTSTATUS(WINAPI* pNtSetInformationThread)(HANDLE, UINT, PVOID, ULONG);
        
        HMODULE ntdll = GetModuleHandleA("ntdll.dll");
        if (!ntdll) {
            return false;
        }
        
        auto NtSetInformationThread = reinterpret_cast<pNtSetInformationThread>(
            GetProcAddress(ntdll, "NtSetInformationThread"));
        
        if (!NtSetInformationThread) {
            return false;
        }
        
        // ThreadHideFromDebugger = 0x11
        return NtSetInformationThread(hThread, 0x11, nullptr, 0) == 0;
    }

    // ===== String Obfuscation =====

    std::string Evasion::EncryptString(const std::string& str, BYTE key) {
        std::string encrypted = str;
        XORCrypt(reinterpret_cast<BYTE*>(encrypted.data()), encrypted.size(), key);
        return encrypted;
    }

    std::string Evasion::DecryptString(const std::string& encrypted, BYTE key) {
        // XOR is symmetric
        return EncryptString(encrypted, key);
    }

    void Evasion::XORCrypt(BYTE* data, size_t size, BYTE key) {
        for (size_t i = 0; i < size; i++) {
            data[i] ^= key;
        }
    }

    // ===== Detection Checks =====

    bool Evasion::IsDebuggerPresent() {
        return ::IsDebuggerPresent() != 0;
    }

    bool Evasion::IsBeingAnalyzed() {
        return IsCheatEngineRunning() || IsX64DbgRunning() || IsIDARunning();
    }

    bool Evasion::IsVirtualMachine() {
        // Check for VM artifacts
        
        // CPUID check
        int cpuInfo[4] = {};
        __cpuid(cpuInfo, 1);
        bool hypervisorBit = (cpuInfo[2] & (1 << 31)) != 0;
        
        if (hypervisorBit) {
            return true;
        }
        
        // Check for VMware/VirtualBox registry keys
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\VBoxGuest", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return true;
        }
        
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\VMTools", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return true;
        }
        
        return false;
    }

    bool Evasion::IsCheatEngineRunning() {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        if (Process32FirstW(snapshot, &pe32)) {
            do {
                if (wcsstr(pe32.szExeFile, L"cheatengine") || 
                    wcsstr(pe32.szExeFile, L"Cheat Engine")) {
                    CloseHandle(snapshot);
                    return true;
                }
            } while (Process32NextW(snapshot, &pe32));
        }
        
        CloseHandle(snapshot);
        return false;
    }

    bool Evasion::IsX64DbgRunning() {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        if (Process32FirstW(snapshot, &pe32)) {
            do {
                if (wcsstr(pe32.szExeFile, L"x64dbg") || 
                    wcsstr(pe32.szExeFile, L"x32dbg")) {
                    CloseHandle(snapshot);
                    return true;
                }
            } while (Process32NextW(snapshot, &pe32));
        }
        
        CloseHandle(snapshot);
        return false;
    }

    bool Evasion::IsIDARunning() {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        if (Process32FirstW(snapshot, &pe32)) {
            do {
                if (wcsstr(pe32.szExeFile, L"ida") || 
                    wcsstr(pe32.szExeFile, L"IDA")) {
                    CloseHandle(snapshot);
                    return true;
                }
            } while (Process32NextW(snapshot, &pe32));
        }
        
        CloseHandle(snapshot);
        return false;
    }

    // ===== Direct Syscalls =====

    bool Evasion::InitializeSyscalls() {
        if (s_syscallsInitialized) {
            return true;
        }
        
        s_ntdll = GetModuleHandleA("ntdll.dll");
        if (!s_ntdll) {
            return false;
        }
        
        s_NtReadVirtualMemory = GetNtFunction("NtReadVirtualMemory");
        s_NtWriteVirtualMemory = GetNtFunction("NtWriteVirtualMemory");
        s_NtAllocateVirtualMemory = GetNtFunction("NtAllocateVirtualMemory");
        s_NtFreeVirtualMemory = GetNtFunction("NtFreeVirtualMemory");
        
        s_syscallsInitialized = (s_NtReadVirtualMemory != nullptr) && 
                                (s_NtWriteVirtualMemory != nullptr);
        
        return s_syscallsInitialized;
    }

    void Evasion::ShutdownSyscalls() {
        s_syscallsInitialized = false;
        s_NtReadVirtualMemory = nullptr;
        s_NtWriteVirtualMemory = nullptr;
        s_NtAllocateVirtualMemory = nullptr;
        s_NtFreeVirtualMemory = nullptr;
    }

    bool Evasion::AreSyscallsInitialized() {
        return s_syscallsInitialized;
    }

    bool Evasion::NT_ReadVirtualMemory(HANDLE processHandle, PVOID baseAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesRead) {
        typedef NTSTATUS(NTAPI* pNtReadVirtualMemory)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
        
        auto NtReadVM = reinterpret_cast<pNtReadVirtualMemory>(s_NtReadVirtualMemory);
        if (!NtReadVM) {
            return false;
        }
        
        NTSTATUS status = NtReadVM(processHandle, baseAddress, buffer, size, bytesRead);
        return status == 0;
    }

    bool Evasion::NT_WriteVirtualMemory(HANDLE processHandle, PVOID baseAddress, PVOID buffer, SIZE_T size, SIZE_T* bytesWritten) {
        typedef NTSTATUS(NTAPI* pNtWriteVirtualMemory)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
        
        auto NtWriteVM = reinterpret_cast<pNtWriteVirtualMemory>(s_NtWriteVirtualMemory);
        if (!NtWriteVM) {
            return false;
        }
        
        NTSTATUS status = NtWriteVM(processHandle, baseAddress, buffer, size, bytesWritten);
        return status == 0;
    }

    uintptr_t Evasion::SyscallAllocate(HANDLE processHandle, size_t size, DWORD protection) {
        typedef NTSTATUS(NTAPI* pNtAllocateVirtualMemory)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
        
        auto NtAllocVM = reinterpret_cast<pNtAllocateVirtualMemory>(s_NtAllocateVirtualMemory);
        if (!NtAllocVM) {
            return 0;
        }
        
        PVOID baseAddress = nullptr;
        SIZE_T regionSize = size;
        
        NTSTATUS status = NtAllocVM(processHandle, &baseAddress, 0, &regionSize, MEM_COMMIT | MEM_RESERVE, protection);
        
        if (status == 0) {
            return reinterpret_cast<uintptr_t>(baseAddress);
        }
        
        return 0;
    }

    bool Evasion::SyscallFree(HANDLE processHandle, uintptr_t address) {
        typedef NTSTATUS(NTAPI* pNtFreeVirtualMemory)(HANDLE, PVOID*, PSIZE_T, ULONG);
        
        auto NtFreeVM = reinterpret_cast<pNtFreeVirtualMemory>(s_NtFreeVirtualMemory);
        if (!NtFreeVM) {
            return false;
        }
        
        PVOID baseAddress = reinterpret_cast<PVOID>(address);
        SIZE_T regionSize = 0;
        
        NTSTATUS status = NtFreeVM(processHandle, &baseAddress, &regionSize, MEM_RELEASE);
        return status == 0;
    }

    void* Evasion::GetNtFunction(const char* functionName) {
        return GetProcAddress(s_ntdll, functionName);
    }

    // ===== Process Protection =====

    bool Evasion::SetProcessCritical(bool critical) {
        typedef NTSTATUS(WINAPI* pRtlSetProcessIsCritical)(BOOLEAN, PBOOLEAN, BOOLEAN);
        
        HMODULE ntdll = GetModuleHandleA("ntdll.dll");
        if (!ntdll) {
            return false;
        }
        
        auto RtlSetProcessIsCritical = reinterpret_cast<pRtlSetProcessIsCritical>(
            GetProcAddress(ntdll, "RtlSetProcessIsCritical"));
        
        if (!RtlSetProcessIsCritical) {
            return false;
        }
        
        return RtlSetProcessIsCritical(critical ? TRUE : FALSE, nullptr, FALSE) == 0;
    }

    bool Evasion::ElevatePrivileges() {
        HANDLE hToken;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
            return false;
        }
        
        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        
        if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &tp.Privileges[0].Luid)) {
            CloseHandle(hToken);
            return false;
        }
        
        bool result = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr) != 0;
        
        CloseHandle(hToken);
        return result;
    }

} // namespace WarlockLib
