#include "Process.h"

namespace WarlockLib {

    Process::Process() : m_processId(0), m_processHandle(nullptr) {
    }

    Process::~Process() {
        Detach();
    }

    bool Process::Attach(const std::wstring& processName) {
        auto pid = FindProcessId(processName);
        if (!pid.has_value()) {
            return false;
        }
        return Attach(pid.value());
    }

    bool Process::Attach(DWORD processId) {
        Detach();
        m_processId = processId;
        return OpenProcess(processId);
    }

    void Process::Detach() {
        if (m_processHandle) {
            CloseHandle(m_processHandle);
            m_processHandle = nullptr;
        }
        m_processId = 0;
    }

    bool Process::OpenProcess(DWORD processId) {
        m_processHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        return m_processHandle != nullptr;
    }

    uintptr_t Process::GetModuleBaseAddress(const std::wstring& moduleName) {
        auto moduleInfo = GetModuleInfo(moduleName);
        if (moduleInfo.has_value()) {
            return reinterpret_cast<uintptr_t>(moduleInfo->modBaseAddr);
        }
        return 0;
    }

    std::optional<MODULEENTRY32W> Process::GetModuleInfo(const std::wstring& moduleName) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_processId);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return std::nullopt;
        }

        MODULEENTRY32W moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32W);

        if (Module32FirstW(snapshot, &moduleEntry)) {
            do {
                if (_wcsicmp(moduleEntry.szModule, moduleName.c_str()) == 0) {
                    CloseHandle(snapshot);
                    return moduleEntry;
                }
            } while (Module32NextW(snapshot, &moduleEntry));
        }

        CloseHandle(snapshot);
        return std::nullopt;
    }

    std::vector<MODULEENTRY32W> Process::GetAllModules() {
        std::vector<MODULEENTRY32W> modules;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_processId);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return modules;
        }

        MODULEENTRY32W moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32W);

        if (Module32FirstW(snapshot, &moduleEntry)) {
            do {
                modules.push_back(moduleEntry);
            } while (Module32NextW(snapshot, &moduleEntry));
        }

        CloseHandle(snapshot);
        return modules;
    }

    std::optional<DWORD> Process::FindProcessId(const std::wstring& processName) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return std::nullopt;
        }

        PROCESSENTRY32W processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                if (_wcsicmp(processEntry.szExeFile, processName.c_str()) == 0) {
                    CloseHandle(snapshot);
                    return processEntry.th32ProcessID;
                }
            } while (Process32NextW(snapshot, &processEntry));
        }

        CloseHandle(snapshot);
        return std::nullopt;
    }

    std::vector<std::wstring> Process::EnumerateProcesses() {
        std::vector<std::wstring> processes;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return processes;
        }

        PROCESSENTRY32W processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                processes.push_back(processEntry.szExeFile);
            } while (Process32NextW(snapshot, &processEntry));
        }

        CloseHandle(snapshot);
        return processes;
    }

} // namespace WarlockLib
