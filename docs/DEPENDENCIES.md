# Astraeus Dependency Footprint & Platform Abstraction Policy

## Overview

This document defines the dependency management and platform abstraction rules for Astraeus. The goal is to maintain a **minimal, stable, and portable** codebase with controlled dependencies and zero scattered platform-specific code.

## Core Principles

1. **Minimal Dependencies**: Prefer standard libraries (C++17, JDK 21+) over third-party libraries
2. **Stable ABI**: Platform-specific code must not leak into public headers
3. **Controlled Growth**: Every new dependency must be justified and budgeted
4. **Isolation**: Platform-specific code confined to `engine/platform/` module
5. **Module Separation**: Tooling dependencies isolated from runtime dependencies

---

## Dependency Tiers

### Tier 0: Always Allowed (Core Dependencies)

These dependencies are fundamental and require no approval:

**C++:**
- C++17 standard library (`<cstdint>`, `<cstddef>`, `<vector>`, `<string>`, etc.)
- Standard math library (`<cmath>`)
- Platform API abstraction via `engine/platform/Platform.hpp`

**Java:**
- JDK 21+ standard library
- JavaFX (UI module only - `astraeus-ui`)
- JDK FFM (Foreign Function & Memory API)

### Tier 1: Allowed With Justification

These dependencies may be used with proper justification and module placement:

**C++:**
- OpenGL loader (GLAD) - **Required for rendering**
  - Justification: Standard way to load OpenGL functions
  - Location: `engine/third_party/glad/`
  - Impact: ~200KB source, header-only after generation
  
- Minimal math library (GLM) - **Conditional use**
  - Justification: Only if standard library math is insufficient
  - Must be header-only
  - Impact: Header-only, <50KB

**Java:**
- YAML parser (SnakeYAML or similar) - **Tools module only**
  - Justification: Configuration and schema parsing
  - Location: `astraeus-tools` module only
  - Impact: ~300KB, not in runtime classpath
  
- JSON library (Gson/Jackson) - **Tools module only**
  - Justification: Data interchange for debugging/tools
  - Location: `astraeus-tools` module only
  - Impact: ~500KB, not in runtime classpath

### Tier 2: Avoid Unless Critical

These dependencies should be avoided and require strong justification:

**Discouraged:**
- Mega header-only utility libraries (Boost, Abseil) - Use only specific components if needed
- Logging frameworks (spdlog, log4j) - Use simple internal logging
- Reflection/serialization frameworks - Keep ABI simple and explicit
- Heavy UI frameworks beyond JavaFX
- Any library with transitive dependencies exceeding 5 additional libraries

**Never Allowed:**
- Libraries requiring exceptions across ABI boundary
- Libraries with callbacks from C++ to Java
- Libraries with unstable ABI (frequent breaking changes)
- GPL-licensed dependencies (use MIT/BSD/Apache 2.0)

---

## Dependency Budgets

### Per-Module Limits

| Module | Max Third-Party Deps | Max Transitive Deps | Notes |
|--------|---------------------|---------------------|-------|
| `engine/core` | 0 | 0 | Standard library only |
| `engine/platform` | 1 | 0 | OS APIs only (Win32/X11) |
| `engine/renderer` | 2 | 0 | GLAD + optional math lib |
| `engine/scene` | 0 | 0 | Standard library only |
| `engine/ingest` | 0 | 0 | Standard library only |
| `astraeus-native` (Java) | 0 | 0 | JDK only, no JavaFX |
| `astraeus-ui` (Java) | 1 | ~8 | JavaFX + transitive deps |
| `astraeus-tools` (Java) | 3 | <15 | YAML/JSON/CLI parsers |

### Binary Size Budget

- **Engine shared library (release)**: <5 MB (excluding third-party like OpenGL)
- **Java runtime JAR**: <2 MB (excluding JavaFX)
- **Tools JAR**: <5 MB (may include heavier dependencies)

---

## Platform Abstraction Rules

### Must Follow

1. **All platform-specific code lives in `engine/platform/`**
   - Only `engine/platform/` may contain `#ifdef _WIN32`, `#ifdef __linux__`, etc.
   - Only `engine/platform/` may include `<windows.h>`, X11 headers, etc.

2. **Public headers are platform-agnostic**
   - Headers in `engine/api/`, `engine/core/`, `engine/renderer/`, etc. must not include platform headers
   - Use forward declarations and opaque handles where needed

3. **Use `Platform.hpp` for all platform operations**
   - Never call Win32/POSIX APIs directly outside `engine/platform/`
   - All platform functionality exposed through `platform::*` functions

4. **CMake compile definitions centralized**
   - `NOMINMAX`, `WIN32_LEAN_AND_MEAN` defined in root CMakeLists.txt
   - No scattered definitions in individual files

5. **GL headers isolated in `engine/platform/GL/GLHeaders.hpp`**
   - Prevents GL header pollution into engine headers
   - Single point of control for GL version and extensions

### Never Do

1. ❌ **Never use `<windows.h>` outside `engine/platform/Win32/`**
2. ❌ **Never use `#ifdef _WIN32` outside `engine/platform/` or CMakeLists.txt**
3. ❌ **Never leak HWND, HDC, or other platform types into public headers**
4. ❌ **Never define `NOMINMAX` or `WIN32_LEAN_AND_MEAN` in individual files**
5. ❌ **Never use `min()/max()` macros - use `std::min()` / `std::max()`**

---

## Include Hygiene Guidelines

### Public Headers (engine/api/*.h, engine/core/*.hpp)

**Allowed includes:**
```cpp
#include <cstdint>
#include <cstddef>
#include <span>
#include <string_view>
#include <array>
#include "Types.hpp"  // Your own types
```

**Discouraged includes:**
```cpp
#include <string>     // Prefer std::string_view in headers
#include <vector>     // Prefer std::span or forward declare
#include <memory>     // Use forward declarations + PIMPL
```

**Never include in public headers:**
```cpp
#include <windows.h>      // ❌ Use platform::* instead
#include <iostream>       // ❌ Too heavy, use Log.hpp
#include <algorithm>      // ❌ Include only in .cpp files
#include <glad/glad.h>    // ❌ GL types only in backend
```

### Implementation Files (.cpp)

Implementation files have more flexibility but should still prefer:
1. Forward declarations over includes where possible
2. Include order: own header, C++ std, third-party, engine headers
3. Minimal includes for faster compilation

### Example: Compliant Header

```cpp
// engine/scene/Transform.hpp
#ifndef ASTRAEUS_TRANSFORM_HPP
#define ASTRAEUS_TRANSFORM_HPP

#include <cstdint>
#include <array>

namespace astraeus {

// Compliant: uses only standard types
struct Transform {
    std::array<float, 3> position;
    std::array<float, 4> rotation;  // quaternion
    std::array<float, 3> scale;
};

} // namespace astraeus

#endif
```

### Example: Non-Compliant Header (Don't Do This)

```cpp
// ❌ BAD: Leaks platform headers
#ifdef _WIN32
#include <windows.h>  // ❌ Platform header in public API
#endif

// ❌ BAD: Heavy includes
#include <vector>     // ❌ Use std::span instead
#include <string>     // ❌ Use std::string_view instead
#include <iostream>   // ❌ Too heavy for headers

namespace astraeus {

// ❌ BAD: Platform types in public API
#ifdef _WIN32
    HWND get_window_handle();  // ❌ HWND is Win32-specific
#endif

} // namespace astraeus
```

---

## Java Module Structure & Dependency Isolation

### Module Boundaries

Astraeus Java code is organized into three logical modules with strict dependency rules:

#### 1. `astraeus-native` (FFM Bindings)
- **Location**: `java/src/main/java/com/astraeus/native_api/`
- **Purpose**: FFM bindings to C++ engine, utility classes
- **Dependencies**: JDK only (no JavaFX, no third-party libs)
- **Budget**: 0 third-party dependencies

#### 2. `astraeus-ui` (JavaFX Application)
- **Location**: `java/src/main/java/com/astraeus/ui/`, `java/src/main/java/com/astraeus/rendering/`
- **Purpose**: JavaFX UI, viewport management, user interaction
- **Dependencies**: `astraeus-native` + JavaFX
- **Budget**: JavaFX only (1 primary dependency)

#### 3. `astraeus-tools` (Tooling & Debugging)
- **Location**: `java/src/main/java/com/astraeus/tools/`
- **Purpose**: Schema generation, debugging tools, data import/export
- **Dependencies**: `astraeus-native` + YAML/JSON libraries
- **Budget**: Max 3 primary dependencies (YAML, JSON, CLI parser)

### Dependency Flow Rules

```
astraeus-tools  →  astraeus-native  ← astraeus-ui
     ↓                    ↑                 ↓
  YAML/JSON           JDK only          JavaFX
  (not in runtime)
```

**Key Rules:**
1. ✅ `astraeus-ui` can depend on `astraeus-native`
2. ✅ `astraeus-tools` can depend on `astraeus-native`
3. ❌ `astraeus-native` must NOT depend on JavaFX or tools libraries
4. ❌ `astraeus-ui` must NOT depend on `astraeus-tools`
5. ❌ Tool dependencies (YAML/JSON) must NOT appear in runtime classpath

### Gradle Enforcement (Future)

When splitting into Gradle subprojects:

```kotlin
// Future: settings.gradle.kts
include("astraeus-native")
include("astraeus-ui")
include("astraeus-tools")

// Future: astraeus-ui/build.gradle.kts
dependencies {
    implementation(project(":astraeus-native"))
    implementation("org.openjfx:javafx-controls:25.0.1")
}

// Future: astraeus-tools/build.gradle.kts
dependencies {
    implementation(project(":astraeus-native"))
    implementation("org.yaml:snakeyaml:2.0")  // Tools only
}
```

---

## Approval Checklist for New Dependencies

Before adding any new dependency, answer these questions:

### 1. Problem Statement
- [ ] What problem does this dependency solve?
- [ ] Can you solve it with existing dependencies (stdlib, JDK)?
- [ ] Is the problem specific enough to justify the dependency?

### 2. Alternatives Considered
- [ ] Have you evaluated at least 2 alternatives?
- [ ] Why is this dependency better than alternatives?
- [ ] Did you consider implementing the feature yourself?

### 3. Module Placement
- [ ] Is this a runtime or tooling dependency?
- [ ] Which module will use it? (`core`, `renderer`, `tools`, `ui`)
- [ ] Does it fit within the module's dependency budget?

### 4. Impact Assessment
- [ ] Binary size impact? (Run `ls -lh` on built library)
- [ ] Build time impact? (Measure before/after)
- [ ] Deployment complexity? (Additional DLLs/JARs to ship?)
- [ ] Cross-platform support? (Works on Windows + Linux?)

### 5. License & Legal
- [ ] What is the license? (MIT/BSD/Apache 2.0 preferred)
- [ ] Is it compatible with Astraeus license?
- [ ] Any patent concerns?

### 6. Maintenance & Stability
- [ ] Is it actively maintained? (Recent commits/releases)
- [ ] What is the ABI stability guarantee?
- [ ] How often does it break backwards compatibility?

### 7. Documentation
- [ ] Update `DEPENDENCIES.md` with dependency rationale
- [ ] Document version requirements
- [ ] Note any special build/configuration needs

---

## Platform Module Interface

### Location: `engine/platform/Platform.hpp`

The platform module provides a **minimal, stable interface** for platform-specific operations:

```cpp
namespace astraeus::platform {

// Initialization
void init();

// Time
uint64_t monotonic_time_ns();

// OpenGL
void* load_gl_proc(const char* name);

// Optional utilities (add as needed)
void set_thread_name(const char* name);
size_t get_page_size();

} // namespace astraeus::platform
```

**Guidelines:**
- Keep interface minimal - add functions only when needed by multiple modules
- All functions must be implementable on Windows and Linux
- No platform types (HWND, HDC, etc.) in the interface
- Document each function's behavior and guarantees

### Platform-Specific Files

#### Windows: `engine/platform/Win32/`
- `Win32Headers.hpp` - Includes `<windows.h>` with proper guards
- `Win32Platform.cpp` - Implements `Platform.hpp` for Windows

#### Linux: `engine/platform/Linux/`
- `X11Headers.hpp` - Includes X11 headers (if needed)
- `LinuxPlatform.cpp` - Implements `Platform.hpp` for Linux

#### OpenGL: `engine/platform/GL/`
- `GLHeaders.hpp` - Isolates GLAD and OpenGL headers

**Only these files may:**
- Use `#ifdef _WIN32` / `#ifdef __linux__`
- Include `<windows.h>`, X11 headers, etc.
- Call platform-specific APIs

---

## Build System Enforcement

### CMakeLists.txt Centralized Definitions

```cmake
# Root CMakeLists.txt
if(WIN32)
    add_compile_definitions(NOMINMAX)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()

# Platform module
add_library(astraeus_platform STATIC
    platform/Platform.cpp
    $<$<PLATFORM_ID:Windows>:platform/Win32/Win32Platform.cpp>
    $<$<PLATFORM_ID:Linux>:platform/Linux/LinuxPlatform.cpp>
)
```

### CI Include Hygiene Check (Optional)

Create a CI step to validate include hygiene:

```bash
# .github/workflows/check-includes.sh
#!/bin/bash

# Check for platform headers outside engine/platform/
if grep -r "#include <windows.h>" engine --include="*.hpp" --include="*.cpp" \
   --exclude-dir=platform; then
    echo "ERROR: <windows.h> found outside engine/platform/"
    exit 1
fi

# Check for scattered ifdefs
if grep -r "#ifdef _WIN32\|#if defined(_WIN32)" engine --include="*.hpp" --include="*.cpp" \
   --exclude-dir=platform --exclude="CMakeLists.txt"; then
    echo "ERROR: Platform ifdefs found outside engine/platform/"
    exit 1
fi

echo "Include hygiene check passed"
```

---

## Extension Points

### How to Add a New Dependency

1. **Fill out the Approval Checklist** above
2. **Document in this file** (add to appropriate tier)
3. **Update module budget** if needed
4. **Add to CMakeLists.txt** or `build.gradle.kts`
5. **Add license info** to `LICENSE` or `THIRD_PARTY_NOTICES`
6. **Coordinate with Build Agent** for cross-platform integration

### How to Add Platform-Specific Code

1. **Check if `Platform.hpp` has what you need**
   - If yes, use existing interface
   - If no, continue to step 2

2. **Propose a new `platform::*` function**
   - Function name and signature
   - Behavior on Windows and Linux
   - Why it's needed by multiple modules

3. **Implement in platform module**
   - Update `Platform.hpp` with declaration
   - Update `Win32Platform.cpp` with Windows implementation
   - Update `LinuxPlatform.cpp` with Linux implementation (or stub)

4. **Never bypass the abstraction**
   - Don't call Win32 APIs directly from engine code
   - Don't add scattered `#ifdef _WIN32` blocks

### How to Add a New Render Pass

See [ARCHITECTURE.md - How to Add a Render Pass](ARCHITECTURE.md#how-to-add-a-render-pass)

### How to Add a New Java Tool Panel

See [ARCHITECTURE.md - How to Add a Java Tool Panel](ARCHITECTURE.md#how-to-add-a-java-tool-panel)

---

## Enforcement & Compliance

### Automated Checks
- [ ] CI script validates no `<windows.h>` outside `engine/platform/Win32/`
- [ ] CI script validates no scattered `#ifdef` outside `engine/platform/`
- [ ] Gradle build enforces module dependencies (when split into subprojects)

### Manual Review
- All PRs adding dependencies require architecture review
- Dependency budget violations require justification and approval
- Include hygiene reviewed during code review

### Exceptions
Exceptions to these rules require:
1. Written justification in PR description
2. Approval from project maintainer
3. Documentation update in this file

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-31 | Initial dependency and platform abstraction policy |

---

## References

- [ARCHITECTURE.md](ARCHITECTURE.md) - Overall system architecture
- [ABI_CODEGEN_GUIDE.md](ABI_CODEGEN_GUIDE.md) - FFM/ABI guidelines
- C++ Core Guidelines: [SF.11: Header files should be self-contained](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#sf11-header-files-should-be-self-contained)
