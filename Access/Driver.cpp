#include "Driver.h"

namespace WarlockLib {

    Driver::Driver() : m_driverHandle(INVALID_HANDLE_VALUE), m_targetProcessId(0), m_encryptionMode(ENCRYPT_NONE) {
    }

    Driver::~Driver() {
        Disconnect();
    }

    bool Driver::Connect(const std::wstring& driverName) {
        // Construct device path: \\.\DriverName
        std::wstring devicePath = L"\\\\.\\" + driverName;

        m_driverHandle = CreateFileW(
            devicePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        return m_driverHandle != INVALID_HANDLE_VALUE;
    }

    void Driver::Disconnect() {
        if (m_driverHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_driverHandle);
            m_driverHandle = INVALID_HANDLE_VALUE;
        }
        m_targetProcessId = 0;
    }

    bool Driver::ReadMemory(uintptr_t address, PVOID buffer, SIZE_T size) {
        if (!IsConnected() || m_targetProcessId == 0) {
            return false;
        }

        ReadWriteRequest request{};
        request.processId = m_targetProcessId;
        request.address = address;
        request.buffer = buffer;
        request.size = size;

        DWORD bytesReturned = 0;
        return DeviceIoControl(
            m_driverHandle,
            IOCTL_READ_MEMORY,
            &request,
            sizeof(request),
            buffer,
            static_cast<DWORD>(size),
            &bytesReturned,
            nullptr
        ) && bytesReturned == size;
    }

    bool Driver::WriteMemory(uintptr_t address, PVOID buffer, SIZE_T size) {
        if (!IsConnected() || m_targetProcessId == 0) {
            return false;
        }

        ReadWriteRequest request{};
        request.processId = m_targetProcessId;
        request.address = address;
        request.buffer = buffer;
        request.size = size;

        DWORD bytesReturned = 0;
        return DeviceIoControl(
            m_driverHandle,
            IOCTL_WRITE_MEMORY,
            &request,
            sizeof(request),
            nullptr,
            0,
            &bytesReturned,
            nullptr
        ) != 0;
    }

    std::optional<uintptr_t> Driver::GetModuleBase(const std::wstring& moduleName) {
        if (!IsConnected() || m_targetProcessId == 0) {
            return std::nullopt;
        }

        ModuleRequest request{};
        request.processId = m_targetProcessId;
        wcsncpy_s(request.moduleName, moduleName.c_str(), _TRUNCATE);

        DWORD bytesReturned = 0;
        if (DeviceIoControl(
            m_driverHandle,
            IOCTL_GET_MODULE_BASE,
            &request,
            sizeof(request),
            &request,
            sizeof(request),
            &bytesReturned,
            nullptr
        )) {
            return request.moduleBase;
        }

        return std::nullopt;
    }

    std::optional<SIZE_T> Driver::GetModuleSize(const std::wstring& moduleName) {
        if (!IsConnected() || m_targetProcessId == 0) {
            return std::nullopt;
        }

        ModuleRequest request{};
        request.processId = m_targetProcessId;
        wcsncpy_s(request.moduleName, moduleName.c_str(), _TRUNCATE);

        DWORD bytesReturned = 0;
        if (DeviceIoControl(
            m_driverHandle,
            IOCTL_GET_MODULE_BASE,
            &request,
            sizeof(request),
            &request,
            sizeof(request),
            &bytesReturned,
            nullptr
        )) {
            return request.moduleSize;
        }

        return std::nullopt;
    }

    std::optional<uintptr_t> Driver::AllocateMemory(SIZE_T size, DWORD protection) {
        if (!IsConnected() || m_targetProcessId == 0) {
            return std::nullopt;
        }

        AllocateRequest request{};
        request.processId = m_targetProcessId;
        request.size = size;
        request.protection = protection;

        DWORD bytesReturned = 0;
        if (DeviceIoControl(
            m_driverHandle,
            IOCTL_ALLOCATE_MEMORY,
            &request,
            sizeof(request),
            &request,
            sizeof(request),
            &bytesReturned,
            nullptr
        )) {
            return request.allocatedAddress;
        }

        return std::nullopt;
    }

    bool Driver::FreeMemory(uintptr_t address) {
        if (!IsConnected() || m_targetProcessId == 0) {
            return false;
        }

        AllocateRequest request{};
        request.processId = m_targetProcessId;
        request.allocatedAddress = address;

        DWORD bytesReturned = 0;
        return DeviceIoControl(
            m_driverHandle,
            IOCTL_FREE_MEMORY,
            &request,
            sizeof(request),
            nullptr,
            0,
            &bytesReturned,
            nullptr
        ) != 0;
    }

    bool Driver::ProtectMemory(uintptr_t address, SIZE_T size, DWORD newProtection, DWORD* oldProtection) {
        if (!IsConnected() || m_targetProcessId == 0) {
            return false;
        }

        ProtectRequest request{};
        request.processId = m_targetProcessId;
        request.address = address;
        request.size = size;
        request.newProtection = newProtection;

        DWORD bytesReturned = 0;
        bool result = DeviceIoControl(
            m_driverHandle,
            IOCTL_PROTECT_MEMORY,
            &request,
            sizeof(request),
            &request,
            sizeof(request),
            &bytesReturned,
            nullptr
        ) != 0;

        if (result && oldProtection) {
            *oldProtection = request.oldProtection;
        }

        return result;
    }

    bool Driver::SendIOCTL(DWORD ioctl, PVOID inputBuffer, DWORD inputSize, PVOID outputBuffer, DWORD outputSize, DWORD* bytesReturned) {
        if (!IsConnected()) {
            return false;
        }

        // Encrypt input if encryption is enabled
        if (m_encryptionMode != ENCRYPT_NONE && inputBuffer && inputSize > 0) {
            EncryptBuffer(inputBuffer, inputSize);
        }

        DWORD bytes = 0;
        bool result = DeviceIoControl(
            m_driverHandle,
            ioctl,
            inputBuffer,
            inputSize,
            outputBuffer,
            outputSize,
            &bytes,
            nullptr
        ) != 0;

        // Decrypt output if encryption is enabled
        if (result && m_encryptionMode != ENCRYPT_NONE && outputBuffer && outputSize > 0) {
            DecryptBuffer(outputBuffer, outputSize);
        }

        if (bytesReturned) {
            *bytesReturned = bytes;
        }

        return result;
    }

    // ===== IOCTL Encryption =====

    void Driver::SetEncryption(EncryptionMode mode, const std::vector<BYTE>& key) {
        m_encryptionMode = mode;
        if (!key.empty()) {
            m_encryptionKey = key;
        }
        else {
            // Default key if none provided
            m_encryptionKey = { 0x42, 0x13, 0x37, 0xDE, 0xAD, 0xBE, 0xEF, 0xCA };
        }
    }

    void Driver::SetEncryptionKey(const std::vector<BYTE>& key) {
        m_encryptionKey = key;
    }

    void Driver::EncryptBuffer(PVOID buffer, DWORD size) {
        if (m_encryptionMode == ENCRYPT_XOR) {
            BYTE* data = static_cast<BYTE*>(buffer);
            for (DWORD i = 0; i < size; ++i) {
                data[i] ^= m_encryptionKey[i % m_encryptionKey.size()];
            }
        }
        // AES implementation would go here
    }

    void Driver::DecryptBuffer(PVOID buffer, DWORD size) {
        // XOR is symmetric, so same as encrypt
        EncryptBuffer(buffer, size);
    }

} // namespace WarlockLib
