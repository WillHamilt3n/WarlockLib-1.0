#pragma once

#ifndef SCANNER_H
#define SCANNER_H

#include <Windows.h>
#include <string>
#include <vector>
#include <optional>

namespace WarlockLib {

    /**
     * Scanner - Code analysis and pattern matching
     * Combines pattern scanning (AOB) and code cave finding
     */
    class Scanner {
    public:
        // ===== Pattern Scanning (AOB - Array of Bytes) =====
        
        // Find pattern in module
        static std::optional<uintptr_t> FindPattern(HANDLE processHandle, const std::wstring& moduleName, const std::string& pattern);
        
        // Find pattern in memory region
        static std::optional<uintptr_t> FindPatternInRegion(HANDLE processHandle, uintptr_t startAddress, size_t size, const std::string& pattern);
        
        // Find all occurrences of pattern
        static std::vector<uintptr_t> FindAllPatterns(HANDLE processHandle, const std::wstring& moduleName, const std::string& pattern);
        
        // Internal pattern scan (current process)
        static std::optional<uintptr_t> FindPatternInternal(const std::wstring& moduleName, const std::string& pattern);
        
        // ===== Code Cave Finding =====
        
        // Find code cave (unused memory) in module
        static std::optional<uintptr_t> FindCodeCave(HANDLE processHandle, const std::wstring& moduleName, size_t size);
        
        // Find code cave near specific address (for relative jumps)
        static std::optional<uintptr_t> FindCodeCaveNear(HANDLE processHandle, uintptr_t address, size_t size, size_t maxDistance = 0x1000);
        
        // Find all code caves in module
        static std::vector<uintptr_t> FindAllCodeCaves(HANDLE processHandle, const std::wstring& moduleName, size_t minSize, size_t maxCount = 10);
        
        // Verify code cave is empty (all zeros or NOPs)
        static bool VerifyCodeCave(HANDLE processHandle, uintptr_t address, size_t size);
        
        // ===== Signature Helpers =====
        
        // Convert IDA-style pattern to bytes and mask
        static bool ParsePattern(const std::string& pattern, std::vector<BYTE>& bytes, std::vector<bool>& mask);
        
        // Create pattern from bytes
        static std::string CreatePattern(const std::vector<BYTE>& bytes, const std::vector<bool>& mask);
        
        // ===== Memory Analysis =====
        
        // Find all executable regions
        static std::vector<std::pair<uintptr_t, size_t>> FindExecutableRegions(HANDLE processHandle);
        
        // Check if address is executable
        static bool IsExecutable(HANDLE processHandle, uintptr_t address);
        
        // Find strings in memory
        static std::vector<std::pair<uintptr_t, std::string>> FindStrings(HANDLE processHandle, const std::wstring& moduleName, size_t minLength = 4);
        
    private:
        // Pattern matching helper
        static bool PatternMatch(const BYTE* data, size_t dataSize, const std::vector<BYTE>& pattern, const std::vector<bool>& mask);
        
        // Code cave helpers
        static bool IsCodeCaveValid(const BYTE* data, size_t size);
        static bool IsNOPSled(const BYTE* data, size_t size);
    };

} // namespace WarlockLib

#endif // SCANNER_H
