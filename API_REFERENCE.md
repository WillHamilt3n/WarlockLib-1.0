# WarlockLib Function Reference

## WarlockLib (High-Level API)

```cpp
WarlockLib::Attach(const std::wstring& processName) -> bool
WarlockLib::Attach(L"game.exe");

WarlockLib::AttachWithDriver(const std::wstring& processName, const std::wstring& driverName) -> bool
WarlockLib::AttachWithDriver(L"game.exe", L"MyDriver");

WarlockLib::AttachWithDMA(const std::wstring& processName) -> bool
WarlockLib::AttachWithDMA(L"game.exe");

WarlockLib::Detach() -> void
WarlockLib::Detach();

WarlockLib::IsAttached() -> bool
if (WarlockLib::IsAttached()) { }

WarlockLib::GetModuleBase(const std::wstring& moduleName) -> uintptr_t
uintptr_t base = WarlockLib::GetModuleBase(L"game.exe");

WarlockLib::Read<T>(uintptr_t address) -> T
float health = WarlockLib::Read<float>(0x12345678);

WarlockLib::Write<T>(uintptr_t address, const T& value) -> bool
WarlockLib::Write<float>(0x12345678, 9999.0f);

WarlockLib::ReadModule<T>(const std::wstring& moduleName, uintptr_t offset) -> T
float health = WarlockLib::ReadModule<float>(L"game.exe", 0x1234);

WarlockLib::WriteModule<T>(const std::wstring& moduleName, uintptr_t offset, const T& value) -> bool
WarlockLib::WriteModule<float>(L"game.exe", 0x1234, 9999.0f);

WarlockLib::ReadPointer<T>(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets) -> T
int ammo = WarlockLib::ReadPointer<int>(base, {0x10, 0x20, 0x30});

WarlockLib::ReadModulePointer<T>(const std::wstring& moduleName, const std::vector<uintptr_t>& offsets) -> T
float ammo = WarlockLib::ReadModulePointer<float>(L"game.exe", {0x1000, 0x20});

WarlockLib::FindPattern(const std::wstring& moduleName, const std::string& pattern) -> uintptr_t
uintptr_t addr = WarlockLib::FindPattern(L"game.exe", "48 8B 0D ?? ?? ?? ??");

WarlockLib::Nop(uintptr_t address, size_t count) -> bool
WarlockLib::Nop(0x12345678, 5);

WarlockLib::FreezeModule<T>(const std::wstring& moduleName, uintptr_t offset, const T& value) -> bool
WarlockLib::FreezeModule<float>(L"game.exe", 0x1234, 9999.0f);

WarlockLib::Unfreeze(uintptr_t address) -> bool
WarlockLib::Unfreeze(healthAddress);

WarlockLib::UnfreezeAll() -> void
WarlockLib::UnfreezeAll();

WarlockLib::GetFrozenCount() -> size_t
size_t count = WarlockLib::GetFrozenCount();

WarlockLib::FindCodeCave(const std::wstring& moduleName, size_t size) -> uintptr_t
uintptr_t cave = WarlockLib::FindCodeCave(L"game.exe", 100);

WarlockLib::VerifyCodeCave(uintptr_t address, size_t size) -> bool
if (WarlockLib::VerifyCodeCave(cave, 100)) { }

WarlockLib::HookFunction(uintptr_t targetAddress, uintptr_t hookAddress) -> std::unique_ptr<Memory::InlineHook>
auto hook = WarlockLib::HookFunction(0x12345678, hookAddr);
```

## Memory

```cpp
Memory::Read<T>(HANDLE processHandle, uintptr_t address) -> std::optional<T>
auto value = Memory::Read<float>(handle, 0x12345678);

Memory::Write<T>(HANDLE processHandle, uintptr_t address, const T& value) -> bool
Memory::Write<float>(handle, 0x12345678, 9999.0f);

Memory::ReadInternal<T>(uintptr_t address) -> T
float health = Memory::ReadInternal<float>(0x12345678);

Memory::WriteInternal<T>(uintptr_t address, const T& value) -> void
Memory::WriteInternal<float>(0x12345678, 9999.0f);

Memory::ReadBytes(HANDLE processHandle, uintptr_t address, size_t size) -> std::vector<byte>
auto bytes = Memory::ReadBytes(handle, 0x12345678, 100);

Memory::WriteBytes(HANDLE processHandle, uintptr_t address, const std::vector<byte>& bytes) -> bool
Memory::WriteBytes(handle, 0x12345678, bytes);

Memory::WriteProtected<T>(HANDLE processHandle, uintptr_t address, const T& value) -> bool
Memory::WriteProtected<int>(handle, 0x12345678, 1337);

Memory::WriteProtectedInternal<T>(uintptr_t address, const T& value) -> bool
Memory::WriteProtectedInternal<int>(0x12345678, 1337);

Memory::FollowPointerChain(HANDLE processHandle, uintptr_t baseAddress, const std::vector<uintptr_t>& offsets) -> std::optional<uintptr_t>
auto addr = Memory::FollowPointerChain(handle, base, {0x10, 0x20});

Memory::FollowPointerChainInternal(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets) -> uintptr_t
uintptr_t addr = Memory::FollowPointerChainInternal(base, {0x10, 0x20});

Memory::QueryMemory(HANDLE processHandle, uintptr_t address) -> std::optional<MEMORY_BASIC_INFORMATION>
auto mbi = Memory::QueryMemory(handle, 0x12345678);

Memory::GetMemoryRegions(HANDLE processHandle) -> std::vector<MEMORY_BASIC_INFORMATION>
auto regions = Memory::GetMemoryRegions(handle);

Memory::AllocateMemory(HANDLE processHandle, size_t size, DWORD protection) -> uintptr_t
uintptr_t addr = Memory::AllocateMemory(handle, 1024, PAGE_EXECUTE_READWRITE);

Memory::FreeMemory(HANDLE processHandle, uintptr_t address) -> bool
Memory::FreeMemory(handle, allocatedAddr);

Memory::Freeze<T>(uintptr_t address, const T& value, DWORD intervalMs, HANDLE processHandle) -> bool
Memory::Freeze<float>(0x12345678, 9999.0f, 100, handle);

Memory::Unfreeze(uintptr_t address) -> bool
Memory::Unfreeze(0x12345678);

Memory::UnfreezeAll() -> void
Memory::UnfreezeAll();

Memory::IsFrozen(uintptr_t address) -> bool
if (Memory::IsFrozen(0x12345678)) { }

Memory::GetFrozenCount() -> size_t
size_t count = Memory::GetFrozenCount();

Memory::Nop(uintptr_t address, size_t count) -> bool
Memory::Nop(0x12345678, 5);

Memory::RestoreBytes(uintptr_t address, const std::vector<byte>& originalBytes) -> bool
Memory::RestoreBytes(0x12345678, original);

Memory::SaveBytes(uintptr_t address, size_t count) -> std::vector<byte>
auto original = Memory::SaveBytes(0x12345678, 5);
```

## Memory::InlineHook

```cpp
InlineHook(uintptr_t targetFunction, uintptr_t hookFunction)
auto hook = Memory::InlineHook(targetFunc, hookFunc);

Install() -> bool
hook.Install();

Uninstall() -> bool
hook.Uninstall();

IsInstalled() -> bool
if (hook.IsInstalled()) { }

GetOriginalFunction() -> uintptr_t
uintptr_t original = hook.GetOriginalFunction();

GetTrampoline() -> uintptr_t
uintptr_t trampoline = hook.GetTrampoline();
```

## Memory::VTableHook

```cpp
VTableHook(void* instance)
auto vtHook = Memory::VTableHook(objectInstance);

HookFunction(size_t index, uintptr_t hookFunction) -> bool
vtHook.HookFunction(5, hookAddr);

UnhookFunction(size_t index) -> bool
vtHook.UnhookFunction(5);

UnhookAll() -> void
vtHook.UnhookAll();

GetOriginalFunction(size_t index) -> uintptr_t
uintptr_t orig = vtHook.GetOriginalFunction(5);
```

## Process

```cpp
Attach(const std::wstring& processName) -> bool
proc.Attach(L"game.exe");

Attach(DWORD processId) -> bool
proc.Attach(1234);

Detach() -> void
proc.Detach();

IsAttached() -> bool
if (proc.IsAttached()) { }

GetProcessId() -> DWORD
DWORD pid = proc.GetProcessId();

GetProcessHandle() -> HANDLE
HANDLE handle = proc.GetProcessHandle();

GetModuleBase(const std::wstring& moduleName) -> std::optional<uintptr_t>
auto base = proc.GetModuleBase(L"game.exe");

GetModuleSize(const std::wstring& moduleName) -> std::optional<size_t>
auto size = proc.GetModuleSize(L"game.exe");

GetModules() -> std::vector<ModuleInfo>
auto modules = proc.GetModules(handle);

FindProcess(const std::wstring& processName) -> std::optional<DWORD>
auto pid = Process::FindProcess(L"game.exe");

GetProcessList() -> std::vector<ProcessInfo>
auto processes = Process::GetProcessList();
```

## Scanner

```cpp
Scanner::FindPattern(HANDLE processHandle, const std::wstring& moduleName, const std::string& pattern) -> std::optional<uintptr_t>
auto addr = Scanner::FindPattern(handle, L"game.exe", "48 8B 0D ?? ?? ?? ??");

Scanner::FindPatternInRegion(HANDLE processHandle, uintptr_t startAddress, size_t size, const std::string& pattern) -> std::optional<uintptr_t>
auto addr = Scanner::FindPatternInRegion(handle, base, 0x1000, "48 8B");

Scanner::FindAllPatterns(HANDLE processHandle, const std::wstring& moduleName, const std::string& pattern) -> std::vector<uintptr_t>
auto addrs = Scanner::FindAllPatterns(handle, L"game.exe", "48 8B");

Scanner::FindPatternInternal(const std::wstring& moduleName, const std::string& pattern) -> std::optional<uintptr_t>
auto addr = Scanner::FindPatternInternal(L"game.exe", "48 8B");

Scanner::FindCodeCave(HANDLE processHandle, const std::wstring& moduleName, size_t size) -> std::optional<uintptr_t>
auto cave = Scanner::FindCodeCave(handle, L"game.exe", 100);

Scanner::FindCodeCaveNear(HANDLE processHandle, uintptr_t address, size_t size, size_t maxDistance) -> std::optional<uintptr_t>
auto cave = Scanner::FindCodeCaveNear(handle, 0x12345678, 64, 0x1000);

Scanner::FindAllCodeCaves(HANDLE processHandle, const std::wstring& moduleName, size_t minSize, size_t maxCount) -> std::vector<uintptr_t>
auto caves = Scanner::FindAllCodeCaves(handle, L"game.exe", 64, 10);

Scanner::VerifyCodeCave(HANDLE processHandle, uintptr_t address, size_t size) -> bool
if (Scanner::VerifyCodeCave(handle, cave, 100)) { }

Scanner::ParsePattern(const std::string& pattern, std::vector<BYTE>& bytes, std::vector<bool>& mask) -> bool
Scanner::ParsePattern("48 8B ?? ??", bytes, mask);

Scanner::CreatePattern(const std::vector<BYTE>& bytes, const std::vector<bool>& mask) -> std::string
std::string pat = Scanner::CreatePattern(bytes, mask);

Scanner::FindExecutableRegions(HANDLE processHandle) -> std::vector<std::pair<uintptr_t, size_t>>
auto regions = Scanner::FindExecutableRegions(handle);

Scanner::IsExecutable(HANDLE processHandle, uintptr_t address) -> bool
if (Scanner::IsExecutable(handle, 0x12345678)) { }

Scanner::FindStrings(HANDLE processHandle, const std::wstring& moduleName, size_t minLength) -> std::vector<std::pair<uintptr_t, std::string>>
auto strings = Scanner::FindStrings(handle, L"game.exe", 4);
```

## Driver

```cpp
Connect(const std::wstring& deviceName) -> bool
driver.Connect(L"MyDriver");

Disconnect() -> void
driver.Disconnect();

IsConnected() -> bool
if (driver.IsConnected()) { }

SetTargetProcess(DWORD processId) -> bool
driver.SetTargetProcess(1234);

Read<T>(uintptr_t address) -> std::optional<T>
auto value = driver.Read<float>(0x12345678);

Write<T>(uintptr_t address, const T& value) -> bool
driver.Write<float>(0x12345678, 9999.0f);

ReadBytes(uintptr_t address, size_t size) -> std::vector<byte>
auto bytes = driver.ReadBytes(0x12345678, 100);

WriteBytes(uintptr_t address, const std::vector<byte>& bytes) -> bool
driver.WriteBytes(0x12345678, bytes);

AllocateMemory(size_t size, DWORD protection) -> uintptr_t
uintptr_t addr = driver.AllocateMemory(1024, PAGE_EXECUTE_READWRITE);

FreeMemory(uintptr_t address) -> bool
driver.FreeMemory(allocatedAddr);

ProtectMemory(uintptr_t address, size_t size, DWORD newProtection, DWORD* oldProtection) -> bool
driver.ProtectMemory(addr, 1024, PAGE_EXECUTE_READWRITE, &old);

GetModuleBase(const std::wstring& moduleName) -> std::optional<uintptr_t>
auto base = driver.GetModuleBase(L"game.exe");

SendIOCTL(DWORD ioControlCode, void* inputBuffer, DWORD inputSize, void* outputBuffer, DWORD outputSize) -> bool
driver.SendIOCTL(CUSTOM_IOCTL, &input, sizeof(input), &output, sizeof(output));

SetEncryption(EncryptionMode mode, BYTE key) -> void
driver.SetEncryption(Driver::ENCRYPT_XOR, 0x42);
```

## DMA

```cpp
DMA::Initialize(DeviceType type, const std::string& config) -> bool
DMA::Initialize(DMA::DEVICE_PCILEECH, "");

DMA::Shutdown() -> void
DMA::Shutdown();

DMA::IsAvailable() -> bool
if (DMA::IsAvailable()) { }

DMA::GetDeviceInfo() -> std::string
std::string info = DMA::GetDeviceInfo();

DMA::ReadPhysical(uint64_t physicalAddress, void* buffer, size_t size) -> bool
DMA::ReadPhysical(0x1000000, buffer, 4);

DMA::WritePhysical(uint64_t physicalAddress, const void* buffer, size_t size) -> bool
DMA::WritePhysical(0x1000000, &value, 4);

DMA::GetProcessDTB(DWORD processId) -> std::optional<uint64_t>
auto dtb = DMA::GetProcessDTB(1234);

DMA::VirtualToPhysical(DWORD processId, uintptr_t virtualAddress) -> std::optional<uint64_t>
auto phys = DMA::VirtualToPhysical(1234, 0x12345678);

DMA::ReadVirtual(DWORD processId, uintptr_t virtualAddress, void* buffer, size_t size) -> bool
DMA::ReadVirtual(1234, 0x12345678, &value, 4);

DMA::WriteVirtual(DWORD processId, uintptr_t virtualAddress, const void* buffer, size_t size) -> bool
DMA::WriteVirtual(1234, 0x12345678, &value, 4);

DMA::Read<T>(DWORD processId, uintptr_t virtualAddress) -> std::optional<T>
auto health = DMA::Read<float>(1234, 0x12345678);

DMA::Write<T>(DWORD processId, uintptr_t virtualAddress, const T& value) -> bool
DMA::Write<float>(1234, 0x12345678, 9999.0f);

DMA::ReadBytes(DWORD processId, uintptr_t virtualAddress, size_t size) -> std::vector<BYTE>
auto bytes = DMA::ReadBytes(1234, 0x12345678, 100);

DMA::FindProcess(const std::wstring& processName) -> DWORD
DWORD pid = DMA::FindProcess(L"game.exe");

DMA::GetProcessList() -> std::vector<std::pair<DWORD, std::wstring>>
auto procs = DMA::GetProcessList();

DMA::GetModuleBase(DWORD processId, const std::wstring& moduleName) -> std::optional<uintptr_t>
auto base = DMA::GetModuleBase(1234, L"game.exe");

DMA::GetModuleSize(DWORD processId, const std::wstring& moduleName) -> std::optional<size_t>
auto size = DMA::GetModuleSize(1234, L"game.exe");

DMA::GetModuleList(DWORD processId) -> std::vector<std::pair<std::wstring, uintptr_t>>
auto modules = DMA::GetModuleList(1234);

DMA::FindPattern(DWORD processId, const std::wstring& moduleName, const std::string& pattern) -> std::optional<uintptr_t>
auto addr = DMA::FindPattern(1234, L"game.exe", "48 8B");

DMA::FindPatternInRange(DWORD processId, uintptr_t start, size_t size, const std::string& pattern) -> std::optional<uintptr_t>
auto addr = DMA::FindPatternInRange(1234, base, 0x1000, "48 8B");

DMA::FindAllPatterns(DWORD processId, const std::wstring& moduleName, const std::string& pattern, size_t maxResults) -> std::vector<uintptr_t>
auto addrs = DMA::FindAllPatterns(1234, L"game.exe", "48 8B", 10);

DMA::ScatterRead(DWORD processId, std::vector<ScatterEntry>& entries) -> bool
DMA::ScatterRead(1234, entries);

DMA::ScatterWrite(DWORD processId, std::vector<ScatterEntry>& entries) -> bool
DMA::ScatterWrite(1234, entries);

DMA::SetDTBCaching(bool enabled) -> void
DMA::SetDTBCaching(true);

DMA::ClearDTBCache() -> void
DMA::ClearDTBCache();

DMA::GetStatistics() -> Statistics
auto stats = DMA::GetStatistics();

DMA::ResetStatistics() -> void
DMA::ResetStatistics();
```

## Hypervisor

```cpp
Hypervisor::IsSupportedCPU() -> bool
if (Hypervisor::IsSupportedCPU()) { }

Hypervisor::IsVirtualizationEnabled() -> bool
if (Hypervisor::IsVirtualizationEnabled()) { }

Hypervisor::GetCPUVendor() -> std::string
std::string vendor = Hypervisor::GetCPUVendor();

Hypervisor::GetVirtualizationTech() -> std::string
std::string tech = Hypervisor::GetVirtualizationTech();

Hypervisor::LoadHypervisor() -> bool
Hypervisor::LoadHypervisor();

Hypervisor::UnloadHypervisor() -> void
Hypervisor::UnloadHypervisor();

Hypervisor::IsLoaded() -> bool
if (Hypervisor::IsLoaded()) { }

Hypervisor::GetStatus() -> std::string
std::string status = Hypervisor::GetStatus();

Hypervisor::HideMemoryPage(uintptr_t address) -> bool
Hypervisor::HideMemoryPage(dllBase);

Hypervisor::UnhideMemoryPage(uintptr_t address) -> bool
Hypervisor::UnhideMemoryPage(dllBase);

Hypervisor::IsPageHidden(uintptr_t address) -> bool
if (Hypervisor::IsPageHidden(dllBase)) { }

Hypervisor::ShadowMemoryPage(uintptr_t address, const void* fakeData, size_t size) -> bool
Hypervisor::ShadowMemoryPage(healthAddr, &fakeHealth, 4);

Hypervisor::RemoveShadow(uintptr_t address) -> bool
Hypervisor::RemoveShadow(healthAddr);

Hypervisor::HideVMSignatures() -> bool
Hypervisor::HideVMSignatures();

Hypervisor::EnableCPUIDSpoofing(bool enable) -> void
Hypervisor::EnableCPUIDSpoofing(true);

Hypervisor::EnableRDTSCSpoofing(bool enable) -> void
Hypervisor::EnableRDTSCSpoofing(true);

Hypervisor::SetFakeCPUVendor(const std::string& vendor) -> void
Hypervisor::SetFakeCPUVendor("GenuineIntel");

Hypervisor::IsRunningUnderHypervisor() -> bool
if (Hypervisor::IsRunningUnderHypervisor()) { }

Hypervisor::HookVMExit(VMExitReason reason, VMExitHandler handler) -> bool
Hypervisor::HookVMExit(Hypervisor::EXIT_CPUID, handler);

Hypervisor::UnhookVMExit(VMExitReason reason) -> bool
Hypervisor::UnhookVMExit(Hypervisor::EXIT_CPUID);

Hypervisor::ReadPhysicalMemory(uint64_t physicalAddress, void* buffer, size_t size) -> bool
Hypervisor::ReadPhysicalMemory(0x1000000, buffer, 4);

Hypervisor::WritePhysicalMemory(uint64_t physicalAddress, const void* buffer, size_t size) -> bool
Hypervisor::WritePhysicalMemory(0x1000000, &value, 4);
```

## Evasion

```cpp
Evasion::HideModuleFromPEB(const std::wstring& moduleName) -> bool
Evasion::HideModuleFromPEB(L"MyCheat.dll");

Evasion::HideCurrentModule() -> bool
Evasion::HideCurrentModule();

Evasion::UnlinkModuleFromPEB(HMODULE hModule) -> bool
Evasion::UnlinkModuleFromPEB(GetModuleHandle(L"cheat.dll"));

Evasion::HideCurrentThread() -> bool
Evasion::HideCurrentThread();

Evasion::HideThread(HANDLE hThread) -> bool
Evasion::HideThread(GetCurrentThread());

Evasion::EncryptString(const std::string& str, BYTE key) -> std::string
std::string encrypted = Evasion::EncryptString("secret", 0x42);

Evasion::DecryptString(const std::string& encrypted, BYTE key) -> std::string
std::string plain = Evasion::DecryptString(encrypted, 0x42);

Evasion::XORCrypt(BYTE* data, size_t size, BYTE key) -> void
Evasion::XORCrypt(buffer, 100, 0x42);

Evasion::IsDebuggerPresent() -> bool
if (Evasion::IsDebuggerPresent()) { }

Evasion::IsBeingAnalyzed() -> bool
if (Evasion::IsBeingAnalyzed()) { }

Evasion::IsVirtualMachine() -> bool
if (Evasion::IsVirtualMachine()) { }

Evasion::IsCheatEngineRunning() -> bool
if (Evasion::IsCheatEngineRunning()) { }

Evasion::IsX64DbgRunning() -> bool
if (Evasion::IsX64DbgRunning()) { }

Evasion::IsIDARunning() -> bool
if (Evasion::IsIDARunning()) { }

Evasion::InitializeSyscalls() -> bool
Evasion::InitializeSyscalls();

Evasion::ShutdownSyscalls() -> void
Evasion::ShutdownSyscalls();

Evasion::AreSyscallsInitialized() -> bool
if (Evasion::AreSyscallsInitialized()) { }

Evasion::SyscallRead<T>(HANDLE processHandle, uintptr_t address) -> std::optional<T>
auto value = Evasion::SyscallRead<float>(handle, 0x12345678);

Evasion::SyscallWrite<T>(HANDLE processHandle, uintptr_t address, const T& value) -> bool
Evasion::SyscallWrite<float>(handle, 0x12345678, 9999.0f);

Evasion::SyscallAllocate(HANDLE processHandle, size_t size, DWORD protection) -> uintptr_t
uintptr_t addr = Evasion::SyscallAllocate(handle, 1024, PAGE_EXECUTE_READWRITE);

Evasion::SyscallFree(HANDLE processHandle, uintptr_t address) -> bool
Evasion::SyscallFree(handle, addr);

Evasion::SetProcessCritical(bool critical) -> bool
Evasion::SetProcessCritical(true);

Evasion::ElevatePrivileges() -> bool
Evasion::ElevatePrivileges();
```

## IO

```cpp
IO::SendKey(int vkey) -> void
IO::SendKey(VK_SPACE);

IO::SendKeyDown(int vkey) -> void
IO::SendKeyDown(VK_SHIFT);

IO::SendKeyUp(int vkey) -> void
IO::SendKeyUp(VK_SHIFT);

IO::SendText(const std::string& text) -> void
IO::SendText("Hello World!");

IO::IsKeyPressed(int vkey) -> bool
if (IO::IsKeyPressed(VK_F1)) { }

IO::MoveMouse(int x, int y) -> void
IO::MoveMouse(500, 300);

IO::ClickMouse(int x, int y) -> void
IO::ClickMouse(500, 300);

IO::ClickRight(int x, int y) -> void
IO::ClickRight(500, 300);

IO::MouseDown(int x, int y, bool rightButton) -> void
IO::MouseDown(500, 300, false);

IO::MouseUp(int x, int y, bool rightButton) -> void
IO::MouseUp(500, 300, false);

IO::GetMousePosition(int& x, int& y) -> void
IO::GetMousePosition(x, y);

IO::GetPixelColor(int x, int y) -> COLORREF
COLORREF color = IO::GetPixelColor(100, 100);

IO::FindColor(COLORREF color, int& outX, int& outY, int tolerance) -> bool
if (IO::FindColor(RGB(255, 0, 0), x, y, 10)) { }

IO::FindColorInRegion(COLORREF color, int x, int y, int width, int height, int& outX, int& outY, int tolerance) -> bool
if (IO::FindColorInRegion(RGB(255, 0, 0), 0, 0, 1920, 1080, x, y, 10)) { }

IO::CaptureScreen(const std::string& filename) -> bool
IO::CaptureScreen("screenshot.bmp");

IO::CaptureRegion(int x, int y, int width, int height, const std::string& filename) -> bool
IO::CaptureRegion(0, 0, 1920, 1080, "capture.bmp");

IO::GetScreenWidth() -> int
int width = IO::GetScreenWidth();

IO::GetScreenHeight() -> int
int height = IO::GetScreenHeight();

IO::GetScreenPixels(int x, int y, int width, int height) -> std::vector<COLORREF>
auto pixels = IO::GetScreenPixels(0, 0, 100, 100);

IO::ColorMatch(COLORREF color1, COLORREF color2, int tolerance) -> bool
if (IO::ColorMatch(c1, c2, 10)) { }

IO::RGB(unsigned char r, unsigned char g, unsigned char b) -> COLORREF
COLORREF red = IO::RGB(255, 0, 0);
```

## Overlay

```cpp
Overlay::Initialize(DXVersion version) -> bool
Overlay::Initialize(Overlay::DX_AUTO);

Overlay::Shutdown() -> void
Overlay::Shutdown();

Overlay::IsInitialized() -> bool
if (Overlay::IsInitialized()) { }

Overlay::GetVersion() -> DXVersion
auto ver = Overlay::GetVersion();

Overlay::DrawLine(float x1, float y1, float x2, float y2, const Color& color, float thickness) -> void
Overlay::DrawLine(100, 100, 200, 200, Overlay::Color::Red(), 2.0f);

Overlay::DrawBox(float x, float y, float width, float height, const Color& color, float thickness) -> void
Overlay::DrawBox(100, 100, 50, 100, Overlay::Color::Red(), 2.0f);

Overlay::DrawFilledBox(float x, float y, float width, float height, const Color& color) -> void
Overlay::DrawFilledBox(100, 100, 50, 100, Overlay::Color::Red());

Overlay::DrawCircle(float x, float y, float radius, const Color& color, int segments, float thickness) -> void
Overlay::DrawCircle(500, 300, 50, Overlay::Color::Green(), 32, 2.0f);

Overlay::DrawFilledCircle(float x, float y, float radius, const Color& color, int segments) -> void
Overlay::DrawFilledCircle(500, 300, 30, Overlay::Color::Blue(), 32);

Overlay::DrawText(float x, float y, const std::string& text, const Color& color, int fontSize) -> void
Overlay::DrawText(10, 10, "FPS: 60", Overlay::Color::White(), 16);

Overlay::Draw3DBox(const float worldPos[3], float width, float height, float depth, const Color& color) -> void
Overlay::Draw3DBox(worldPos, 2.0f, 4.0f, 2.0f, Overlay::Color::Red());

Overlay::DrawESPBox(float screenX, float screenY, float width, float height, const Color& color, bool filled) -> void
Overlay::DrawESPBox(100, 100, 50, 100, Overlay::Color::Red(), false);

Overlay::DrawHealthBar(float x, float y, float width, float height, float healthPercent, const Color& bgColor, const Color& fgColor) -> void
Overlay::DrawHealthBar(100, 80, 50, 5, 75.0f, Overlay::Color::Black(), Overlay::Color::Green());

Overlay::DrawCrosshair(float x, float y, float size, const Color& color, float thickness) -> void
Overlay::DrawCrosshair(960, 540, 10, Overlay::Color::White(), 2.0f);

Overlay::DrawDistance(float x, float y, float distance, const Color& color) -> void
Overlay::DrawDistance(125, 200, 25.5f, Overlay::Color::Yellow());

Overlay::WorldToScreen(const float worldPos[3], float& screenX, float& screenY) -> bool
if (Overlay::WorldToScreen(worldPos, screenX, screenY)) { }

Overlay::SetViewMatrix(const float matrix[16]) -> void
Overlay::SetViewMatrix(viewMatrix);

Overlay::GetScreenSize(int& width, int& height) -> void
Overlay::GetScreenSize(width, height);

Overlay::SetEnabled(bool enabled) -> void
Overlay::SetEnabled(true);

Overlay::IsEnabled() -> bool
if (Overlay::IsEnabled()) { }

Overlay::DrawFPS(float x, float y, const Color& color) -> void
Overlay::DrawFPS(10, 10, Overlay::Color::Yellow());

Overlay::GetFPS() -> float
float fps = Overlay::GetFPS();

Overlay::RegisterDrawCallback(DrawCallback callback) -> void
Overlay::RegisterDrawCallback([]() { /* draw code */ });

Overlay::ClearDrawCallbacks() -> void
Overlay::ClearDrawCallbacks();
```

## ImGuiMenu

```cpp
ImGuiMenu::Initialize() -> bool
ImGuiMenu::Initialize();

ImGuiMenu::Shutdown() -> void
ImGuiMenu::Shutdown();

ImGuiMenu::IsInitialized() -> bool
if (ImGuiMenu::IsInitialized()) { }

ImGuiMenu::SetVisible(bool visible) -> void
ImGuiMenu::SetVisible(true);

ImGuiMenu::IsVisible() -> bool
if (ImGuiMenu::IsVisible()) { }

ImGuiMenu::Toggle() -> void
ImGuiMenu::Toggle();

ImGuiMenu::SetToggleKey(int vkey) -> void
ImGuiMenu::SetToggleKey(VK_INSERT);

ImGuiMenu::SetStyle(Style style) -> void
ImGuiMenu::SetStyle(ImGuiMenu::STYLE_DARK);

ImGuiMenu::BeginMainMenu(const char* title) -> bool
if (ImGuiMenu::BeginMainMenu("Cheat Menu")) { }

ImGuiMenu::EndMainMenu() -> void
ImGuiMenu::EndMainMenu();

ImGuiMenu::BeginTabBar(const char* id) -> bool
if (ImGuiMenu::BeginTabBar("MainTabs")) { }

ImGuiMenu::EndTabBar() -> void
ImGuiMenu::EndTabBar();

ImGuiMenu::BeginTab(const char* label) -> bool
if (ImGuiMenu::BeginTab("Aimbot")) { }

ImGuiMenu::EndTab() -> void
ImGuiMenu::EndTab();

ImGuiMenu::Checkbox(const char* label, bool* value) -> bool
ImGuiMenu::Checkbox("Enable", &enabled);

ImGuiMenu::Button(const char* label) -> bool
if (ImGuiMenu::Button("Click Me")) { }

ImGuiMenu::SliderInt(const char* label, int* value, int min, int max) -> bool
ImGuiMenu::SliderInt("Value", &val, 0, 100);

ImGuiMenu::SliderFloat(const char* label, float* value, float min, float max) -> bool
ImGuiMenu::SliderFloat("FOV", &fov, 1.0f, 20.0f);

ImGuiMenu::Combo(const char* label, int* current, const char* const items[], int itemsCount) -> bool
ImGuiMenu::Combo("Options", &current, items, 3);

ImGuiMenu::InputText(const char* label, char* buffer, size_t bufferSize) -> bool
ImGuiMenu::InputText("Name", buffer, 256);

ImGuiMenu::InputFloat(const char* label, float* value) -> bool
ImGuiMenu::InputFloat("Value", &value);

ImGuiMenu::InputInt(const char* label, int* value) -> bool
ImGuiMenu::InputInt("Count", &count);

ImGuiMenu::ColorPicker(const char* label, float color[4]) -> bool
ImGuiMenu::ColorPicker("Color", color);

ImGuiMenu::Text(const char* format, ...) -> void
ImGuiMenu::Text("Status: %s", "Active");

ImGuiMenu::TextColored(float r, float g, float b, float a, const char* format, ...) -> void
ImGuiMenu::TextColored(1, 0, 0, 1, "Error!");

ImGuiMenu::Separator() -> void
ImGuiMenu::Separator();

ImGuiMenu::Spacing() -> void
ImGuiMenu::Spacing();

ImGuiMenu::SameLine() -> void
ImGuiMenu::SameLine();

ImGuiMenu::Columns(int count) -> void
ImGuiMenu::Columns(2);

ImGuiMenu::NextColumn() -> void
ImGuiMenu::NextColumn();
```

## XAMLMenu

```cpp
XAMLMenu::Initialize(WindowMode mode) -> bool
XAMLMenu::Initialize(XAMLMenu::MODE_OVERLAY);

XAMLMenu::Shutdown() -> void
XAMLMenu::Shutdown();

XAMLMenu::IsInitialized() -> bool
if (XAMLMenu::IsInitialized()) { }

XAMLMenu::SetVisible(bool visible) -> void
XAMLMenu::SetVisible(true);

XAMLMenu::IsVisible() -> bool
if (XAMLMenu::IsVisible()) { }

XAMLMenu::Toggle() -> void
XAMLMenu::Toggle();

XAMLMenu::SetSize(int width, int height) -> void
XAMLMenu::SetSize(400, 600);

XAMLMenu::SetPosition(int x, int y) -> void
XAMLMenu::SetPosition(100, 100);

XAMLMenu::SetOpacity(float opacity) -> void
XAMLMenu::SetOpacity(0.95f);

XAMLMenu::AddTab(const std::string& name) -> void
XAMLMenu::AddTab("Aimbot");

XAMLMenu::SetCurrentTab(const std::string& name) -> void
XAMLMenu::SetCurrentTab("Aimbot");

XAMLMenu::AddCheckbox(const std::string& label, bool* value) -> void
XAMLMenu::AddCheckbox("Enable", &enabled);

XAMLMenu::AddSliderInt(const std::string& label, int* value, int min, int max) -> void
XAMLMenu::AddSliderInt("Value", &val, 0, 100);

XAMLMenu::AddSliderFloat(const std::string& label, float* value, float min, float max) -> void
XAMLMenu::AddSliderFloat("FOV", &fov, 1.0f, 20.0f);

XAMLMenu::AddButton(const std::string& label, std::function<void()> onClick) -> void
XAMLMenu::AddButton("Click", []() { });

XAMLMenu::AddTextInput(const std::string& label, std::string* value) -> void
XAMLMenu::AddTextInput("Name", &name);

XAMLMenu::AddComboBox(const std::string& label, int* selected, const std::vector<std::string>& items) -> void
XAMLMenu::AddComboBox("Options", &sel, items);

XAMLMenu::AddToggle(const std::string& label, bool* value) -> void
XAMLMenu::AddToggle("Enable", &enabled);

XAMLMenu::AddColorPicker(const std::string& label, float color[4]) -> void
XAMLMenu::AddColorPicker("Color", color);

XAMLMenu::AddSeparator() -> void
XAMLMenu::AddSeparator();

XAMLMenu::AddLabel(const std::string& text) -> void
XAMLMenu::AddLabel("=== Settings ===");

XAMLMenu::SetTheme(const std::string& theme) -> void
XAMLMenu::SetTheme("Dark");

XAMLMenu::SetAccentColor(unsigned char r, unsigned char g, unsigned char b) -> void
XAMLMenu::SetAccentColor(255, 0, 0);

XAMLMenu::SetAcrylicBlur(bool enabled) -> void
XAMLMenu::SetAcrylicBlur(true);
```
