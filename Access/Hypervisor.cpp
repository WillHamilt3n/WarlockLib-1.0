#include "Hypervisor.h"
#include <intrin.h>
#include <cstring>
#include <string>

namespace MemLib {

    // ===== CPU Detection =====

    bool Hypervisor::IsSupportedCPU() {
        int cpuInfo[4] = {};
        __cpuid(cpuInfo, 0);

        // Check for Intel or AMD
        char vendor[13] = {};
        *reinterpret_cast<int*>(vendor) = cpuInfo[1];
        *reinterpret_cast<int*>(vendor + 4) = cpuInfo[3];
        *reinterpret_cast<int*>(vendor + 8) = cpuInfo[2];

        std::string vendorStr(vendor);
        return vendorStr == "GenuineIntel" || vendorStr == "AuthenticAMD";
    }

    bool Hypervisor::IsVirtualizationEnabled() {
        int cpuInfo[4] = {};

        // Check CPUID.1:ECX[5] for VMX (Intel VT-x)
        __cpuid(cpuInfo, 1);
        bool vmxSupport = (cpuInfo[2] & (1 << 5)) != 0;

        if (vmxSupport) {
            // Check if VMX is enabled in IA32_FEATURE_CONTROL MSR
            uint64_t featureControl = __readmsr(0x3A);
            bool vmxLocked = (featureControl & 1) != 0;
            bool vmxEnabled = (featureControl & 4) != 0;
            return vmxLocked && vmxEnabled;
        }

        // Check for AMD-V (SVM)
        __cpuid(cpuInfo, 0x80000001);
        bool svmSupport = (cpuInfo[2] & (1 << 2)) != 0;

        if (svmSupport) {
            // Check if SVM is enabled
            uint64_t vmCr = __readmsr(0xC0010114);
            return (vmCr & (1 << 4)) != 0;
        }

        return false;
    }

    std::string Hypervisor::GetCPUVendor() {
        int cpuInfo[4] = {};
        __cpuid(cpuInfo, 0);

        char vendor[13] = {};
        *reinterpret_cast<int*>(vendor) = cpuInfo[1];
        *reinterpret_cast<int*>(vendor + 4) = cpuInfo[3];
        *reinterpret_cast<int*>(vendor + 8) = cpuInfo[2];

        return std::string(vendor);
    }

    std::string Hypervisor::GetVirtualizationTech() {
        std::string vendor = GetCPUVendor();
        if (vendor == "GenuineIntel") {
            return "Intel VT-x";
        }
        else if (vendor == "AuthenticAMD") {
            return "AMD-V (SVM)";
        }
        return "Unknown";
    }

    bool Hypervisor::IsRunningUnderHypervisor() {
        int cpuInfo[4] = {};
        __cpuid(cpuInfo, 1);
        
        // Check CPUID.1:ECX[31] - hypervisor present bit
        return (cpuInfo[2] & (1 << 31)) != 0;
    }

    // ===== Hypervisor Loading =====

    bool Hypervisor::LoadHypervisor() {
        if (s_loaded) {
            return true;
        }

        // Check CPU support
        if (!IsSupportedCPU()) {
            return false;
        }

        if (!IsVirtualizationEnabled()) {
            return false;
        }

        // Determine CPU type and initialize
        std::string vendor = GetCPUVendor();
        bool success = false;

        if (vendor == "GenuineIntel") {
            success = InitializeVTx();
        }
        else if (vendor == "AuthenticAMD") {
            success = InitializeSVM();
        }

        if (success) {
            // Setup default handlers
            HookVMExit(EXIT_CPUID, [](GuestRegisters& regs) {
                HandleCPUID(regs);
                return true;
            });

            HookVMExit(EXIT_RDTSC, [](GuestRegisters& regs) {
                HandleRDTSC(regs);
                return true;
            });

            s_loaded = true;
        }

        return success;
    }

    void Hypervisor::UnloadHypervisor() {
        if (!s_loaded) {
            return;
        }

        // Clear all hooks
        ClearVMExitHooks();

        // Unhide all pages
        for (auto& pair : s_hiddenPages) {
            UnhideMemoryPage(pair.first);
        }

        // Remove all shadows
        for (auto& pair : s_shadowPages) {
            RemoveShadow(pair.first);
        }

        // Exit VMX mode
        // __vmx_off(); // This would require actual VMX implementation

        s_loaded = false;
    }

    bool Hypervisor::IsLoaded() {
        return s_loaded;
    }

    std::string Hypervisor::GetStatus() {
        if (!s_loaded) {
            return "Not loaded";
        }

        return "Loaded (" + GetVirtualizationTech() + ")";
    }

    // ===== Initialization Helpers =====

    bool Hypervisor::InitializeVTx() {
        // This is a simplified framework
        // Real implementation would:
        // 1. Allocate VMXON region
        // 2. Enable VMX via CR4
        // 3. Execute VMXON
        // 4. Allocate VMCS
        // 5. Setup VMCS fields
        // 6. Setup EPT
        // 7. Launch VM

        // For now, this is a placeholder
        // Full implementation requires kernel driver

        return false; // Placeholder - requires kernel mode
    }

    bool Hypervisor::InitializeSVM() {
        // AMD-V initialization
        // Similar to VT-x but uses VMCB instead of VMCS

        return false; // Placeholder - requires kernel mode
    }

    bool Hypervisor::SetupVMCS() {
        // Setup Virtual Machine Control Structure
        // This controls VM execution

        return false; // Placeholder
    }

    bool Hypervisor::SetupEPT() {
        // Setup Extended Page Tables
        // This allows memory hiding/shadowing

        return false; // Placeholder
    }

    // ===== VM Exit Hooks =====

    bool Hypervisor::HookVMExit(VMExitReason reason, VMExitHandler handler) {
        s_vmExitHandlers[reason] = handler;
        return true;
    }

    bool Hypervisor::UnhookVMExit(VMExitReason reason) {
        auto it = s_vmExitHandlers.find(reason);
        if (it != s_vmExitHandlers.end()) {
            s_vmExitHandlers.erase(it);
            return true;
        }
        return false;
    }

    void Hypervisor::ClearVMExitHooks() {
        s_vmExitHandlers.clear();
    }

    // ===== EPT Operations =====

    bool Hypervisor::HideMemoryPage(uintptr_t virtualAddress) {
        if (!s_loaded) {
            return false;
        }

        // Align to page boundary
        uintptr_t pageAddress = virtualAddress & ~0xFFF;

        // Mark as hidden
        s_hiddenPages[pageAddress] = true;

        // Modify EPT entry to hide page
        return ModifyEPTEntry(pageAddress, true);
    }

    bool Hypervisor::UnhideMemoryPage(uintptr_t virtualAddress) {
        uintptr_t pageAddress = virtualAddress & ~0xFFF;

        auto it = s_hiddenPages.find(pageAddress);
        if (it != s_hiddenPages.end()) {
            s_hiddenPages.erase(it);
            return ModifyEPTEntry(pageAddress, false);
        }

        return false;
    }

    bool Hypervisor::ShadowMemoryPage(uintptr_t virtualAddress, void* shadowData, size_t size) {
        if (!s_loaded) {
            return false;
        }

        uintptr_t pageAddress = virtualAddress & ~0xFFF;

        // Copy shadow data
        std::vector<BYTE> shadow(size);
        memcpy(shadow.data(), shadowData, size);
        s_shadowPages[pageAddress] = shadow;

        // Setup EPT to redirect reads to shadow page
        return ModifyEPTEntry(pageAddress, true);
    }

    bool Hypervisor::RemoveShadow(uintptr_t virtualAddress) {
        uintptr_t pageAddress = virtualAddress & ~0xFFF;

        auto it = s_shadowPages.find(pageAddress);
        if (it != s_shadowPages.end()) {
            s_shadowPages.erase(it);
            return ModifyEPTEntry(pageAddress, false);
        }

        return false;
    }

    bool Hypervisor::IsPageHidden(uintptr_t virtualAddress) {
        uintptr_t pageAddress = virtualAddress & ~0xFFF;
        return s_hiddenPages.find(pageAddress) != s_hiddenPages.end();
    }

    bool Hypervisor::ModifyEPTEntry(uintptr_t virtualAddress, bool hide) {
        // This would modify the EPT entry for the page
        // Real implementation requires:
        // 1. Walk EPT page tables
        // 2. Find PTE for virtual address
        // 3. Modify read/write/execute permissions
        // 4. Invalidate EPT

        // Placeholder
        return true;
    }

    // ===== CPUID Spoofing =====

    bool Hypervisor::EnableCPUIDSpoofing(bool enable) {
        s_cpuidSpoofing = enable;
        return true;
    }

    bool Hypervisor::SetFakeCPUVendor(const std::string& vendor) {
        if (vendor.length() != 12) {
            return false;
        }
        s_fakeCPUVendor = vendor;
        return true;
    }

    void Hypervisor::HandleCPUID(GuestRegisters& regs) {
        int leaf = static_cast<int>(regs.rax);

        if (s_cpuidSpoofing) {
            if (leaf == 0 && !s_fakeCPUVendor.empty()) {
                // Spoof vendor string
                regs.rbx = *reinterpret_cast<const uint32_t*>(s_fakeCPUVendor.c_str());
                regs.rdx = *reinterpret_cast<const uint32_t*>(s_fakeCPUVendor.c_str() + 4);
                regs.rcx = *reinterpret_cast<const uint32_t*>(s_fakeCPUVendor.c_str() + 8);
                return;
            }

            if (leaf == 1) {
                // Hide hypervisor bit (ECX[31])
                int cpuInfo[4] = {};
                __cpuid(cpuInfo, 1);
                regs.rcx = cpuInfo[2] & ~(1 << 31);
                return;
            }

            if (leaf == 0x40000000) {
                // Hide hypervisor vendor
                regs.rax = 0;
                regs.rbx = 0;
                regs.rcx = 0;
                regs.rdx = 0;
                return;
            }
        }

        // Default behavior - pass through to real CPUID
        int cpuInfo[4] = {};
        __cpuidex(cpuInfo, leaf, static_cast<int>(regs.rcx));
        regs.rax = cpuInfo[0];
        regs.rbx = cpuInfo[1];
        regs.rcx = cpuInfo[2];
        regs.rdx = cpuInfo[3];
    }

    // ===== RDTSC Spoofing =====

    bool Hypervisor::EnableRDTSCSpoofing(bool enable) {
        s_rdtscSpoofing = enable;
        return true;
    }

    void Hypervisor::HandleRDTSC(GuestRegisters& regs) {
        if (s_rdtscSpoofing) {
            // Return fake timestamp to defeat timing attacks
            uint64_t fakeTimestamp = __rdtsc();
            // Add some offset to hide VM overhead
            fakeTimestamp -= 1000;

            regs.rax = fakeTimestamp & 0xFFFFFFFF;
            regs.rdx = (fakeTimestamp >> 32) & 0xFFFFFFFF;
        }
        else {
            // Pass through real timestamp
            uint64_t timestamp = __rdtsc();
            regs.rax = timestamp & 0xFFFFFFFF;
            regs.rdx = (timestamp >> 32) & 0xFFFFFFFF;
        }
    }

    bool Hypervisor::HideVMSignatures() {
        // Enable all anti-detection features
        EnableCPUIDSpoofing(true);
        EnableRDTSCSpoofing(true);
        SetFakeCPUVendor("GenuineIntel"); // Pretend to be Intel
        return true;
    }

    // ===== Memory Operations =====

    bool Hypervisor::ReadPhysicalMemory(uint64_t physicalAddress, void* buffer, size_t size) {
        if (!s_loaded) {
            return false;
        }

        // Would use VMCALL or direct physical memory access
        return false; // Placeholder
    }

    bool Hypervisor::WritePhysicalMemory(uint64_t physicalAddress, const void* buffer, size_t size) {
        if (!s_loaded) {
            return false;
        }

        // Would use VMCALL or direct physical memory access
        return false; // Placeholder
    }

    uint64_t Hypervisor::GuestVirtualToPhysical(uintptr_t guestVirtual) {
        // Would walk guest page tables
        return 0; // Placeholder
    }

    // ===== VMCALL Interface =====

    uint64_t Hypervisor::VMCall(uint64_t command, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
        if (!s_loaded) {
            return 0;
        }

        // Execute VMCALL instruction
        // This communicates with the hypervisor
        // __vmx_vmcall would be used here in real implementation

        return 0; // Placeholder
    }

} // namespace MemLib
