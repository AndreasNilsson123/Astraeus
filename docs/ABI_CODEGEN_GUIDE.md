# ABI Struct Code Generation Guide

## Overview

The Astraeus engine uses a **single-source code generation system** for ABI structs to prevent drift between C++ and Java FFM bindings. All struct definitions are maintained in a YAML schema file, and code is automatically generated for both languages.

## Architecture

```
abi_structs_schema.yaml  (Single Source of Truth)
         │
         ├──> ABICodeGenerator.java (Code Generator)
         │
         ├──> EngineABI_Structs.gen.h (C++ POD structs)
         └──> StructLayouts.gen.java  (Java FFM layouts)
```

## Prerequisites

### For Code Generation

The ABICodeGenerator tool can run with **Java 17+** as it only uses standard Java APIs (no FFM dependencies).

### For Building the Project

The Astraeus Java frontend requires **Java 21+** because it uses the Foreign Function & Memory (FFM) API.

**Note:** If you only need to regenerate ABI code, Java 17 is sufficient. The `regenerate_abi.sh` script will work with Java 17.

## Schema File Location

- **Schema:** `engine/api/abi_structs_schema.yaml`
- **C++ Output:** `engine/api/EngineABI_Structs.gen.h`
- **Java Output:** `java/src/main/java/com/astraeus/native_api/StructLayouts.gen.java`

## Schema Format

The schema is a YAML file with the following structure:

```yaml
version: "1.0.0"
namespace: "astraeus"

structs:
  - name: MyStruct
    description: "Description of the struct"
    fields:
      - name: field_name
        type: uint32
        description: "Field description"
      - name: array_field
        type: uint8
        array_size: 4
        description: "Array field (for padding)"
```

### Supported Types

| Schema Type | C Type      | Java Layout           | Size (bytes) | Alignment |
|-------------|-------------|-----------------------|--------------|-----------|
| `uint8`     | `uint8_t`   | `JAVA_BYTE`          | 1            | 1         |
| `uint16`    | `uint16_t`  | `JAVA_SHORT`         | 2            | 2         |
| `uint32`    | `uint32_t`  | `JAVA_INT`           | 4            | 4         |
| `uint64`    | `uint64_t`  | `JAVA_LONG`          | 8            | 8         |
| `int8`      | `int8_t`    | `JAVA_BYTE`          | 1            | 1         |
| `int16`     | `int16_t`   | `JAVA_SHORT`         | 2            | 2         |
| `int32`     | `int32_t`   | `JAVA_INT`           | 4            | 4         |
| `int64`     | `int64_t`   | `JAVA_LONG`          | 8            | 8         |
| `float32`   | `float`     | `JAVA_FLOAT`         | 4            | 4         |
| `float64`   | `double`    | `JAVA_DOUBLE`        | 8            | 8         |
| `bool`      | `bool`      | `JAVA_BOOLEAN`       | 1            | 1         |
| `pointer`   | `void*`     | `ADDRESS`            | 8 (x64)      | 8         |

## How to Add a New Struct

### Step 1: Add to Schema

Edit `engine/api/abi_structs_schema.yaml` and add your struct definition:

```yaml
  - name: MyNewStruct
    description: "My new struct for XYZ"
    fields:
      - name: field1
        type: uint32
        description: "First field"
      - name: field2
        type: float64
        description: "Second field"
      - name: _padding
        type: uint8
        array_size: 4
        description: "Padding for alignment"
```

**Important:** Always add padding fields to ensure proper alignment:
- Structs should be aligned to their largest field size
- Use `_padding` arrays to fill gaps
- Common alignments: 4 bytes (int/float), 8 bytes (long/double/pointer)

### Step 2: Regenerate Code

Run the code generator:

```bash
# Using Maven (recommended)
mvn generate-sources

# Or manually
java com.astraeus.tools.ABICodeGenerator \
  engine/api/abi_structs_schema.yaml \
  .
```

### Step 3: Verify Generated Files

Check that the generated files look correct:

```bash
# View generated C++ header
cat engine/api/EngineABI_Structs.gen.h

# View generated Java layouts
cat java/src/main/java/com/astraeus/native_api/StructLayouts.gen.java
```

### Step 4: Use in Your Code

**C++ Usage:**
```cpp
#include "EngineAPI.h"  // Already includes EngineABI_Structs.gen.h

MyNewStruct s;
s.field1 = 42;
s.field2 = 3.14;
```

**Java Usage:**
```java
import com.astraeus.native_api.StructLayouts;

// Allocate memory for struct
MemorySegment segment = arena.allocate(StructLayouts.MYNEWSTRUCT_LAYOUT);

// Set field values using VarHandles
StructLayouts.MYNEWSTRUCT_FIELD1.set(segment, 0L, 42);
StructLayouts.MYNEWSTRUCT_FIELD2.set(segment, 0L, 3.14);
```

## How to Modify an Existing Struct

### Step 1: Update Schema

Edit the struct definition in `engine/api/abi_structs_schema.yaml`:

```yaml
  - name: ExistingStruct
    description: "Updated description"
    fields:
      - name: existing_field
        type: uint32
        description: "Unchanged"
      - name: new_field      # ADD NEW FIELD
        type: float32
        description: "New field"
      - name: _padding       # UPDATE PADDING
        type: uint8
        array_size: 4        # May need to adjust size
        description: "Padding for alignment"
```

### Step 2: Regenerate and Verify

```bash
mvn generate-sources
./verify_abi_codegen.sh
```

### Step 3: Update Code

Update any C++ and Java code that uses the modified struct.

**IMPORTANT:** Struct modifications are **ABI-breaking changes**. Consider:
- Incrementing the schema version
- Adding migration code if needed
- Testing with all existing data

## Padding and Alignment

### Why Padding Matters

C structs follow platform-specific alignment rules. Without proper padding:
- Struct size may differ between C++ and Java
- Memory corruption can occur
- Undefined behavior on some platforms

### Calculating Padding

1. Calculate struct size without padding
2. Find largest field alignment (usually 8 bytes for pointers/longs)
3. Pad to next multiple of that alignment

**Example:**
```
Fields:
- uint64_t (8 bytes)
- double (8 bytes)
- uint32_t (4 bytes)
- uint32_t (4 bytes)
Total: 24 bytes (already aligned to 8)
No padding needed!

Fields:
- uint64_t (8 bytes)
- double (8 bytes)
- uint32_t (4 bytes)
Total: 20 bytes
Needs 4 bytes padding to align to 24 (next multiple of 8)
```

## Versioning

The schema includes versioning to track changes:

```yaml
version: "1.0.0"  # Schema version
```

Generated files include:
- Schema version
- Generation timestamp
- SHA-256 hash of schema file

This enables **out-of-date detection** at build time.

## Build Integration

### Maven

Code generation runs automatically during the `generate-sources` phase:

```bash
mvn generate-sources   # Regenerate code
mvn clean compile      # Clean build with regeneration
```

### CMake

The C++ build includes the generated header automatically:

```cpp
#include "EngineAPI.h"  // Includes EngineABI_Structs.gen.h
```

### Out-of-Date Detection

Run the verification script to check if generated files are current:

```bash
./verify_abi_codegen.sh
```

This compares the schema file hash with the hash embedded in generated files.

## Best Practices

### DO:
- ✅ Always add structs through the schema
- ✅ Include proper padding for alignment
- ✅ Add descriptive comments
- ✅ Run `verify_abi_codegen.sh` before committing
- ✅ Increment schema version for breaking changes
- ✅ Test on all target platforms

### DON'T:
- ❌ Manually edit generated files
- ❌ Forget padding fields
- ❌ Change field order without understanding alignment
- ❌ Mix signed/unsigned without documentation
- ❌ Use platform-specific types (use sized types)

## Troubleshooting

### Generated Files Out of Date

**Problem:** Verification script reports hash mismatch

**Solution:**
```bash
mvn generate-sources
```

### Struct Size Mismatch

**Problem:** C++ static_assert fails

**Solution:**
1. Check padding in schema
2. Verify field order
3. Check for platform-specific alignment issues

### Java Layout Error

**Problem:** Java code fails with `IllegalArgumentException`

**Solution:**
1. Ensure schema types are correct
2. Check array sizes
3. Verify padding matches C++ struct

### Code Generator Fails

**Problem:** `ABICodeGenerator` throws exception

**Solution:**
1. Validate YAML syntax
2. Check field names are valid identifiers
3. Ensure types are supported
4. Check for proper indentation

## Advanced Topics

### Custom Alignment

For structs requiring specific alignment, add explicit padding:

```yaml
fields:
  - name: data
    type: pointer
    description: "Must be 16-byte aligned"
  - name: _align_padding
    type: uint8
    array_size: 8
    description: "Force 16-byte alignment"
```

### Array Fields

Arrays are supported for padding and fixed-size data:

```yaml
  - name: matrix
    type: float32
    array_size: 16
    description: "4x4 matrix"
```

### Platform-Specific Considerations

The current implementation assumes:
- x64 architecture (8-byte pointers)
- Standard C alignment rules
- Little-endian byte order

For other platforms, additional testing is required.

## References

- [Java FFM API Documentation](https://docs.oracle.com/en/java/javase/21/docs/api/java.base/java/lang/foreign/package-summary.html)
- [C Struct Alignment Rules](https://en.cppreference.com/w/c/language/object#Alignment)
- Project: `ARCHITECTURE.md` for overall design

## Support

For questions or issues:
1. Check this guide
2. Review existing struct definitions in schema
3. Run verification script
4. Check build logs for specific errors
