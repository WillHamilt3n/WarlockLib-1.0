#include "Memory.h"
#include <chrono>

namespace WarlockLib {

    // ===== Basic Memory Operations =====

    std::vector<byte> Memory::ReadBytes(HANDLE processHandle, uintptr_t address, size_t size) {
        std::vector<byte> buffer(size);
        SIZE_T bytesRead;
        if (ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(address), buffer.data(), size, &bytesRead)) {
            buffer.resize(bytesRead);
            return buffer;
        }
        return {};
    }

    bool Memory::WriteBytes(HANDLE processHandle, uintptr_t address, const std::vector<byte>& bytes) {
        SIZE_T bytesWritten;
        return WriteProcessMemory(processHandle, reinterpret_cast<LPVOID>(address), bytes.data(), bytes.size(), &bytesWritten)
            && bytesWritten == bytes.size();
    }

    std::optional<uintptr_t> Memory::FollowPointerChain(HANDLE processHandle, uintptr_t baseAddress, const std::vector<uintptr_t>& offsets) {
        uintptr_t address = baseAddress;
        
        for (size_t i = 0; i < offsets.size(); ++i) {
            auto value = Read<uintptr_t>(processHandle, address + offsets[i]);
            if (!value.has_value()) {
                return std::nullopt;
            }
            address = value.value();
            
            // Don't dereference on the last offset
            if (i == offsets.size() - 1) {
                return address;
            }
        }
        
        return address;
    }

    uintptr_t Memory::FollowPointerChainInternal(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets) {
        uintptr_t address = baseAddress;
        
        for (size_t i = 0; i < offsets.size(); ++i) {
            address = *reinterpret_cast<uintptr_t*>(address + offsets[i]);
            
            // Don't dereference on the last offset
            if (i == offsets.size() - 1) {
                return address;
            }
        }
        
        return address;
    }

    std::optional<MEMORY_BASIC_INFORMATION> Memory::QueryMemory(HANDLE processHandle, uintptr_t address) {
        MEMORY_BASIC_INFORMATION mbi{};
        if (VirtualQueryEx(processHandle, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi))) {
            return mbi;
        }
        return std::nullopt;
    }

    std::vector<MEMORY_BASIC_INFORMATION> Memory::GetMemoryRegions(HANDLE processHandle) {
        std::vector<MEMORY_BASIC_INFORMATION> regions;
        MEMORY_BASIC_INFORMATION mbi{};
        uintptr_t address = 0;

        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        uintptr_t maxAddress = reinterpret_cast<uintptr_t>(sysInfo.lpMaximumApplicationAddress);

        while (address < maxAddress) {
            if (VirtualQueryEx(processHandle, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi))) {
                regions.push_back(mbi);
                address += mbi.RegionSize;
            }
            else {
                break;
            }
        }

        return regions;
    }

    uintptr_t Memory::AllocateMemory(HANDLE processHandle, size_t size, DWORD protection) {
        LPVOID allocatedMemory = VirtualAllocEx(processHandle, nullptr, size, MEM_COMMIT | MEM_RESERVE, protection);
        return reinterpret_cast<uintptr_t>(allocatedMemory);
    }

    bool Memory::FreeMemory(HANDLE processHandle, uintptr_t address) {
        return VirtualFreeEx(processHandle, reinterpret_cast<LPVOID>(address), 0, MEM_RELEASE) != 0;
    }

    // ===== Memory Freezing =====

    void Memory::StartFreezer() {
        s_freezerRunning = true;
        s_freezerThread = std::thread(FreezerLoop);
    }

    void Memory::StopFreezer() {
        s_freezerRunning = false;
        if (s_freezerThread.joinable()) {
            s_freezerThread.join();
        }
    }

    void Memory::FreezerLoop() {
        while (s_freezerRunning) {
            {
                std::lock_guard<std::mutex> lock(s_freezerMutex);
                for (const auto& pair : s_frozenValues) {
                    WriteFrozenValue(pair.second);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    void Memory::WriteFrozenValue(const FrozenValue& frozen) {
        if (frozen.processHandle) {
            WriteBytes(frozen.processHandle, frozen.address, frozen.value);
        }
        else {
            // Internal write
            DWORD oldProtection;
            VirtualProtect(reinterpret_cast<LPVOID>(frozen.address), frozen.size, PAGE_EXECUTE_READWRITE, &oldProtection);
            memcpy(reinterpret_cast<void*>(frozen.address), frozen.value.data(), frozen.size);
            VirtualProtect(reinterpret_cast<LPVOID>(frozen.address), frozen.size, oldProtection, &oldProtection);
        }
    }

    bool Memory::Unfreeze(uintptr_t address) {
        std::lock_guard<std::mutex> lock(s_freezerMutex);
        auto it = s_frozenValues.find(address);
        if (it != s_frozenValues.end()) {
            s_frozenValues.erase(it);
            if (s_frozenValues.empty()) {
                StopFreezer();
            }
            return true;
        }
        return false;
    }

    void Memory::UnfreezeAll() {
        std::lock_guard<std::mutex> lock(s_freezerMutex);
        s_frozenValues.clear();
        StopFreezer();
    }

    bool Memory::IsFrozen(uintptr_t address) {
        std::lock_guard<std::mutex> lock(s_freezerMutex);
        return s_frozenValues.find(address) != s_frozenValues.end();
    }

    size_t Memory::GetFrozenCount() {
        std::lock_guard<std::mutex> lock(s_freezerMutex);
        return s_frozenValues.size();
    }

    // ===== Code Patching =====

    bool Memory::Nop(uintptr_t address, size_t count) {
        std::vector<byte> nops(count, 0x90); // 0x90 = NOP instruction
        
        DWORD oldProtection;
        if (VirtualProtect(reinterpret_cast<LPVOID>(address), count, PAGE_EXECUTE_READWRITE, &oldProtection)) {
            memcpy(reinterpret_cast<void*>(address), nops.data(), count);
            VirtualProtect(reinterpret_cast<LPVOID>(address), count, oldProtection, &oldProtection);
            return true;
        }
        return false;
    }

    bool Memory::RestoreBytes(uintptr_t address, const std::vector<byte>& originalBytes) {
        DWORD oldProtection;
        if (VirtualProtect(reinterpret_cast<LPVOID>(address), originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtection)) {
            memcpy(reinterpret_cast<void*>(address), originalBytes.data(), originalBytes.size());
            VirtualProtect(reinterpret_cast<LPVOID>(address), originalBytes.size(), oldProtection, &oldProtection);
            return true;
        }
        return false;
    }

    std::vector<byte> Memory::SaveBytes(uintptr_t address, size_t count) {
        std::vector<byte> bytes(count);
        memcpy(bytes.data(), reinterpret_cast<void*>(address), count);
        return bytes;
    }

    // ===== Inline Hook Implementation =====

    Memory::InlineHook::InlineHook(uintptr_t targetFunction, uintptr_t hookFunction)
        : m_targetFunction(targetFunction)
        , m_hookFunction(hookFunction)
        , m_trampoline(0)
        , m_originalFunction(targetFunction)
        , m_installed(false)
    {
    }

    Memory::InlineHook::~InlineHook() {
        if (m_installed) {
            Uninstall();
        }
        if (m_trampoline) {
            VirtualFree(reinterpret_cast<LPVOID>(m_trampoline), 0, MEM_RELEASE);
        }
    }

    bool Memory::InlineHook::Install() {
        if (m_installed) {
            return true;
        }

        // Save original bytes
        m_originalBytes.resize(14);
        memcpy(m_originalBytes.data(), reinterpret_cast<void*>(m_targetFunction), 14);

        // Create trampoline
        if (!CreateTrampoline()) {
            return false;
        }

        // Write jump at target
        auto jumpBytes = CreateJump(m_targetFunction, m_hookFunction);
        
        DWORD oldProtection;
        if (VirtualProtect(reinterpret_cast<LPVOID>(m_targetFunction), jumpBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtection)) {
            memcpy(reinterpret_cast<void*>(m_targetFunction), jumpBytes.data(), jumpBytes.size());
            VirtualProtect(reinterpret_cast<LPVOID>(m_targetFunction), jumpBytes.size(), oldProtection, &oldProtection);
            m_installed = true;
            return true;
        }

        return false;
    }

    bool Memory::InlineHook::Uninstall() {
        if (!m_installed) {
            return true;
        }

        DWORD oldProtection;
        if (VirtualProtect(reinterpret_cast<LPVOID>(m_targetFunction), m_originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtection)) {
            memcpy(reinterpret_cast<void*>(m_targetFunction), m_originalBytes.data(), m_originalBytes.size());
            VirtualProtect(reinterpret_cast<LPVOID>(m_targetFunction), m_originalBytes.size(), oldProtection, &oldProtection);
            m_installed = false;
            return true;
        }

        return false;
    }

    bool Memory::InlineHook::CreateTrampoline() {
        m_trampoline = reinterpret_cast<uintptr_t>(VirtualAlloc(nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
        if (!m_trampoline) {
            return false;
        }

        // Copy original bytes to trampoline
        memcpy(reinterpret_cast<void*>(m_trampoline), m_originalBytes.data(), 14);

        // Add jump back to original function + 14
        auto jumpBack = CreateJump(m_trampoline + 14, m_originalFunction + 14);
        memcpy(reinterpret_cast<void*>(m_trampoline + 14), jumpBack.data(), jumpBack.size());

        return true;
    }

    std::vector<byte> Memory::InlineHook::CreateJump(uintptr_t from, uintptr_t to) {
        std::vector<byte> jump;

        // For x64, use absolute jump
        // FF 25 00 00 00 00 [8 byte address]
        jump.push_back(0xFF);
        jump.push_back(0x25);
        jump.push_back(0x00);
        jump.push_back(0x00);
        jump.push_back(0x00);
        jump.push_back(0x00);

        // Add target address
        for (int i = 0; i < 8; i++) {
            jump.push_back(static_cast<byte>((to >> (i * 8)) & 0xFF));
        }

        return jump;
    }

    // ===== VTable Hook Implementation =====

    Memory::VTableHook::VTableHook(void* instance)
        : m_instance(instance)
        , m_originalVTable(nullptr)
        , m_newVTable(nullptr)
        , m_vTableSize(0)
    {
        m_originalVTable = *reinterpret_cast<uintptr_t**>(instance);
        m_vTableSize = GetVTableSize();

        // Create copy of VTable
        m_newVTable = new uintptr_t[m_vTableSize];
        memcpy(m_newVTable, m_originalVTable, m_vTableSize * sizeof(uintptr_t));

        // Replace VTable pointer
        *reinterpret_cast<uintptr_t**>(instance) = m_newVTable;
    }

    Memory::VTableHook::~VTableHook() {
        if (m_instance && m_originalVTable) {
            *reinterpret_cast<uintptr_t**>(m_instance) = m_originalVTable;
        }
        if (m_newVTable) {
            delete[] m_newVTable;
        }
    }

    bool Memory::VTableHook::HookFunction(size_t index, uintptr_t hookFunction) {
        if (index >= m_vTableSize) {
            return false;
        }

        m_newVTable[index] = hookFunction;
        m_hookedIndices.push_back(index);
        return true;
    }

    bool Memory::VTableHook::UnhookFunction(size_t index) {
        if (index >= m_vTableSize) {
            return false;
        }

        m_newVTable[index] = m_originalVTable[index];
        
        auto it = std::find(m_hookedIndices.begin(), m_hookedIndices.end(), index);
        if (it != m_hookedIndices.end()) {
            m_hookedIndices.erase(it);
        }

        return true;
    }

    void Memory::VTableHook::UnhookAll() {
        for (size_t index : m_hookedIndices) {
            m_newVTable[index] = m_originalVTable[index];
        }
        m_hookedIndices.clear();
    }

    uintptr_t Memory::VTableHook::GetOriginalFunction(size_t index) const {
        if (index >= m_vTableSize) {
            return 0;
        }
        return m_originalVTable[index];
    }

    size_t Memory::VTableHook::GetVTableSize() {
        size_t size = 0;
        while (m_originalVTable[size]) {
            size++;
        }
        return size;
    }

} // namespace WarlockLib
