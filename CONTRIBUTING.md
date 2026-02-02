# Contributing to WarlockLib

Thank you for your interest in contributing to WarlockLib!

## Code of Conduct

- Be respectful and professional
- Focus on educational and research purposes
- Do not promote malicious use
- Follow legal and ethical guidelines

## How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported
2. Use the bug report template
3. Include steps to reproduce
4. Provide system information
5. Include code samples if applicable

### Suggesting Features

1. Check existing feature requests
2. Clearly describe the feature
3. Explain the use case
4. Consider educational value

### Pull Requests

1. **Fork the repository**
2. **Create a feature branch**
   ```bash
   git checkout -b feature/AmazingFeature
   ```
3. **Follow coding standards**
   - Use C++17 features
   - Follow existing code style
   - Add comments for complex logic
   - Update API_REFERENCE.md if adding functions

4. **Test your changes**
   - Ensure code compiles
   - Test on Windows x64
   - No memory leaks

5. **Commit your changes**
   ```bash
   git commit -m "Add: AmazingFeature description"
   ```

6. **Push to your fork**
   ```bash
   git push origin feature/AmazingFeature
   ```

7. **Open a Pull Request**

## Coding Standards

### C++ Style

```cpp
// Use meaningful names
void ProcessMemoryOperation() { }  // Good
void pmo() { }                      // Bad

// Comment complex logic
// Parse IDA-style pattern: "48 8B ?? ??"
bool ParsePattern(const std::string& pattern) {
    // Implementation
}

// Use const references for large objects
void ProcessData(const std::vector<byte>& data) { }

// Prefer std::optional over raw pointers for return values
std::optional<uintptr_t> FindAddress() {
    return address;
}
```

### Namespace

All code must be in the `WarlockLib` namespace:

```cpp
namespace WarlockLib {
    class MyClass {
        // Implementation
    };
}
```

### File Organization

```
WarlockLib/
??? Core/         # Core functionality
??? Access/       # Access methods (Driver, DMA, etc.)
??? Evasion/      # Anti-detection
??? Utils/        # Utilities
??? UI/           # User interfaces
```

## Documentation

- Update `API_REFERENCE.md` for new functions
- Add inline comments for complex code
- Include usage examples

## Testing

- Test on clean Windows 10/11 installation
- Verify no anti-virus false positives
- Check for memory leaks with tools like Valgrind/DrMemory

## Legal Considerations

- All contributions must be for educational purposes
- Do not include malware or malicious code
- Respect intellectual property
- Follow applicable laws

## Questions?

Open an issue with the "question" label.

Thank you for contributing! ?????
