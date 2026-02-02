#pragma once

#ifndef DMA_H
#define DMA_H

#include <Windows.h>
#include <string>
#include <vector>
#include <optional>
#include <map>

namespace WarlockLib {

    /**
     * DMA - Direct Memory Access via hardware (PCIe card)
     * The ultimate bypass - completely undetectable by software anti-cheat
     * 
     * Supported Hardware:
     * - PCILeech (FPGA-based DMA)
     * - ScreenPeek
     * - Custom FPGA devices
     * - Generic memory-mapped devices
     */
    class DMA {
    public:
        // Device types
        enum DeviceType {
            DEVICE_AUTO = 0,        // Auto-detect
            DEVICE_PCILEECH = 1,    // PCILeech FPGA
            DEVICE_SCREENPEAK = 2,  // ScreenPeek
            DEVICE_CUSTOM = 3,      // Custom device
            DEVICE_MEMMAP = 4       // Memory-mapped device
        };

        // ===== Initialization =====
        
        // Initialize DMA device
        static bool Initialize(DeviceType type = DEVICE_AUTO, const std::string& config = "");
        
        // Close DMA device
        static void Shutdown();
        
        // Check if DMA is available and initialized
        static bool IsAvailable();
        
        // Get device info
        static std::string GetDeviceInfo();
        
        // ===== Physical Memory Access =====
        
        // Read physical memory
        static bool ReadPhysical(uint64_t physicalAddress, void* buffer, size_t size);
        
        // Write physical memory
        static bool WritePhysical(uint64_t physicalAddress, const void* buffer, size_t size);
        
        // Read physical memory (templated)
        template<typename T>
        static std::optional<T> ReadPhysical(uint64_t physicalAddress) {
            T value{};
            if (ReadPhysical(physicalAddress, &value, sizeof(T))) {
                return value;
            }
            return std::nullopt;
        }
        
        // Write physical memory (templated)
        template<typename T>
        static bool WritePhysical(uint64_t physicalAddress, const T& value) {
            return WritePhysical(physicalAddress, &value, sizeof(T));
        }
        
        // ===== Virtual Memory Access =====
        
        // Read virtual memory (handles V2P translation automatically)
        static bool ReadVirtual(DWORD processId, uintptr_t virtualAddress, void* buffer, size_t size);
        
        // Write virtual memory
        static bool WriteVirtual(DWORD processId, uintptr_t virtualAddress, const void* buffer, size_t size);
        
        // Read virtual memory (templated)
        template<typename T>
        static std::optional<T> Read(DWORD processId, uintptr_t virtualAddress) {
            T value{};
            if (ReadVirtual(processId, virtualAddress, &value, sizeof(T))) {
                return value;
            }
            return std::nullopt;
        }
        
        // Write virtual memory (templated)
        template<typename T>
        static bool Write(DWORD processId, uintptr_t virtualAddress, const T& value) {
            return WriteVirtual(processId, virtualAddress, &value, sizeof(T));
        }
        
        // Read bytes
        static std::vector<BYTE> ReadBytes(DWORD processId, uintptr_t virtualAddress, size_t size);
        
        // ===== Address Translation =====
        
        // Convert virtual address to physical address
        static std::optional<uint64_t> VirtualToPhysical(DWORD processId, uintptr_t virtualAddress);
        
        // Get process DirectoryTableBase (CR3)
        static std::optional<uint64_t> GetProcessDTB(DWORD processId);
        
        // ===== Process Operations =====
        
        // Find process by name
        static DWORD FindProcess(const std::wstring& processName);
        
        // Get all processes
        static std::vector<std::pair<DWORD, std::wstring>> GetProcessList();
        
        // Get module base address
        static std::optional<uintptr_t> GetModuleBase(DWORD processId, const std::wstring& moduleName);
        
        // Get module size
        static std::optional<size_t> GetModuleSize(DWORD processId, const std::wstring& moduleName);
        
        // List all modules in process
        static std::vector<std::pair<std::wstring, uintptr_t>> GetModuleList(DWORD processId);
        
        // ===== Pattern Scanning =====
        
        // Find pattern in module
        static std::optional<uintptr_t> FindPattern(DWORD processId, const std::wstring& moduleName, const std::string& pattern);
        
        // Find pattern in memory range
        static std::optional<uintptr_t> FindPatternInRange(DWORD processId, uintptr_t start, size_t size, const std::string& pattern);
        
        // Find all patterns
        static std::vector<uintptr_t> FindAllPatterns(DWORD processId, const std::wstring& moduleName, const std::string& pattern, size_t maxResults = 10);
        
        // ===== Scatter Read/Write (Performance) =====
        
        // Read multiple addresses at once (performance optimization)
        struct ScatterEntry {
            uintptr_t address;
            void* buffer;
            size_t size;
            bool success;
        };
        
        static bool ScatterRead(DWORD processId, std::vector<ScatterEntry>& entries);
        static bool ScatterWrite(DWORD processId, std::vector<ScatterEntry>& entries);
        
        // ===== Caching (Performance) =====
        
        // Enable/disable DTB caching
        static void SetDTBCaching(bool enabled);
        
        // Clear DTB cache
        static void ClearDTBCache();
        
        // ===== Statistics =====
        
        struct Statistics {
            uint64_t totalReads;
            uint64_t totalWrites;
            uint64_t bytesRead;
            uint64_t bytesWritten;
            uint64_t failedReads;
            uint64_t failedWrites;
        };
        
        static Statistics GetStatistics();
        static void ResetStatistics();
        
    private:
        // Device handle
        static inline void* s_deviceHandle = nullptr;
        static inline DeviceType s_deviceType = DEVICE_AUTO;
        static inline bool s_initialized = false;
        
        // DTB cache
        static inline bool s_dtbCachingEnabled = true;
        static inline std::map<DWORD, uint64_t> s_dtbCache;
        
        // Statistics
        static inline Statistics s_stats = {};
        
        // Internal helpers
        static bool InitializePCILeech(const std::string& config);
        static bool InitializeScreenPeek(const std::string& config);
        static bool InitializeMemMap(const std::string& config);
        
        static uint64_t TranslateAddress(uint64_t dtb, uintptr_t virtualAddress);
        static bool ParsePattern(const std::string& pattern, std::vector<BYTE>& bytes, std::vector<bool>& mask);
    };

} // namespace WarlockLib

#endif // DMA_H
