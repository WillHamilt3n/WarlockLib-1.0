[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com/windows)
[![Educational](https://img.shields.io/badge/Purpose-Educational-green.svg)]()

Memory manipulation library for C++ game development and reverse engineering research.

Features

- Multiple Access Levels**: User-mode, Kernel, Syscalls, Hypervisor, DMA
- Memory Operations**: Read/Write/Freeze with pointer chain support
- Pattern Scanning**: IDA-style AOB scanning with wildcards
- Function Hooking**: Inline hooks, VTable hooks, trampolines
- Anti-Cheat Evasion**: PEB hiding, syscalls, string obfuscation
- DirectX Overlays**: ESP, health bars, crosshairs
- UI Systems**: ImGui and XAML menu integration
- Code Injection**: Code cave finder, NOP patching

Quick Start

```cpp
#include "MemoryLib.h"
using namespace WarlockLib;

int main() {
    WarlockLib::Attach(L"game.exe");
    WarlockLib::WriteModule<float>(L"game.exe", 0x1234, 9999.0f);
    WarlockLib::FreezeModule<float>(L"game.exe", 0x1234, 9999.0f);
    WarlockLib::Detach();
    return 0;
}
```

Installation

 Prerequisites
- Windows Vista or higher (x64)
- Visual Studio 2017+ or compatible C++17 compiler
- Windows SDK

Build from Source

1. **Clone the repository**
```bash
git clone https://github.com/yourusername/WarlockLib.git
cd WarlockLib
```

2. **Organize project structure**
```bash
CleanOrganize.bat
```

3. **Open in Visual Studio**
- Open `WarlockLib.sln`
- Add `WarlockLib.props` to project (Property Manager)
- Build → Rebuild Solution

4. **Or build with CMake**
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Documentation

- [API Reference](API_REFERENCE.md)** - Complete function reference
- [Examples](#-examples)** - Usage examples below

Examples

Basic Memory Operations
```cpp
WarlockLib::Attach(L"game.exe");
float health = WarlockLib::ReadModule<float>(L"game.exe", 0x1234);
WarlockLib::WriteModule<float>(L"game.exe", 0x1234, 9999.0f);
```

Pattern Scanning
```cpp
uintptr_t addr = WarlockLib::FindPattern(L"game.exe", "48 8B 0D ?? ?? ?? ??");
WarlockLib::Nop(addr, 5);
```

Memory Freezing
```cpp
WarlockLib::FreezeModule<float>(L"game.exe", 0x1234, 9999.0f);
WarlockLib::UnfreezeAll();
```

Kernel Driver Mode
```cpp
WarlockLib::AttachWithDriver(L"game.exe", L"MyDriver");
WarlockLib::WriteModule<float>(L"game.exe", 0x1234, 9999.0f);
```

DMA Hardware Access
```cpp
WarlockLib::AttachWithDMA(L"game.exe");
WarlockLib::WriteModule<float>(L"game.exe", 0x1234, 9999.0f);
```

Architecture

```
WarlockLib/
├── Include/      # Public API headers
├── Core/         # Memory, Process, Scanner
├── Access/       # Driver, DMA, Hypervisor
├── Evasion/      # Anti-cheat techniques
├── Utils/        # IO operations
└── UI/           # Overlay and menu systems
```

Access Levels

| Level | Ring | Stealth | Speed | Use Case |
|-------|------|---------|-------|----------|
| User-mode | Ring 3 | Low | Fast | Development, Testing |
| Kernel Driver | Ring 0 | Medium | Fast | Bypass basic anti-cheat |
| Syscalls | Ring 0 | High | Fast | Bypass API hooks |
| Hypervisor | Ring -1 | Maximum | Medium | EPT hiding, VM hooks |
| DMA | Hardware | Maximum | Fastest | Complete undetectability |

Anti-Detection Features

- **PEB Hiding**: Hide DLL from module enumeration
- **Thread Hiding**: Hide threads from debuggers
- **String Obfuscation**: Compile-time encryption
- **Direct Syscalls**: Bypass all API hooks
- **EPT Memory Hiding**: Hide pages from kernel scans
- **CPUID/RDTSC Spoofing**: Defeat VM detection

UI Features

DirectX Overlay
```cpp
Overlay::Initialize();
Overlay::DrawESPBox(100, 100, 50, 100, Overlay::Color::Red());
Overlay::DrawHealthBar(100, 80, 50, 5, 75.0f);
```

ImGui Menu
```cpp
ImGuiMenu::Initialize();
if (ImGuiMenu::BeginMainMenu("Cheat")) {
    ImGuiMenu::Checkbox("Aimbot", &enabled);
    ImGuiMenu::SliderFloat("FOV", &fov, 1.0f, 20.0f);
    ImGuiMenu::EndMainMenu();
}
```

Requirements

- **OS**: Windows Vista+ (x64)
- **Compiler**: C++17 or higher
- **IDE**: Visual Studio 2017+ (recommended)
- **Optional**: Kernel driver for Ring 0/Ring -1 access
- **Optional**: DMA hardware (PCILeech, ScreenPeek, etc.)

Legal Disclaimer

FOR EDUCATIONAL AND RESEARCH PURPOSES ONLY**

This library is intended for:
- Learning about Windows internals and memory management
- Reverse engineering research
- Security research and vulnerability testing
- Educational purposes


-  Only use on software you own or have permission to analyze
-  Comply with all applicable laws and regulations
-  Accept full responsibility for your use

**You must NOT:**
-  Use for malicious purposes
-  Use on online games or services (violates TOS/EULA)
-  Distribute malware or cheats
-  Violate any laws or regulations

**The authors and contributors are not responsible for any misuse of this software.**

Contributing

Contributions are welcome! Please feel free to submit pull requests.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

**Educational Use Clause**: This software is provided for educational purposes only. Misuse may violate laws and regulations.

Acknowledgments

- Shoutout to Aezone - especially Opel,Teeko,Alura,and anyone else i bothered for their help and support
- Shoutout to GuidedHacking for wasting my money on shit i learnt for free
- Shoutout to UnknownCheats for random ppl posting bangers and teaching me shit for fun
- Shoutout to destiny 2 for being so ass i had to learn how to cheat
