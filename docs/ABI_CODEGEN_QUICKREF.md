# ABI Codegen Quick Reference

## Common Commands

```bash
# Regenerate code after schema change
./regenerate_abi.sh

# Check if generated files are up-to-date
./verify_abi_codegen.sh

# View struct sizes (C++)
g++ -std=c++17 -I. /tmp/test_abi_structs.cpp -o /tmp/test && /tmp/test
```

## Adding a New Struct

1. Edit `engine/api/abi_structs_schema.yaml`:
```yaml
  - name: MyStruct
    description: "My struct description"
    fields:
      - name: field1
        type: uint32
        description: "Field description"
      - name: _padding
        type: uint8
        array_size: 4
        description: "Alignment padding"
```

2. Regenerate:
```bash
./regenerate_abi.sh
```

3. Use in C++:
```cpp
#include "EngineAPI.h"
MyStruct s;
s.field1 = 42;
```

4. Use in Java:
```java
MemorySegment seg = arena.allocate(StructLayouts.MYSTRUCT_LAYOUT);
StructLayouts.MYSTRUCT_FIELD1.set(seg, 0L, 42);
```

## Type Mapping

| Schema     | C++        | Java              |
|------------|------------|-------------------|
| `uint32`   | `uint32_t` | `JAVA_INT`       |
| `uint64`   | `uint64_t` | `JAVA_LONG`      |
| `float32`  | `float`    | `JAVA_FLOAT`     |
| `float64`  | `double`   | `JAVA_DOUBLE`    |
| `bool`     | `bool`     | `JAVA_BOOLEAN`   |
| `pointer`  | `void*`    | `ADDRESS`        |

## Padding Rules

Align to largest field:
- int/float: 4 bytes
- long/double/pointer: 8 bytes

Example:
```
3 x uint32 = 12 bytes
+ 4 bytes padding = 16 bytes (aligned to 4)
```

## Files

- **Schema:** `engine/api/abi_structs_schema.yaml`
- **C++ output:** `engine/api/EngineABI_Structs.gen.h`
- **Java output:** `java/.../StructLayouts.gen.java`
- **Docs:** `docs/ABI_CODEGEN_GUIDE.md`

## Prerequisites

- Java 17+ (for code generator)
- Java 21+ (for full project build with FFM)
