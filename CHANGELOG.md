# Changelog

All notable changes to WarlockLib will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2024-01-XX

### Added
- **Core Features**
  - Memory read/write operations (user-mode)
  - Pointer chain following
  - Memory freezing (continuous writing)
  - Pattern scanning with wildcard support
  - Code cave finder
  - Function hooking (inline, VTable, trampoline)

- **Access Methods**
  - User-mode (Ring 3)
  - Kernel driver support (Ring 0)
  - Direct syscalls (Ring 0)
  - Hypervisor mode (Ring -1)
  - DMA hardware support

- **Anti-Detection**
  - PEB module hiding
  - Thread hiding from debuggers
  - String obfuscation
  - VM detection and spoofing
  - CPUID/RDTSC spoofing
  - EPT memory hiding

- **UI Systems**
  - DirectX overlay (ESP, health bars, crosshairs)
  - ImGui menu integration
  - XAML UI system

- **Utilities**
  - Keyboard/mouse input simulation
  - Screen capture and pixel reading
  - Color detection
  - Process enumeration

### Documentation
- Complete API reference
- Code examples
- Build instructions
- Contributing guidelines

### Project Structure
- Organized folder structure
- CMake build system
- Visual Studio project files
- GitHub Actions CI/CD

## [Unreleased]

### Planned Features
- Assembly code injection helpers
- Network packet manipulation
- Module parser (PE parsing)
- Speedhack functionality
- Additional overlay styles

---

**Note**: Version numbers follow [Semantic Versioning](https://semver.org/).
- **MAJOR**: Incompatible API changes
- **MINOR**: Backwards-compatible functionality
- **PATCH**: Backwards-compatible bug fixes
