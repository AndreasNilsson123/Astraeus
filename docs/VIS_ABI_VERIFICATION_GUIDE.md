# ABI Verification & Validation Guide

**Document**: VIS-GEN-001 ABI/FFM Verification  
**Purpose**: Define ABI stability contracts and verification procedures  
**Scope**: C API ↔ Java FFM boundary validation

---

## 1. Overview

The ABI (Application Binary Interface) boundary between C++ and Java is critical for correctness. This document defines:

- **Struct layout contracts** and validation procedures
- **Alignment and padding** requirements
- **Platform compatibility** testing
- **Automated verification** tools

### 1.1 Stability Guarantees

The Astraeus ABI provides:

1. **Forward Compatibility**: Old Java code works with newer native library (within major version)
2. **Struct Size Stability**: Struct sizes never decrease, only increase with padding
3. **Field Offset Stability**: Existing field offsets never change
4. **Explicit Padding**: All padding is documented and verified
5. **Platform Independence**: Same struct layouts on Windows/Linux/macOS x64

---

## 2. Struct Layout Verification

### 2.1 Target Platform: x64 (64-bit)

All struct layouts target **x64 architecture** with standard System V ABI (Linux/macOS) and Microsoft x64 ABI (Windows).

**Common Alignment Rules**:
- `uint8_t`, `int8_t`: 1-byte alignment
- `uint16_t`, `int16_t`: 2-byte alignment
- `uint32_t`, `int32_t`, `float`: 4-byte alignment
- `uint64_t`, `int64_t`, `double`: 8-byte alignment
- Pointers (`void*`): 8-byte alignment (x64)
- Struct alignment: Largest member alignment (max 8 bytes typically)

### 2.2 Critical Structs

#### 2.2.1 PixelBufferView

**Expected Layout (x64)**:
```c
typedef struct {
    void* data;                  // Offset: 0,  Size: 8,  Align: 8
    uint32_t width;              // Offset: 8,  Size: 4,  Align: 4
    uint32_t height;             // Offset: 12, Size: 4,  Align: 4
    uint32_t stride;             // Offset: 16, Size: 4,  Align: 4
    uint32_t format;             // Offset: 20, Size: 4,  Align: 4
    uint32_t max_backing_width;  // Offset: 24, Size: 4,  Align: 4
    uint32_t max_backing_height; // Offset: 28, Size: 4,  Align: 4
    uint32_t max_backing_size;   // Offset: 32, Size: 4,  Align: 4
    // No padding needed: total = 36 bytes, align = 8
    // Padded to: 40 bytes (nearest multiple of 8)
} PixelBufferView;

static_assert(sizeof(PixelBufferView) == 40, "PixelBufferView size must be 40 bytes");
static_assert(alignof(PixelBufferView) == 8, "PixelBufferView alignment must be 8 bytes");
```

**Java FFM Layout**:
```java
public static final MemoryLayout PIXEL_BUFFER_VIEW = MemoryLayout.structLayout(
    ValueLayout.ADDRESS.withName("data"),           // 0-7
    ValueLayout.JAVA_INT.withName("width"),         // 8-11
    ValueLayout.JAVA_INT.withName("height"),        // 12-15
    ValueLayout.JAVA_INT.withName("stride"),        // 16-19
    ValueLayout.JAVA_INT.withName("format"),        // 20-23
    ValueLayout.JAVA_INT.withName("max_backing_width"),  // 24-27
    ValueLayout.JAVA_INT.withName("max_backing_height"), // 28-31
    ValueLayout.JAVA_INT.withName("max_backing_size"),   // 32-35
    MemoryLayout.paddingLayout(32)                  // 36-39 (4 bytes padding)
).withName("PixelBufferView");
```

**Verification**:
```c
// In C++ (compile-time)
static_assert(offsetof(PixelBufferView, data) == 0);
static_assert(offsetof(PixelBufferView, width) == 8);
static_assert(offsetof(PixelBufferView, height) == 12);
static_assert(offsetof(PixelBufferView, stride) == 16);
static_assert(offsetof(PixelBufferView, format) == 20);
static_assert(offsetof(PixelBufferView, max_backing_width) == 24);
static_assert(offsetof(PixelBufferView, max_backing_height) == 28);
static_assert(offsetof(PixelBufferView, max_backing_size) == 32);
```

```java
// In Java (runtime)
@Test
public void testPixelBufferViewLayout() {
    assertEquals(40, PIXEL_BUFFER_VIEW.byteSize());
    assertEquals(8, PIXEL_BUFFER_VIEW.byteAlignment());
    
    // Verify field offsets
    assertEquals(0, PIXEL_BUFFER_VIEW.byteOffset(PathElement.groupElement("data")));
    assertEquals(8, PIXEL_BUFFER_VIEW.byteOffset(PathElement.groupElement("width")));
    assertEquals(12, PIXEL_BUFFER_VIEW.byteOffset(PathElement.groupElement("height")));
    // ... etc
}
```

#### 2.2.2 FrameStats

**Expected Layout (x64)**:
```c
typedef struct {
    uint64_t frame_number;     // Offset: 0,  Size: 8,  Align: 8
    double delta_time_ms;      // Offset: 8,  Size: 8,  Align: 8
    double render_time_ms;     // Offset: 16, Size: 8,  Align: 8
    uint32_t draw_calls;       // Offset: 24, Size: 4,  Align: 4
    uint32_t triangle_count;   // Offset: 28, Size: 4,  Align: 4
    uint32_t entity_count;     // Offset: 32, Size: 4,  Align: 4
    uint8_t _padding[4];       // Offset: 36, Size: 4,  Align: 1
    // Total: 40 bytes, align: 8
} FrameStats;

static_assert(sizeof(FrameStats) == 40);
static_assert(alignof(FrameStats) == 8);
```

**Java FFM Layout**:
```java
public static final MemoryLayout FRAME_STATS = MemoryLayout.structLayout(
    ValueLayout.JAVA_LONG.withName("frame_number"),     // 0-7
    ValueLayout.JAVA_DOUBLE.withName("delta_time_ms"),  // 8-15
    ValueLayout.JAVA_DOUBLE.withName("render_time_ms"), // 16-23
    ValueLayout.JAVA_INT.withName("draw_calls"),        // 24-27
    ValueLayout.JAVA_INT.withName("triangle_count"),    // 28-31
    ValueLayout.JAVA_INT.withName("entity_count"),      // 32-35
    MemoryLayout.paddingLayout(32)                      // 36-39
).withName("FrameStats");
```

#### 2.2.3 CameraDesc

**Expected Layout (x64)**:
```c
typedef struct {
    float pos_x, pos_y, pos_z;           // 0-11   (12 bytes)
    float target_x, target_y, target_z;  // 12-23  (12 bytes)
    float up_x, up_y, up_z;              // 24-35  (12 bytes)
    float fov_degrees;                   // 36-39  (4 bytes)
    float near_plane;                    // 40-43  (4 bytes)
    float far_plane;                     // 44-47  (4 bytes)
    uint32_t mode;                       // 48-51  (4 bytes)
    uint8_t _padding[4];                 // 52-55  (4 bytes padding)
    // Total: 56 bytes, align: 4 (or 8 if aligned to 8)
} CameraDesc;

// Note: Might need to pad to 56 or 64 bytes depending on struct alignment
static_assert(sizeof(CameraDesc) == 56 || sizeof(CameraDesc) == 64);
```

---

## 3. ABI Self-Check API

### 3.1 Native Side

```c
/**
 * ABI version and validation info.
 */
typedef struct {
    uint32_t api_version;
    uint32_t abi_version;
    
    // Struct sizes
    uint32_t sizeof_PixelBufferView;
    uint32_t sizeof_FrameStats;
    uint32_t sizeof_CameraDesc;
    uint32_t sizeof_MaterialDesc;
    uint32_t sizeof_PickResult;
    
    // Struct alignments
    uint32_t alignof_PixelBufferView;
    uint32_t alignof_FrameStats;
    
    // Platform info
    char platform_name[32];     // "Linux-x64", "Windows-x64", etc.
    uint32_t pointer_size;      // sizeof(void*)
    
} ABIInfo;

/**
 * Get ABI information for validation.
 * Call this at startup to verify Java and C++ agree on struct layouts.
 */
ASTRAEUS_API void astraeus_get_abi_info(ABIInfo* out_info);
```

**Implementation**:
```cpp
void astraeus_get_abi_info(ABIInfo* out_info) {
    if (!out_info) return;
    
    out_info->api_version = astraeus_api_version();
    out_info->abi_version = 1;  // Bump when struct layouts change
    
    out_info->sizeof_PixelBufferView = sizeof(PixelBufferView);
    out_info->sizeof_FrameStats = sizeof(FrameStats);
    out_info->sizeof_CameraDesc = sizeof(CameraDesc);
    out_info->sizeof_MaterialDesc = sizeof(MaterialDesc);
    out_info->sizeof_PickResult = sizeof(PickResult);
    
    out_info->alignof_PixelBufferView = alignof(PixelBufferView);
    out_info->alignof_FrameStats = alignof(FrameStats);
    
    #ifdef _WIN32
        strncpy(out_info->platform_name, "Windows-x64", 32);
    #elif defined(__linux__)
        strncpy(out_info->platform_name, "Linux-x64", 32);
    #elif defined(__APPLE__)
        strncpy(out_info->platform_name, "macOS-x64", 32);
    #else
        strncpy(out_info->platform_name, "Unknown", 32);
    #endif
    
    out_info->pointer_size = sizeof(void*);
}
```

### 3.2 Java Side

```java
public class ABIValidator {
    
    /**
     * Validate ABI compatibility at startup.
     * Throws exception if Java and native layouts don't match.
     */
    public static void validateABI(NativeEngine engine) {
        ABIInfo info = engine.getABIInfo();
        
        List<String> errors = new ArrayList<>();
        
        // Check struct sizes
        long expectedPixelBufferViewSize = PixelBufferViewLayout.LAYOUT.byteSize();
        if (info.sizeofPixelBufferView != expectedPixelBufferViewSize) {
            errors.add(String.format(
                "PixelBufferView size mismatch: native=%d, Java=%d",
                info.sizeofPixelBufferView, expectedPixelBufferViewSize));
        }
        
        long expectedFrameStatsSize = FrameStatsLayout.LAYOUT.byteSize();
        if (info.sizeofFrameStats != expectedFrameStatsSize) {
            errors.add(String.format(
                "FrameStats size mismatch: native=%d, Java=%d",
                info.sizeofFrameStats, expectedFrameStatsSize));
        }
        
        // Check pointer size
        long expectedPointerSize = ValueLayout.ADDRESS.byteSize();
        if (info.pointerSize != expectedPointerSize) {
            errors.add(String.format(
                "Pointer size mismatch: native=%d, Java=%d",
                info.pointerSize, expectedPointerSize));
        }
        
        // Report errors
        if (!errors.isEmpty()) {
            String message = "ABI validation failed:\n" + 
                String.join("\n", errors) +
                "\nPlatform: " + info.platformName +
                "\nAPI Version: " + info.apiVersion +
                "\nABI Version: " + info.abiVersion;
            
            throw new RuntimeException(message);
        }
        
        System.out.println("✓ ABI validation passed");
        System.out.println("  Platform: " + info.platformName);
        System.out.println("  API Version: " + info.apiVersion);
        System.out.println("  ABI Version: " + info.abiVersion);
    }
}
```

### 3.3 Startup Validation

```java
public class AstraeusApp extends Application {
    
    @Override
    public void start(Stage primaryStage) {
        try {
            // Load native library
            System.loadLibrary("astraeus");
            
            // Create engine
            NativeEngine engine = new NativeEngine(1280, 720, true);
            
            // CRITICAL: Validate ABI before any FFM calls
            ABIValidator.validateABI(engine);
            
            // Continue with app initialization...
            
        } catch (Exception e) {
            showErrorDialog("Failed to initialize Astraeus", e);
            Platform.exit();
        }
    }
}
```

---

## 4. Codegen Validation

### 4.1 Schema-Based Size Verification

The ABI codegen tool (`java/codegen`) generates both C and Java struct layouts from `abi_structs_schema.yaml`. Add automatic size verification:

**In generated C header**:
```c
// Auto-generated by codegen
// DO NOT EDIT

// Compile-time size assertions
static_assert(sizeof(PixelBufferView) == 40, 
    "PixelBufferView size changed! Update schema and ABI version.");
static_assert(sizeof(FrameStats) == 40,
    "FrameStats size changed! Update schema and ABI version.");
// ... etc for all structs

// Runtime size validation function
static inline bool astraeus_validate_struct_sizes(void) {
    return sizeof(PixelBufferView) == 40 &&
           sizeof(FrameStats) == 40 &&
           sizeof(CameraDesc) == 56;
}
```

**In generated Java class**:
```java
// Auto-generated by codegen
// DO NOT EDIT

public class GeneratedABILayouts {
    
    public static final long EXPECTED_PIXELBUFFERVIEW_SIZE = 40;
    public static final long EXPECTED_FRAMESTATS_SIZE = 40;
    public static final long EXPECTED_CAMERADESC_SIZE = 56;
    
    /**
     * Verify generated layouts match expected sizes.
     * Call at class initialization time.
     */
    static {
        assert PixelBufferViewLayout.LAYOUT.byteSize() == EXPECTED_PIXELBUFFERVIEW_SIZE
            : "PixelBufferView layout size mismatch";
        assert FrameStatsLayout.LAYOUT.byteSize() == EXPECTED_FRAMESTATS_SIZE
            : "FrameStats layout size mismatch";
        // ... etc
    }
}
```

### 4.2 Offset Verification

Generate offset verification code:

**C**:
```c
// Generated offset checks
#define CHECK_OFFSET(STRUCT, FIELD, EXPECTED) \
    static_assert(offsetof(STRUCT, FIELD) == EXPECTED, \
        #STRUCT "." #FIELD " offset changed")

CHECK_OFFSET(PixelBufferView, data, 0);
CHECK_OFFSET(PixelBufferView, width, 8);
CHECK_OFFSET(PixelBufferView, height, 12);
// ... etc
```

**Java**:
```java
// Runtime offset verification in unit tests
@Test
public void testPixelBufferViewOffsets() {
    MemoryLayout layout = PixelBufferViewLayout.LAYOUT;
    
    assertEquals(0, layout.byteOffset(PathElement.groupElement("data")));
    assertEquals(8, layout.byteOffset(PathElement.groupElement("width")));
    assertEquals(12, layout.byteOffset(PathElement.groupElement("height")));
    // ... etc
}
```

---

## 5. Platform-Specific Considerations

### 5.1 Windows x64

**ABI**: Microsoft x64 calling convention
**Struct Alignment**: Same as Linux for POD types
**Pointer Size**: 8 bytes
**Issues**: None expected for POD structs

**Validation**:
```bash
# Build on Windows
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release

# Run ABI test
./bin/Release/abi_test.exe
```

### 5.2 Linux x64

**ABI**: System V AMD64 ABI
**Struct Alignment**: Standard C rules
**Pointer Size**: 8 bytes
**Issues**: None expected

**Validation**:
```bash
# Build on Linux
mkdir build && cd build
cmake ..
cmake --build .

# Run ABI test
./bin/abi_test
```

### 5.3 macOS x64 / ARM64

**ABI**: Similar to Linux (System V derived)
**ARM64 Considerations**: Different pointer size (still 8 bytes), but may need separate validation
**Issues**: ARM64 alignment can differ for some types

**Validation**: Test on both Intel and Apple Silicon Macs

---

## 6. Padding Best Practices

### 6.1 Explicit Padding Rules

**Always add explicit padding fields**:
```c
// GOOD
typedef struct {
    uint32_t field1;
    uint8_t _padding1[4];  // Explicit
    uint64_t field2;
} GoodStruct;

// BAD
typedef struct {
    uint32_t field1;
    // Implicit padding (compiler-dependent)
    uint64_t field2;
} BadStruct;
```

### 6.2 Naming Convention

- Padding fields: `_padding`, `_padding1`, `_padding2`, etc.
- Padding is always `uint8_t` array
- Document reason for padding

**Example**:
```c
typedef struct {
    uint32_t field1;
    uint8_t _padding[4];  // Align next field to 8 bytes
    uint64_t field2;
} MyStruct;
```

### 6.3 Schema Padding Specification

In `abi_structs_schema.yaml`:
```yaml
structs:
  - name: MyStruct
    fields:
      - name: field1
        type: uint32
      - name: _padding
        type: uint8
        array_size: 4
        description: "Align field2 to 8 bytes"
      - name: field2
        type: uint64
```

---

## 7. Testing Strategy

### 7.1 Unit Tests

**C++ Tests** (`engine/tests/abi_test.cpp`):
```cpp
TEST(ABI, StructSizes) {
    EXPECT_EQ(sizeof(PixelBufferView), 40);
    EXPECT_EQ(sizeof(FrameStats), 40);
    EXPECT_EQ(sizeof(CameraDesc), 56);
}

TEST(ABI, StructAlignments) {
    EXPECT_EQ(alignof(PixelBufferView), 8);
    EXPECT_EQ(alignof(FrameStats), 8);
}

TEST(ABI, FieldOffsets) {
    EXPECT_EQ(offsetof(PixelBufferView, data), 0);
    EXPECT_EQ(offsetof(PixelBufferView, width), 8);
    EXPECT_EQ(offsetof(PixelBufferView, height), 12);
}
```

**Java Tests** (`java/frontend/src/test/java/ABITest.java`):
```java
@Test
public void testStructSizes() {
    assertEquals(40, PixelBufferViewLayout.LAYOUT.byteSize());
    assertEquals(40, FrameStatsLayout.LAYOUT.byteSize());
    assertEquals(56, CameraDescLayout.LAYOUT.byteSize());
}

@Test
public void testABICompatibility() {
    NativeEngine engine = new NativeEngine(800, 600, true);
    ABIValidator.validateABI(engine);
    engine.close();
}
```

### 7.2 Integration Tests

**Round-trip test**:
```java
@Test
public void testPixelBufferViewRoundTrip() {
    NativeEngine engine = new NativeEngine(800, 600, true);
    
    engine.beginFrame(0.016);
    engine.endFrame();
    
    PixelBufferView view = engine.getColorBuffer();
    
    // Verify all fields are readable and sane
    assertNotEquals(0, view.dataAddress());
    assertEquals(800, view.width());
    assertEquals(600, view.height());
    assertTrue(view.stride() >= 800 * 4);
    assertEquals(PixelFormat.BGRA8.value(), view.format());
    
    engine.close();
}
```

### 7.3 Continuous Integration

Add to CI pipeline:
```yaml
# .github/workflows/ci.yml
- name: Run ABI Tests (C++)
  run: |
    cd build
    ctest --output-on-failure -R abi_test

- name: Run ABI Tests (Java)
  run: |
    mvn test -Dtest=ABITest
```

---

## 8. ABI Versioning

### 8.1 Version Numbering

**ABI Version Format**: `MAJOR.MINOR`

- **MAJOR**: Incremented for breaking changes (struct size/offset changes)
- **MINOR**: Incremented for backward-compatible additions (new fields at end)

**Example**:
- ABI 1.0: Initial release
- ABI 1.1: Added new field at end of FrameStats (backward compatible)
- ABI 2.0: Changed PixelBufferView layout (breaking change)

### 8.2 Compatibility Checks

```c
#define ASTRAEUS_ABI_VERSION_MAJOR 1
#define ASTRAEUS_ABI_VERSION_MINOR 0

ASTRAEUS_API uint32_t astraeus_abi_version(void) {
    return (ASTRAEUS_ABI_VERSION_MAJOR << 16) | ASTRAEUS_ABI_VERSION_MINOR;
}
```

```java
public class ABIValidator {
    private static final int REQUIRED_ABI_MAJOR = 1;
    
    public static void validateABI(NativeEngine engine) {
        int version = engine.getABIVersion();
        int major = version >> 16;
        int minor = version & 0xFFFF;
        
        if (major != REQUIRED_ABI_MAJOR) {
            throw new RuntimeException(
                "ABI version mismatch: Java requires " + REQUIRED_ABI_MAJOR +
                ".x, native has " + major + "." + minor);
        }
        
        System.out.println("ABI version: " + major + "." + minor);
    }
}
```

---

## 9. Common Pitfalls

### 9.1 Implicit Padding

**Problem**: Compiler adds implicit padding, layouts don't match

**Solution**: Always add explicit padding fields

### 9.2 Platform Differences

**Problem**: Struct sizes differ on Windows vs Linux

**Solution**: Test on all target platforms, use fixed-size types

### 9.3 Pointer Size Assumptions

**Problem**: Assuming pointers are always 8 bytes

**Solution**: Use `sizeof(void*)` and `ValueLayout.ADDRESS`, verify at runtime

### 9.4 Alignment Mistakes

**Problem**: Misaligned field access causes crashes or corruption

**Solution**: Use generated layouts, add compile-time assertions

### 9.5 ByteBuffer Position State

**Problem**: Forgetting to rewind ByteBuffer after writing

**Solution**: Always call `buffer.clear()` or `buffer.rewind()` after populating

---

## 10. Summary

**Critical Actions**:

✅ All structs have explicit padding documented in schema  
✅ Generated layouts verified at compile-time (C++) and runtime (Java)  
✅ ABI self-check called at engine initialization  
✅ Unit tests for struct sizes, alignments, and offsets  
✅ Integration tests for round-trip data transfer  
✅ CI pipeline runs ABI tests on all platforms  
✅ ABI version checked and documented  

**Verification Checklist**:

- [ ] Run `astraeus_get_abi_info()` and print results
- [ ] Run all ABI unit tests (C++ and Java)
- [ ] Test on target platforms (Linux x64, Windows x64)
- [ ] Verify no warnings about struct padding
- [ ] Document any platform-specific quirks
- [ ] Update ABI version if layouts change
