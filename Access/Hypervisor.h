#pragma once

#ifndef HYPERVISOR_H
#define HYPERVISOR_H

#include <Windows.h>
#include <functional>
#include <map>
#include <vector>
#include <string>

namespace MemLib {

    /**
     * Hypervisor - Ring -1 access (below kernel)
     * Thin hypervisor using Intel VT-x or AMD-V
     * 
     * Features:
     * - EPT memory hiding (hide DLL pages from scans)
     * - VM exit hooks (CPUID, RDTSC, VMCALL)
     * - Memory shadowing
     * - Anti-VM detection bypass
     */
    class Hypervisor {
    public:
        // VM exit reasons
        enum VMExitReason {
            EXIT_EXCEPTION = 0,
            EXIT_EXTERNAL_INTERRUPT = 1,
            EXIT_TRIPLE_FAULT = 2,
            EXIT_INIT = 3,
            EXIT_SIPI = 4,
            EXIT_IO_SMI = 5,
            EXIT_OTHER_SMI = 6,
            EXIT_INTERRUPT_WINDOW = 7,
            EXIT_NMI_WINDOW = 8,
            EXIT_TASK_SWITCH = 9,
            EXIT_CPUID = 10,
            EXIT_GETSEC = 11,
            EXIT_HLT = 12,
            EXIT_INVD = 13,
            EXIT_INVLPG = 14,
            EXIT_RDPMC = 15,
            EXIT_RDTSC = 16,
            EXIT_RSM = 17,
            EXIT_VMCALL = 18,
            EXIT_VMCLEAR = 19,
            EXIT_VMLAUNCH = 20,
            EXIT_VMPTRLD = 21,
            EXIT_VMPTRST = 22,
            EXIT_VMREAD = 23,
            EXIT_VMRESUME = 24,
            EXIT_VMWRITE = 25,
            EXIT_VMXOFF = 26,
            EXIT_VMXON = 27,
            EXIT_CR_ACCESS = 28,
            EXIT_DR_ACCESS = 29,
            EXIT_IO_INSTRUCTION = 30,
            EXIT_MSR_READ = 31,
            EXIT_MSR_WRITE = 32,
            EXIT_EPT_VIOLATION = 48,
            EXIT_EPT_MISCONFIG = 49,
            EXIT_INVEPT = 50,
            EXIT_RDTSCP = 51,
            EXIT_PREEMPTION_TIMER = 52,
            EXIT_INVVPID = 53,
            EXIT_WBINVD = 54,
            EXIT_XSETBV = 55
        };

        // CPU registers for VM exit handlers
        struct GuestRegisters {
            uint64_t rax, rbx, rcx, rdx;
            uint64_t rsi, rdi, rbp, rsp;
            uint64_t r8, r9, r10, r11;
            uint64_t r12, r13, r14, r15;
            uint64_t rip, rflags;
        };

        // VM exit handler callback
        using VMExitHandler = std::function<bool(GuestRegisters&)>;

        // ===== Initialization =====

        // Check if CPU supports virtualization
        static bool IsSupportedCPU();

        // Check if VT-x/AMD-V is enabled in BIOS
        static bool IsVirtualizationEnabled();

        // Load thin hypervisor
        static bool LoadHypervisor();

        // Unload hypervisor
        static void UnloadHypervisor();

        // Check if hypervisor is loaded
        static bool IsLoaded();

        // Get hypervisor status
        static std::string GetStatus();

        // ===== VM Exit Hooks =====

        // Hook a VM exit reason
        static bool HookVMExit(VMExitReason reason, VMExitHandler handler);

        // Unhook VM exit
        static bool UnhookVMExit(VMExitReason reason);

        // Clear all VM exit hooks
        static void ClearVMExitHooks();

        // ===== EPT (Extended Page Tables) =====

        // Hide memory page from OS/kernel scans
        static bool HideMemoryPage(uintptr_t virtualAddress);

        // Unhide memory page
        static bool UnhideMemoryPage(uintptr_t virtualAddress);

        // Shadow memory page (OS sees fake data, you see real)
        static bool ShadowMemoryPage(uintptr_t virtualAddress, void* shadowData, size_t size);

        // Remove shadow
        static bool RemoveShadow(uintptr_t virtualAddress);

        // Check if page is hidden
        static bool IsPageHidden(uintptr_t virtualAddress);

        // ===== Anti-VM Detection =====

        // Spoof CPUID to hide hypervisor presence
        static bool EnableCPUIDSpoofing(bool enable = true);

        // Spoof RDTSC/RDTSCP (timing attacks)
        static bool EnableRDTSCSpoofing(bool enable = true);

        // Set fake CPU vendor string
        static bool SetFakeCPUVendor(const std::string& vendor);

        // Hide VM artifacts
        static bool HideVMSignatures();

        // ===== Memory Operations =====

        // Read physical memory (bypass OS)
        static bool ReadPhysicalMemory(uint64_t physicalAddress, void* buffer, size_t size);

        // Write physical memory
        static bool WritePhysicalMemory(uint64_t physicalAddress, const void* buffer, size_t size);

        // Translate guest virtual to physical
        static uint64_t GuestVirtualToPhysical(uintptr_t guestVirtual);

        // ===== VMCALL Interface =====

        // Execute VMCALL to hypervisor
        static uint64_t VMCall(uint64_t command, uint64_t arg1 = 0, uint64_t arg2 = 0, uint64_t arg3 = 0);

        // Custom VMCALL commands
        enum VMCallCommand {
            VMCALL_TEST = 0,
            VMCALL_HIDE_PAGE = 1,
            VMCALL_UNHIDE_PAGE = 2,
            VMCALL_SHADOW_PAGE = 3,
            VMCALL_READ_PHYS = 4,
            VMCALL_WRITE_PHYS = 5,
            VMCALL_GET_INFO = 6
        };

        // ===== Utility =====

        // Get CPU vendor (Intel/AMD)
        static std::string GetCPUVendor();

        // Check if running under a hypervisor
        static bool IsRunningUnderHypervisor();

        // Get virtualization technology (VT-x/AMD-V)
        static std::string GetVirtualizationTech();

    private:
        static inline bool s_loaded = false;
        static inline bool s_cpuidSpoofing = false;
        static inline bool s_rdtscSpoofing = false;
        static inline std::string s_fakeCPUVendor = "";
        
        static inline std::map<VMExitReason, VMExitHandler> s_vmExitHandlers;
        static inline std::map<uintptr_t, bool> s_hiddenPages;
        static inline std::map<uintptr_t, std::vector<BYTE>> s_shadowPages;

        // Internal initialization
        static bool InitializeVTx();      // Intel
        static bool InitializeSVM();      // AMD
        static bool SetupVMCS();          // Virtual Machine Control Structure
        static bool SetupEPT();           // Extended Page Tables

        // CPUID handling
        static void HandleCPUID(GuestRegisters& regs);
        static void HandleRDTSC(GuestRegisters& regs);

        // EPT helpers
        static bool ModifyEPTEntry(uintptr_t virtualAddress, bool hide);
    };

} // namespace MemLib

#endif // HYPERVISOR_H
