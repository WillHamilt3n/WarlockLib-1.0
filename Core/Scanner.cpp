#include "Scanner.h"
#include "Process.h"
#include <sstream>
#include <algorithm>

namespace WarlockLib {

    // ===== Pattern Scanning =====

    std::optional<uintptr_t> Scanner::FindPattern(HANDLE processHandle, const std::wstring& moduleName, const std::string& pattern) {
        // Get module info
        Process proc;
        auto modules = proc.GetModules(processHandle);
        
        uintptr_t moduleBase = 0;
        size_t moduleSize = 0;
        
        for (const auto& mod : modules) {
            if (mod.name == moduleName) {
                moduleBase = mod.baseAddress;
                moduleSize = mod.size;
                break;
            }
        }
        
        if (moduleBase == 0) {
            return std::nullopt;
        }
        
        return FindPatternInRegion(processHandle, moduleBase, moduleSize, pattern);
    }

    std::optional<uintptr_t> Scanner::FindPatternInRegion(HANDLE processHandle, uintptr_t startAddress, size_t size, const std::string& pattern) {
        // Parse pattern
        std::vector<BYTE> bytes;
        std::vector<bool> mask;
        
        if (!ParsePattern(pattern, bytes, mask)) {
            return std::nullopt;
        }
        
        // Read memory
        std::vector<BYTE> buffer(size);
        SIZE_T bytesRead;
        
        if (!ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(startAddress), buffer.data(), size, &bytesRead)) {
            return std::nullopt;
        }
        
        // Search for pattern
        for (size_t i = 0; i < bytesRead - bytes.size(); i++) {
            if (PatternMatch(buffer.data() + i, bytesRead - i, bytes, mask)) {
                return startAddress + i;
            }
        }
        
        return std::nullopt;
    }

    std::vector<uintptr_t> Scanner::FindAllPatterns(HANDLE processHandle, const std::wstring& moduleName, const std::string& pattern) {
        std::vector<uintptr_t> results;
        
        // Get module info
        Process proc;
        auto modules = proc.GetModules(processHandle);
        
        uintptr_t moduleBase = 0;
        size_t moduleSize = 0;
        
        for (const auto& mod : modules) {
            if (mod.name == moduleName) {
                moduleBase = mod.baseAddress;
                moduleSize = mod.size;
                break;
            }
        }
        
        if (moduleBase == 0) {
            return results;
        }
        
        // Parse pattern
        std::vector<BYTE> bytes;
        std::vector<bool> mask;
        
        if (!ParsePattern(pattern, bytes, mask)) {
            return results;
        }
        
        // Read memory
        std::vector<BYTE> buffer(moduleSize);
        SIZE_T bytesRead;
        
        if (!ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(moduleBase), buffer.data(), moduleSize, &bytesRead)) {
            return results;
        }
        
        // Search for all patterns
        for (size_t i = 0; i < bytesRead - bytes.size(); i++) {
            if (PatternMatch(buffer.data() + i, bytesRead - i, bytes, mask)) {
                results.push_back(moduleBase + i);
            }
        }
        
        return results;
    }

    std::optional<uintptr_t> Scanner::FindPatternInternal(const std::wstring& moduleName, const std::string& pattern) {
        HMODULE hModule = GetModuleHandleW(moduleName.c_str());
        if (!hModule) {
            return std::nullopt;
        }
        
        MODULEINFO modInfo;
        if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo))) {
            return std::nullopt;
        }
        
        // Parse pattern
        std::vector<BYTE> bytes;
        std::vector<bool> mask;
        
        if (!ParsePattern(pattern, bytes, mask)) {
            return std::nullopt;
        }
        
        uintptr_t moduleBase = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
        size_t moduleSize = modInfo.SizeOfImage;
        
        // Search for pattern
        for (size_t i = 0; i < moduleSize - bytes.size(); i++) {
            if (PatternMatch(reinterpret_cast<BYTE*>(moduleBase + i), moduleSize - i, bytes, mask)) {
                return moduleBase + i;
            }
        }
        
        return std::nullopt;
    }

    // ===== Code Cave Finding =====

    std::optional<uintptr_t> Scanner::FindCodeCave(HANDLE processHandle, const std::wstring& moduleName, size_t size) {
        // Get module info
        Process proc;
        auto modules = proc.GetModules(processHandle);
        
        uintptr_t moduleBase = 0;
        size_t moduleSize = 0;
        
        for (const auto& mod : modules) {
            if (mod.name == moduleName) {
                moduleBase = mod.baseAddress;
                moduleSize = mod.size;
                break;
            }
        }
        
        if (moduleBase == 0) {
            return std::nullopt;
        }
        
        // Read module memory
        std::vector<BYTE> buffer(moduleSize);
        SIZE_T bytesRead;
        
        if (!ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(moduleBase), buffer.data(), moduleSize, &bytesRead)) {
            return std::nullopt;
        }
        
        // Search for code cave
        for (size_t i = 0; i < bytesRead - size; i++) {
            if (IsCodeCaveValid(buffer.data() + i, size)) {
                // Verify it's executable
                MEMORY_BASIC_INFORMATION mbi;
                if (VirtualQueryEx(processHandle, reinterpret_cast<LPCVOID>(moduleBase + i), &mbi, sizeof(mbi))) {
                    if ((mbi.Protect & PAGE_EXECUTE_READ) || (mbi.Protect & PAGE_EXECUTE_READWRITE)) {
                        return moduleBase + i;
                    }
                }
            }
        }
        
        return std::nullopt;
    }

    std::optional<uintptr_t> Scanner::FindCodeCaveNear(HANDLE processHandle, uintptr_t address, size_t size, size_t maxDistance) {
        uintptr_t searchStart = (address > maxDistance) ? (address - maxDistance) : 0;
        uintptr_t searchEnd = address + maxDistance;
        
        // Read memory region
        size_t searchSize = searchEnd - searchStart;
        std::vector<BYTE> buffer(searchSize);
        SIZE_T bytesRead;
        
        if (!ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(searchStart), buffer.data(), searchSize, &bytesRead)) {
            return std::nullopt;
        }
        
        // Search for code cave
        for (size_t i = 0; i < bytesRead - size; i++) {
            if (IsCodeCaveValid(buffer.data() + i, size)) {
                return searchStart + i;
            }
        }
        
        return std::nullopt;
    }

    std::vector<uintptr_t> Scanner::FindAllCodeCaves(HANDLE processHandle, const std::wstring& moduleName, size_t minSize, size_t maxCount) {
        std::vector<uintptr_t> caves;
        
        // Get module info
        Process proc;
        auto modules = proc.GetModules(processHandle);
        
        uintptr_t moduleBase = 0;
        size_t moduleSize = 0;
        
        for (const auto& mod : modules) {
            if (mod.name == moduleName) {
                moduleBase = mod.baseAddress;
                moduleSize = mod.size;
                break;
            }
        }
        
        if (moduleBase == 0) {
            return caves;
        }
        
        // Read module memory
        std::vector<BYTE> buffer(moduleSize);
        SIZE_T bytesRead;
        
        if (!ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(moduleBase), buffer.data(), moduleSize, &bytesRead)) {
            return caves;
        }
        
        // Search for code caves
        for (size_t i = 0; i < bytesRead - minSize && caves.size() < maxCount; i++) {
            if (IsCodeCaveValid(buffer.data() + i, minSize)) {
                caves.push_back(moduleBase + i);
                i += minSize; // Skip past this cave
            }
        }
        
        return caves;
    }

    bool Scanner::VerifyCodeCave(HANDLE processHandle, uintptr_t address, size_t size) {
        std::vector<BYTE> buffer(size);
        SIZE_T bytesRead;
        
        if (!ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(address), buffer.data(), size, &bytesRead)) {
            return false;
        }
        
        return IsCodeCaveValid(buffer.data(), size);
    }

    // ===== Signature Helpers =====

    bool Scanner::ParsePattern(const std::string& pattern, std::vector<BYTE>& bytes, std::vector<bool>& mask) {
        std::istringstream iss(pattern);
        std::string byteStr;
        
        while (iss >> byteStr) {
            if (byteStr == "?" || byteStr == "??") {
                bytes.push_back(0);
                mask.push_back(false);
            }
            else {
                try {
                    BYTE byte = static_cast<BYTE>(std::stoi(byteStr, nullptr, 16));
                    bytes.push_back(byte);
                    mask.push_back(true);
                }
                catch (...) {
                    return false;
                }
            }
        }
        
        return !bytes.empty();
    }

    std::string Scanner::CreatePattern(const std::vector<BYTE>& bytes, const std::vector<bool>& mask) {
        std::ostringstream oss;
        
        for (size_t i = 0; i < bytes.size(); i++) {
            if (mask[i]) {
                oss << std::hex << std::uppercase << static_cast<int>(bytes[i]);
            }
            else {
                oss << "??";
            }
            
            if (i < bytes.size() - 1) {
                oss << " ";
            }
        }
        
        return oss.str();
    }

    // ===== Memory Analysis =====

    std::vector<std::pair<uintptr_t, size_t>> Scanner::FindExecutableRegions(HANDLE processHandle) {
        std::vector<std::pair<uintptr_t, size_t>> regions;
        
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        
        uintptr_t address = 0;
        uintptr_t maxAddress = reinterpret_cast<uintptr_t>(sysInfo.lpMaximumApplicationAddress);
        
        while (address < maxAddress) {
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQueryEx(processHandle, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi))) {
                if (mbi.State == MEM_COMMIT && 
                    ((mbi.Protect & PAGE_EXECUTE_READ) || (mbi.Protect & PAGE_EXECUTE_READWRITE))) {
                    regions.push_back({ address, mbi.RegionSize });
                }
                address += mbi.RegionSize;
            }
            else {
                break;
            }
        }
        
        return regions;
    }

    bool Scanner::IsExecutable(HANDLE processHandle, uintptr_t address) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQueryEx(processHandle, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi))) {
            return (mbi.Protect & PAGE_EXECUTE_READ) || (mbi.Protect & PAGE_EXECUTE_READWRITE);
        }
        return false;
    }

    std::vector<std::pair<uintptr_t, std::string>> Scanner::FindStrings(HANDLE processHandle, const std::wstring& moduleName, size_t minLength) {
        std::vector<std::pair<uintptr_t, std::string>> strings;
        
        // Get module info
        Process proc;
        auto modules = proc.GetModules(processHandle);
        
        uintptr_t moduleBase = 0;
        size_t moduleSize = 0;
        
        for (const auto& mod : modules) {
            if (mod.name == moduleName) {
                moduleBase = mod.baseAddress;
                moduleSize = mod.size;
                break;
            }
        }
        
        if (moduleBase == 0) {
            return strings;
        }
        
        // Read module memory
        std::vector<BYTE> buffer(moduleSize);
        SIZE_T bytesRead;
        
        if (!ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(moduleBase), buffer.data(), moduleSize, &bytesRead)) {
            return strings;
        }
        
        // Find ASCII strings
        std::string currentString;
        uintptr_t stringStart = 0;
        
        for (size_t i = 0; i < bytesRead; i++) {
            if (buffer[i] >= 32 && buffer[i] <= 126) { // Printable ASCII
                if (currentString.empty()) {
                    stringStart = moduleBase + i;
                }
                currentString += static_cast<char>(buffer[i]);
            }
            else {
                if (currentString.length() >= minLength) {
                    strings.push_back({ stringStart, currentString });
                }
                currentString.clear();
            }
        }
        
        return strings;
    }

    // ===== Helper Functions =====

    bool Scanner::PatternMatch(const BYTE* data, size_t dataSize, const std::vector<BYTE>& pattern, const std::vector<bool>& mask) {
        if (dataSize < pattern.size()) {
            return false;
        }
        
        for (size_t i = 0; i < pattern.size(); i++) {
            if (mask[i] && data[i] != pattern[i]) {
                return false;
            }
        }
        
        return true;
    }

    bool Scanner::IsCodeCaveValid(const BYTE* data, size_t size) {
        // Check if all bytes are 0x00 or 0x90 (NOP) or 0xCC (INT3)
        for (size_t i = 0; i < size; i++) {
            if (data[i] != 0x00 && data[i] != 0x90 && data[i] != 0xCC) {
                return false;
            }
        }
        return true;
    }

    bool Scanner::IsNOPSled(const BYTE* data, size_t size) {
        for (size_t i = 0; i < size; i++) {
            if (data[i] != 0x90) {
                return false;
            }
        }
        return true;
    }

} // namespace WarlockLib
