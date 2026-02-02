#include "DMA.h"

namespace WarlockLib {

    // DMA statistics
    DMA::Statistics DMA::s_stats = {};
    bool DMA::s_initialized = false;
    void* DMA::s_deviceHandle = nullptr;
    DeviceType DMA::s_deviceType = DEVICE_AUTO;
    std::unordered_map<DWORD, uint64_t> DMA::s_dtbCache;
    bool DMA::s_dtbCachingEnabled = true;

    // ===== Initialization =====

    bool DMA::Initialize(DeviceType type, const std::string& config) {
        if (s_initialized) {
            return true;
        }

        s_deviceType = type;

        // Auto-detect device type if requested
        if (type == DEVICE_AUTO) {
            // Try PCILeech first (most common)
            if (InitializePCILeech(config)) {
                s_deviceType = DEVICE_PCILEECH;
                s_initialized = true;
                return true;
            }

            // Try ScreenPeek
            if (InitializeScreenPeek(config)) {
                s_deviceType = DEVICE_SCREENPEAK;
                s_initialized = true;
                return true;
            }

            // Try memory-mapped device
            if (InitializeMemMap(config)) {
                s_deviceType = DEVICE_MEMMAP;
                s_initialized = true;
                return true;
            }

            return false;
        }

        // Initialize specific device type
        switch (type) {
        case DEVICE_PCILEECH:
            s_initialized = InitializePCILeech(config);
            break;
        case DEVICE_SCREENPEAK:
            s_initialized = InitializeScreenPeek(config);
            break;
        case DEVICE_MEMMAP:
            s_initialized = InitializeMemMap(config);
            break;
        default:
            return false;
        }

        return s_initialized;
    }

    void DMA::Shutdown() {
        if (s_deviceHandle) {
            // Close device handle based on type
            switch (s_deviceType) {
            case DEVICE_PCILEECH:
            case DEVICE_SCREENPEAK:
                CloseHandle(s_deviceHandle);
                break;
            case DEVICE_MEMMAP:
                UnmapViewOfFile(s_deviceHandle);
                break;
            }
            s_deviceHandle = nullptr;
        }
        s_initialized = false;
        s_dtbCache.clear();
    }

    bool DMA::IsAvailable() {
        return s_initialized && s_deviceHandle != nullptr;
    }

    std::string DMA::GetDeviceInfo() {
        if (!s_initialized) {
            return "Not initialized";
        }

        switch (s_deviceType) {
        case DEVICE_PCILEECH:
            return "PCILeech FPGA Device";
        case DEVICE_SCREENPEAK:
            return "ScreenPeek Device";
        case DEVICE_MEMMAP:
            return "Memory-Mapped Device";
        case DEVICE_CUSTOM:
            return "Custom Device";
        default:
            return "Unknown Device";
        }
    }

    // ===== Device Initialization =====

    bool DMA::InitializePCILeech(const std::string& config) {
        // Try to open PCILeech device
        // This is a placeholder - actual implementation would use PCILeech API
        // For now, we'll try to open a device handle
        
        std::wstring devicePath = L"\\\\.\\PCILeech";
        if (!config.empty()) {
            devicePath = std::wstring(config.begin(), config.end());
        }

        s_deviceHandle = CreateFileW(
            devicePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        return s_deviceHandle != INVALID_HANDLE_VALUE;
    }

    bool DMA::InitializeScreenPeek(const std::string& config) {
        // Try to open ScreenPeek device
        std::wstring devicePath = L"\\\\.\\ScreenPeek";
        if (!config.empty()) {
            devicePath = std::wstring(config.begin(), config.end());
        }

        s_deviceHandle = CreateFileW(
            devicePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        return s_deviceHandle != INVALID_HANDLE_VALUE;
    }

    bool DMA::InitializeMemMap(const std::string& config) {
        // Open physical memory (requires driver support)
        HANDLE hPhysMem = CreateFileW(
            L"\\\\.\\PhysicalMemory",
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (hPhysMem == INVALID_HANDLE_VALUE) {
            return false;
        }

        // Map a portion of physical memory
        HANDLE hMapFile = CreateFileMappingW(
            hPhysMem,
            nullptr,
            PAGE_READWRITE,
            0,
            0,
            nullptr
        );

        CloseHandle(hPhysMem);

        if (!hMapFile) {
            return false;
        }

        s_deviceHandle = MapViewOfFile(
            hMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            0
        );

        CloseHandle(hMapFile);

        return s_deviceHandle != nullptr;
    }

    // ===== Physical Memory Access =====

    bool DMA::ReadPhysical(uint64_t physicalAddress, void* buffer, size_t size) {
        if (!s_initialized || !buffer || size == 0) {
            s_stats.failedReads++;
            return false;
        }

        // Placeholder for actual DMA read
        // Real implementation would use device-specific API
        // For now, this is a framework
        
        bool success = false;

        switch (s_deviceType) {
        case DEVICE_PCILEECH:
        case DEVICE_SCREENPEAK:
            // Would use DeviceIoControl with custom IOCTL
            // IOCTL_READ_PHYSICAL_MEMORY or similar
            success = false; // Placeholder
            break;

        case DEVICE_MEMMAP:
            // Direct memory access
            if (s_deviceHandle) {
                memcpy(buffer, (BYTE*)s_deviceHandle + physicalAddress, size);
                success = true;
            }
            break;
        }

        if (success) {
            s_stats.totalReads++;
            s_stats.bytesRead += size;
        }
        else {
            s_stats.failedReads++;
        }

        return success;
    }

    bool DMA::WritePhysical(uint64_t physicalAddress, const void* buffer, size_t size) {
        if (!s_initialized || !buffer || size == 0) {
            s_stats.failedWrites++;
            return false;
        }

        bool success = false;

        switch (s_deviceType) {
        case DEVICE_PCILEECH:
        case DEVICE_SCREENPEAK:
            success = false; // Placeholder
            break;

        case DEVICE_MEMMAP:
            if (s_deviceHandle) {
                memcpy((BYTE*)s_deviceHandle + physicalAddress, buffer, size);
                success = true;
            }
            break;
        }

        if (success) {
            s_stats.totalWrites++;
            s_stats.bytesWritten += size;
        }
        else {
            s_stats.failedWrites++;
        }

        return success;
    }

    // ===== Address Translation =====

    std::optional<uint64_t> DMA::GetProcessDTB(DWORD processId) {
        // Check cache first
        if (s_dtbCachingEnabled) {
            auto it = s_dtbCache.find(processId);
            if (it != s_dtbCache.end()) {
                return it->second;
            }
        }

        // This is a simplified version
        // Real implementation would:
        // 1. Find EPROCESS structure
        // 2. Read DirectoryTableBase (DTB/CR3) from EPROCESS
        // 3. Cache the result

        // For now, return a placeholder
        // In production, this would scan kernel memory for EPROCESS
        return std::nullopt;
    }

    std::optional<uint64_t> DMA::VirtualToPhysical(DWORD processId, uintptr_t virtualAddress) {
        auto dtb = GetProcessDTB(processId);
        if (!dtb.has_value()) {
            return std::nullopt;
        }

        // Translate using page tables
        uint64_t physicalAddress = TranslateAddress(dtb.value(), virtualAddress);
        if (physicalAddress == 0) {
            return std::nullopt;
        }

        return physicalAddress;
    }

    uint64_t DMA::TranslateAddress(uint64_t dtb, uintptr_t virtualAddress) {
        // Simplified page table walk
        // Real implementation would walk through:
        // PML4 -> PDPT -> PD -> PT -> Physical Address

        // This is a framework - actual implementation needs:
        // 1. Extract page table indices from virtual address
        // 2. Read PML4 entry
        // 3. Read PDPT entry
        // 4. Read PD entry
        // 5. Read PT entry
        // 6. Combine with page offset

        return 0; // Placeholder
    }

    // ===== Virtual Memory Access =====

    bool DMA::ReadVirtual(DWORD processId, uintptr_t virtualAddress, void* buffer, size_t size) {
        auto physicalAddress = VirtualToPhysical(processId, virtualAddress);
        if (!physicalAddress.has_value()) {
            return false;
        }

        return ReadPhysical(physicalAddress.value(), buffer, size);
    }

    bool DMA::WriteVirtual(DWORD processId, uintptr_t virtualAddress, const void* buffer, size_t size) {
        auto physicalAddress = VirtualToPhysical(processId, virtualAddress);
        if (!physicalAddress.has_value()) {
            return false;
        }

        return WritePhysical(physicalAddress.value(), buffer, size);
    }

    std::vector<BYTE> DMA::ReadBytes(DWORD processId, uintptr_t virtualAddress, size_t size) {
        std::vector<BYTE> buffer(size);
        if (ReadVirtual(processId, virtualAddress, buffer.data(), size)) {
            return buffer;
        }
        return {};
    }

    // ===== Process Operations =====

    DWORD DMA::FindProcess(const std::wstring& processName) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return 0;
        }

        PROCESSENTRY32W pe = { sizeof(pe) };
        if (Process32FirstW(snapshot, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, processName.c_str()) == 0) {
                    CloseHandle(snapshot);
                    return pe.th32ProcessID;
                }
            } while (Process32NextW(snapshot, &pe));
        }

        CloseHandle(snapshot);
        return 0;
    }

    std::vector<std::pair<DWORD, std::wstring>> DMA::GetProcessList() {
        std::vector<std::pair<DWORD, std::wstring>> processes;

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return processes;
        }

        PROCESSENTRY32W pe = { sizeof(pe) };
        if (Process32FirstW(snapshot, &pe)) {
            do {
                processes.push_back({ pe.th32ProcessID, pe.szExeFile });
            } while (Process32NextW(snapshot, &pe));
        }

        CloseHandle(snapshot);
        return processes;
    }

    std::optional<uintptr_t> DMA::GetModuleBase(DWORD processId, const std::wstring& moduleName) {
        // Simplified - real implementation would scan PEB
        // For now, use toolhelp32
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return std::nullopt;
        }

        MODULEENTRY32W me = { sizeof(me) };
        if (Module32FirstW(snapshot, &me)) {
            do {
                if (_wcsicmp(me.szModule, moduleName.c_str()) == 0) {
                    CloseHandle(snapshot);
                    return reinterpret_cast<uintptr_t>(me.modBaseAddr);
                }
            } while (Module32NextW(snapshot, &me));
        }

        CloseHandle(snapshot);
        return std::nullopt;
    }

    std::optional<size_t> DMA::GetModuleSize(DWORD processId, const std::wstring& moduleName) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return std::nullopt;
        }

        MODULEENTRY32W me = { sizeof(me) };
        if (Module32FirstW(snapshot, &me)) {
            do {
                if (_wcsicmp(me.szModule, moduleName.c_str()) == 0) {
                    CloseHandle(snapshot);
                    return me.modBaseSize;
                }
            } while (Module32NextW(snapshot, &me));
        }

        CloseHandle(snapshot);
        return std::nullopt;
    }

    std::vector<std::pair<std::wstring, uintptr_t>> DMA::GetModuleList(DWORD processId) {
        std::vector<std::pair<std::wstring, uintptr_t>> modules;

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return modules;
        }

        MODULEENTRY32W me = { sizeof(me) };
        if (Module32FirstW(snapshot, &me)) {
            do {
                modules.push_back({ me.szModule, reinterpret_cast<uintptr_t>(me.modBaseAddr) });
            } while (Module32NextW(snapshot, &me));
        }

        CloseHandle(snapshot);
        return modules;
    }

    // ===== Pattern Scanning =====

    bool DMA::ParsePattern(const std::string& pattern, std::vector<BYTE>& bytes, std::vector<bool>& mask) {
        bytes.clear();
        mask.clear();

        std::string temp;
        for (char c : pattern) {
            if (c == ' ') continue;
            temp += c;

            if (temp.length() == 2) {
                if (temp == "??") {
                    bytes.push_back(0);
                    mask.push_back(false);
                }
                else {
                    bytes.push_back(static_cast<BYTE>(strtol(temp.c_str(), nullptr, 16)));
                    mask.push_back(true);
                }
                temp.clear();
            }
        }

        return !bytes.empty();
    }

    std::optional<uintptr_t> DMA::FindPattern(DWORD processId, const std::wstring& moduleName, const std::string& pattern) {
        auto baseAddr = GetModuleBase(processId, moduleName);
        auto moduleSize = GetModuleSize(processId, moduleName);

        if (!baseAddr.has_value() || !moduleSize.has_value()) {
            return std::nullopt;
        }

        return FindPatternInRange(processId, baseAddr.value(), moduleSize.value(), pattern);
    }

    std::optional<uintptr_t> DMA::FindPatternInRange(DWORD processId, uintptr_t start, size_t size, const std::string& pattern) {
        std::vector<BYTE> patternBytes;
        std::vector<bool> mask;

        if (!ParsePattern(pattern, patternBytes, mask)) {
            return std::nullopt;
        }

        // Read memory in chunks
        const size_t chunkSize = 0x10000; // 64KB chunks
        std::vector<BYTE> buffer(chunkSize);

        for (size_t offset = 0; offset < size; offset += chunkSize) {
            size_t readSize = (std::min)(chunkSize, size - offset);
            if (!ReadVirtual(processId, start + offset, buffer.data(), readSize)) {
                continue;
            }

            // Search for pattern
            for (size_t i = 0; i < readSize - patternBytes.size(); ++i) {
                bool found = true;
                for (size_t j = 0; j < patternBytes.size(); ++j) {
                    if (mask[j] && buffer[i + j] != patternBytes[j]) {
                        found = false;
                        break;
                    }
                }

                if (found) {
                    return start + offset + i;
                }
            }
        }

        return std::nullopt;
    }

    std::vector<uintptr_t> DMA::FindAllPatterns(DWORD processId, const std::wstring& moduleName, const std::string& pattern, size_t maxResults) {
        std::vector<uintptr_t> results;
        // Implementation similar to FindPattern but collect all matches
        return results;
    }

    // ===== Scatter Operations =====

    bool DMA::ScatterRead(DWORD processId, std::vector<ScatterEntry>& entries) {
        // Read multiple addresses in one operation
        // This is a performance optimization
        bool allSuccess = true;

        for (auto& entry : entries) {
            entry.success = ReadVirtual(processId, entry.address, entry.buffer, entry.size);
            if (!entry.success) {
                allSuccess = false;
            }
        }

        return allSuccess;
    }

    bool DMA::ScatterWrite(DWORD processId, std::vector<ScatterEntry>& entries) {
        bool allSuccess = true;

        for (auto& entry : entries) {
            entry.success = WriteVirtual(processId, entry.address, entry.buffer, entry.size);
            if (!entry.success) {
                allSuccess = false;
            }
        }

        return allSuccess;
    }

    // ===== Caching =====

    void DMA::SetDTBCaching(bool enabled) {
        s_dtbCachingEnabled = enabled;
        if (!enabled) {
            s_dtbCache.clear();
        }
    }

    void DMA::ClearDTBCache() {
        s_dtbCache.clear();
    }

    // ===== Statistics =====

    DMA::Statistics DMA::GetStatistics() {
        return s_stats;
    }

    void DMA::ResetStatistics() {
        s_stats = {};
    }

} // namespace WarlockLib
