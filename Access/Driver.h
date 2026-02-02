#pragma once

#ifndef DRIVER_H
#define DRIVER_H

#include <Windows.h>
#include <string>
#include <optional>
#include <vector>

namespace WarlockLib {

    /**
     * Driver class for kernel-mode communication
     * Used for bypassing user-mode anti-cheat protection
     */
    class Driver {
    public:
        // IOCTL codes - Define your driver's control codes
        enum IOCTLCode : DWORD {
            IOCTL_READ_MEMORY = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS),
            IOCTL_WRITE_MEMORY = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS),
            IOCTL_GET_MODULE_BASE = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS),
            IOCTL_ALLOCATE_MEMORY = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS),
            IOCTL_FREE_MEMORY = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS),
            IOCTL_PROTECT_MEMORY = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS),
        };

        // Encryption modes
        enum EncryptionMode {
            ENCRYPT_NONE = 0,
            ENCRYPT_XOR = 1,
            ENCRYPT_AES = 2  // Future implementation
        };

        // Communication structures
        struct ReadWriteRequest {
            DWORD processId;
            uintptr_t address;
            PVOID buffer;
            SIZE_T size;
        };

        struct ModuleRequest {
            DWORD processId;
            wchar_t moduleName[256];
            uintptr_t moduleBase;
            SIZE_T moduleSize;
        };

        struct AllocateRequest {
            DWORD processId;
            SIZE_T size;
            DWORD protection;
            uintptr_t allocatedAddress;
        };

        struct ProtectRequest {
            DWORD processId;
            uintptr_t address;
            SIZE_T size;
            DWORD newProtection;
            DWORD oldProtection;
        };

        Driver();
        ~Driver();

        // Driver connection
        bool Connect(const std::wstring& driverName);
        void Disconnect();
        bool IsConnected() const { return m_driverHandle != INVALID_HANDLE_VALUE; }

        // Set target process
        void SetTargetProcess(DWORD processId) { m_targetProcessId = processId; }
        DWORD GetTargetProcess() const { return m_targetProcessId; }

        // Memory operations through driver
        template<typename T>
        std::optional<T> Read(uintptr_t address) {
            T value{};
            if (ReadMemory(address, &value, sizeof(T))) {
                return value;
            }
            return std::nullopt;
        }

        template<typename T>
        bool Write(uintptr_t address, const T& value) {
            return WriteMemory(address, (PVOID)&value, sizeof(T));
        }

        bool ReadMemory(uintptr_t address, PVOID buffer, SIZE_T size);
        bool WriteMemory(uintptr_t address, PVOID buffer, SIZE_T size);

        // Module operations
        std::optional<uintptr_t> GetModuleBase(const std::wstring& moduleName);
        std::optional<SIZE_T> GetModuleSize(const std::wstring& moduleName);

        // Memory management
        std::optional<uintptr_t> AllocateMemory(SIZE_T size, DWORD protection = PAGE_EXECUTE_READWRITE);
        bool FreeMemory(uintptr_t address);
        bool ProtectMemory(uintptr_t address, SIZE_T size, DWORD newProtection, DWORD* oldProtection = nullptr);

        // Get driver handle for custom IOCTL calls
        HANDLE GetHandle() const { return m_driverHandle; }

        // Send custom IOCTL
        bool SendIOCTL(DWORD ioctl, PVOID inputBuffer, DWORD inputSize, PVOID outputBuffer, DWORD outputSize, DWORD* bytesReturned = nullptr);

        // ===== IOCTL Encryption =====

        // Enable/disable IOCTL encryption
        void SetEncryption(EncryptionMode mode, const std::vector<BYTE>& key = {});
        
        // Set encryption key
        void SetEncryptionKey(const std::vector<BYTE>& key);
        
        // Check if encryption is enabled
        bool IsEncryptionEnabled() const { return m_encryptionMode != ENCRYPT_NONE; }

    private:
        HANDLE m_driverHandle;
        DWORD m_targetProcessId;
        
        // Encryption
        EncryptionMode m_encryptionMode;
        std::vector<BYTE> m_encryptionKey;
        
        // Encrypt/decrypt buffer
        void EncryptBuffer(PVOID buffer, DWORD size);
        void DecryptBuffer(PVOID buffer, DWORD size);
        
    };

} // namespace WarlockLib

#endif // DRIVER_H
