#pragma once

#ifndef PROCESS_H
#define PROCESS_H

#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <optional>
#include <vector>

namespace WarlockLib {

    /**
     * Process class for external memory manipulation
     * Handles process enumeration, opening, and module information
     */
    class Process {
    public:
        Process();
        ~Process();

        // Process attachment
        bool Attach(const std::wstring& processName);
        bool Attach(DWORD processId);
        void Detach();

        // Process information
        DWORD GetProcessId() const { return m_processId; }
        HANDLE GetHandle() const { return m_processHandle; }
        bool IsAttached() const { return m_processHandle != nullptr; }

        // Module utilities
        uintptr_t GetModuleBaseAddress(const std::wstring& moduleName);
        std::optional<MODULEENTRY32W> GetModuleInfo(const std::wstring& moduleName);
        std::vector<MODULEENTRY32W> GetAllModules();

        // Static utility functions
        static std::optional<DWORD> FindProcessId(const std::wstring& processName);
        static std::vector<std::wstring> EnumerateProcesses();

    private:
        DWORD m_processId;
        HANDLE m_processHandle;

        bool OpenProcess(DWORD processId);
    };

} // namespace WarlockLib

#endif // PROCESS_H
