# Native API Module

This module provides the FFM (Foreign Function & Memory) bindings for the Astraeus engine C API.

## Structure

```
native_api/
├── EngineBindings.java       # Main FFM bindings (method handles, function descriptors)
├── NativeEngine.java          # High-level engine wrapper (AutoCloseable)
├── NativeViewport.java        # Viewport wrapper
├── NativeCamera.java          # Camera wrapper
├── NativeMaterial.java        # Material wrapper
├── PickingView.java          # Picking results view
├── layout/                   # Layout definitions and constants
│   ├── Constants.java        # All API constants (result codes, pixel formats, etc.)
│   └── LayoutValidator.java  # Runtime layout validation
└── util/                     # Utility classes
    ├── NativeStrings.java    # Safe string read/write helpers
    └── NativeChecks.java     # Error handling and validation
```

## Key Components

### EngineBindings.java

Low-level FFM bindings that map to C API functions. Provides:
- `MethodHandle` instances for all native functions
- `FunctionDescriptor` definitions for C ABI signatures
- `StructLayout` references (from auto-generated `StructLayouts.java`)

**Usage:**
```java
// Direct use (not recommended for application code)
MemorySegment handle = (MemorySegment) EngineBindings.CREATE_ENGINE.invokeExact(configPtr);
```

### Constants.java

Centralized API constants that must match `EngineAPI.h`:
- Result codes (`ASTRAEUS_SUCCESS`, `ASTRAEUS_ERROR_*`)
- Pixel formats (`PIXEL_FORMAT_RGBA8`, etc.)
- Camera modes (`CAMERA_MODE_ORBIT`, etc.)
- Alpha modes (`ALPHA_MODE_OPAQUE`, etc.)
- Buffer size limits

**Usage:**
```java
import static com.astraeus.native_api.layout.Constants.*;

if (result == ASTRAEUS_SUCCESS) {
    // Success
}
```

### NativeStrings.java

Safe string operations across the FFM boundary:
- `readCString()` - Read null-terminated C strings with bounds checking
- `writeCString()` - Write Java strings as null-terminated C strings
- `allocateCString()` - Allocate and write in one step
- `fitsInBuffer()` - Validate string size before writing

**Usage:**
```java
// Read error message from native code
String error = NativeStrings.readCString(errorPtr, Constants.MAX_ERROR_MESSAGE_LENGTH);

// Write string to native buffer
try (Arena arena = Arena.ofConfined()) {
    MemorySegment nameBuffer = arena.allocate(64);
    NativeStrings.writeCString(nameBuffer, "MyEntity", 64);
    // Use nameBuffer...
}

// Allocate new string
try (Arena arena = Arena.ofConfined()) {
    MemorySegment str = NativeStrings.allocateCString(arena, "Hello");
    // Use str...
}
```

### NativeChecks.java

Error checking and validation utilities:
- `checkResult()` - Throw exception if result code indicates error
- `requireValidHandle()` - Validate handle is non-null
- `requireInRange()` - Validate parameter bounds
- `asBoolean()` / `fromBoolean()` - Convert between Java and C booleans

**Usage:**
```java
import static com.astraeus.native_api.util.NativeChecks.*;

// Check result code
int result = callNativeFunction();
checkResult(result, "Create viewport");

// With context
checkResult(result, "Resize viewport", "width=" + width + ", height=" + height);

// Validate handle
requireValidHandle(engineHandle, "engine");

// Parameter validation
requireInRange(width, 1, 4096, "width");
requireNonNegative(entityId, "entityId");
```

### LayoutValidator.java

Runtime validation of struct layouts:
- Validates that Java FFM layouts match expected C ABI sizes
- Checks alignment and padding
- Useful for detecting platform-specific ABI differences

**Usage:**
```java
// Enable validation with system property:
// -Dastraeus.validate.layouts=true
// or
// -Dastraeus.mode=dev

// In development mode, validation runs automatically during EngineBindings initialization

// Manual validation:
if (LayoutValidator.ENABLE_VALIDATION) {
    LayoutValidator.validateAllLayouts();
}

// Print layout information for debugging:
LayoutValidator.printAllLayouts();
```

## Auto-Generated Files

### StructLayouts.java (generated)

Auto-generated from `engine/api/abi_structs_schema.yaml` by `ABICodeGenerator`.

**Location:** `java/src/main/java/com/astraeus/generated/StructLayouts.java`

**Contains:**
- `StructLayout` definitions for all ABI structs
- `VarHandle` instances for struct field access
- Schema version and hash

**Regenerate with:**
```bash
./regenerate_abi.sh
```

**Note:** This file is ignored by git (auto-generated on build).

## Design Principles

### 1. Centralization
All constants, layouts, and utility patterns are centralized to prevent drift and inconsistency.

### 2. Safety
- Bounded string operations prevent buffer overruns
- Result code checking is mandatory
- Handle validation is explicit
- Parameter validation is encouraged

### 3. Clarity
- Descriptive error messages
- Consistent naming conventions
- Well-documented APIs

### 4. Platform Portability
- Runtime layout validation detects ABI mismatches
- UTF-8 encoding for all strings
- Explicit alignment and padding

## ABI Compatibility

The Java FFM bindings assume **x64 Windows/Linux with standard alignment**:
- Pointers: 8 bytes
- `uint32_t`: 4 bytes, 4-byte aligned
- `uint64_t`: 8 bytes, 8-byte aligned
- `double`: 8 bytes, 8-byte aligned
- `float`: 4 bytes, 4-byte aligned
- `bool`: 1 byte

Structs are padded to align to their largest member (typically 8 bytes for x64).

**Testing on New Platforms:**
1. Enable layout validation: `-Dastraeus.validate.layouts=true`
2. Run the application
3. Check console output for validation warnings
4. If mismatches occur, regenerate layouts or adjust schema

## Best Practices

### For Application Code
- Use high-level wrappers (`NativeEngine`, `NativeViewport`, etc.)
- Import constants from `Constants` class
- Use `NativeChecks` for all result code validation
- Use `NativeStrings` for all string operations

### For Wrapper Code
- Always check result codes with `NativeChecks.checkResult()`
- Validate handles with `NativeChecks.requireValidHandle()`
- Use bounded string operations from `NativeStrings`
- Provide descriptive error messages with context

### For Maintenance
- When adding new constants, add to `Constants.java`
- When adding new structs, update `abi_structs_schema.yaml` and regenerate
- When adding new native functions, add to `EngineBindings.java`
- Keep `EngineAPI.h` and Java constants in sync

## Requirements

- **Java 21+** (for finalized FFM API)
- **--enable-preview** (if using preview features)

## See Also

- `docs/ARCHITECTURE.md` - Overall architecture
- `engine/api/EngineAPI.h` - C API definition
- `engine/api/abi_structs_schema.yaml` - Struct schema
- `tools/ABICodeGenerator.java` - Code generator
