#pragma once

#ifndef MEMORY_LIB_H
#define MEMORY_LIB_H

/**
 * WarlockLib - Complete Memory Manipulation Library
 * Professional-grade game hacking toolkit
 * 
 * Access Levels:
 * - User-mode (Ring 3)
 * - Kernel Driver (Ring 0)
 * - Direct Syscalls (Ring 0)
 * - Hypervisor (Ring -1)
 * - DMA (Hardware)
 * 
 * Features:
 * - Memory operations (read/write/freeze/hook)
 * - Pattern scanning and code caves
 * - Input/output automation
 * - Anti-cheat evasion
 * - DirectX overlays and UI
 */

namespace WarlockLib {

    // Forward declarations
    class Process;
    class Memory;
    class Scanner;
    class Driver;
    class IO;
    class Evasion;
    class DMA;
    class Hypervisor;
    class Overlay;
    class ImGuiMenu;
    class XAMLMenu;
    class WarlockLib;

    // Convenience macros
    #define WARLOCK_READ(type, address) Memory::ReadInternal<type>(address)
    #define WARLOCK_WRITE(type, address, value) Memory::WriteInternal<type>(address, value)

} // namespace WarlockLib

// Include all library components (organized by folder)
#include "Core/Process.h"
#include "Core/Memory.h"
#include "Core/Scanner.h"
#include "Access/Driver.h"
#include "Utils/IO.h"
#include "Evasion/Evasion.h"
#include "Access/DMA.h"
#include "Access/Hypervisor.h"
#include "UI/Overlay.h"
#include "UI/ImGuiMenu.h"
#include "UI/XAMLMenu.h"
#include "SimpleCheat.h"

#endif // MEMORY_LIB_H
