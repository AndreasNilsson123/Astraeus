# Data Ingestion Format Specification

## Overview

The Astraeus engine supports multiple data formats for ingesting external simulation and physics data. This document describes the supported formats, their structure, and usage patterns.

## Format IDs

| Format ID | Name | Status | Description |
|-----------|------|--------|-------------|
| 0 | FixedBinary | ✅ Implemented | Fixed-layout binary format (default) |
| 1 | JSON | 🟡 Planned | Human-readable JSON format |
| 255 | Custom | 🟢 Extensible | Custom format with registered decoder |

## FixedBinary Format (v1)

### Format Overview

The FixedBinary format is a deterministic, high-performance binary format designed for efficient entity snapshot transfer. It uses a fixed memory layout with no dynamic allocations.

**Key Features:**
- Deterministic layout (same input = same output)
- Zero-copy decoding possible
- Suitable for high-frequency updates (10+ Hz)
- Supports up to 100,000 entities per snapshot
- Thread-safe ingestion

### File Structure

```
┌──────────────────────────────────┐
│  Header (28 bytes)               │
├──────────────────────────────────┤
│  Entity Data Array               │
│  (entity_count × EntityData)     │
├──────────────────────────────────┤
│  Metadata Array                  │
│  (metadata_count × MetadataData) │
└──────────────────────────────────┘
```

### Header Structure (28 bytes)

```c
#pragma pack(push, 1)
struct Header {
    uint32_t magic;        // Magic number: 0x41535430 ("AST0")
    uint32_t version;      // Format version: 1
    double timestamp;      // Simulation timestamp (seconds)
    uint64_t frame_number; // Frame number
    uint32_t entity_count; // Number of entities
    uint32_t metadata_count; // Number of metadata entries
};
#pragma pack(pop)
```

**Field Descriptions:**
- **magic**: Must be `0x41535430` ("AST0" in ASCII)
- **version**: Format version, currently `1`
- **timestamp**: Simulation time in seconds (double precision)
- **frame_number**: Sequential frame counter
- **entity_count**: Number of entity records following the header
- **metadata_count**: Number of metadata records (usually equal to entity_count)

### EntityData Structure (73 bytes)

```c
#pragma pack(push, 1)
struct EntityData {
    uint32_t entity_id;    // Entity identifier (1-based)
    float pos_x, pos_y, pos_z;     // Position (world space)
    float rot_x, rot_y, rot_z;     // Rotation (Euler angles, radians)
    float scale_x, scale_y, scale_z; // Scale
    float color_r, color_g, color_b, color_a; // RGBA color [0-1]
    uint32_t metadata_index; // Index into metadata array
    uint8_t active;        // 1 = active, 0 = inactive
};
#pragma pack(pop)
```

**Field Descriptions:**
- **entity_id**: Unique entity identifier (must be > 0)
- **pos_x, pos_y, pos_z**: Position in world space (meters)
- **rot_x, rot_y, rot_z**: Euler angles in radians (order: XYZ)
- **scale_x, scale_y, scale_z**: Non-uniform scale factors
- **color_r, color_g, color_b, color_a**: RGBA color components (0.0 to 1.0)
- **metadata_index**: Zero-based index into metadata array
- **active**: Boolean flag (0 = inactive/deleted, 1 = active)

### MetadataData Structure (128 bytes)

```c
#pragma pack(push, 1)
struct MetadataData {
    char name[64];  // Entity name (null-terminated)
    char team[32];  // Team/group identifier
    char type[32];  // Entity type/class
};
#pragma pack(pop)
```

**Field Descriptions:**
- **name**: Human-readable entity name (max 63 characters + null terminator)
- **team**: Team or group identifier (max 31 characters + null terminator)
- **type**: Entity type or class name (max 31 characters + null terminator)

### Size Calculation

For a snapshot with N entities:
```
Total Size = sizeof(Header) + (N × sizeof(EntityData)) + (N × sizeof(MetadataData))
           = 28 + (N × 73) + (N × 128)
           = 28 + (N × 201)
```

**Examples:**
- 100 entities: 28 + 20,100 = 20,128 bytes (~20 KB)
- 1,000 entities: 28 + 201,000 = 201,028 bytes (~200 KB)
- 10,000 entities: 28 + 2,010,000 = 2,010,028 bytes (~2 MB)

## Usage Examples

### C API

```c
#include "api/EngineAPI.h"
#include <stdio.h>
#include <stdlib.h>

// Load binary file
FILE* file = fopen("sample_data_1000.bin", "rb");
fseek(file, 0, SEEK_END);
size_t file_size = ftell(file);
fseek(file, 0, SEEK_SET);

void* data = malloc(file_size);
fread(data, 1, file_size, file);
fclose(file);

// Ingest data
uint64_t job_id = astraeus_ingest_data(engine, data, file_size, ASTRAEUS_FORMAT_FIXED_BINARY);
if (job_id == 0) {
    fprintf(stderr, "Failed to ingest data\n");
    free(data);
    return;
}

// Poll for completion
IngestStatus status;
while (true) {
    if (astraeus_get_ingest_status(engine, job_id, &status)) {
        printf("Progress: %u / %u bytes\n", status.processed_bytes, status.total_bytes);
        
        if (status.is_complete) {
            if (status.has_error) {
                fprintf(stderr, "Error: %s\n", status.last_error);
            } else {
                printf("Ingestion complete!\n");
            }
            break;
        }
    }
    
    // Sleep to avoid busy-wait
    usleep(10000); // 10ms
}

free(data);
```

### Java (FFM)

```java
// Read binary file
Path path = Paths.get("sample_data_1000.bin");
byte[] data = Files.readAllBytes(path);

// Allocate native memory
try (Arena arena = Arena.ofConfined()) {
    MemorySegment dataSegment = arena.allocateArray(ValueLayout.JAVA_BYTE, data.length);
    dataSegment.copyFrom(MemorySegment.ofArray(data));
    
    // Ingest data
    long jobId = EngineAPI.astraeus_ingest_data(
        engineHandle,
        dataSegment,
        data.length,
        EngineAPI.ASTRAEUS_FORMAT_FIXED_BINARY
    );
    
    if (jobId == 0) {
        System.err.println("Failed to ingest data");
        return;
    }
    
    // Poll for completion
    MemorySegment statusSegment = arena.allocate(IngestStatus.LAYOUT);
    while (true) {
        boolean found = EngineAPI.astraeus_get_ingest_status(
            engineHandle,
            jobId,
            statusSegment
        );
        
        if (found) {
            int processed = statusSegment.get(ValueLayout.JAVA_INT, 12); // processed_bytes offset
            int total = statusSegment.get(ValueLayout.JAVA_INT, 8); // total_bytes offset
            boolean complete = statusSegment.get(ValueLayout.JAVA_BYTE, 16) != 0; // is_complete offset
            
            System.out.printf("Progress: %d / %d bytes\n", processed, total);
            
            if (complete) {
                boolean hasError = statusSegment.get(ValueLayout.JAVA_BYTE, 17) != 0;
                if (hasError) {
                    String error = statusSegment.getString(20); // last_error offset
                    System.err.println("Error: " + error);
                } else {
                    System.out.println("Ingestion complete!");
                }
                break;
            }
        }
        
        Thread.sleep(10); // 10ms
    }
}
```

## Threading Model

### Thread Safety

- **astraeus_ingest_data()**: Thread-safe, can be called from any thread
- **astraeus_get_ingest_status()**: Thread-safe, can be called from any thread
- **Snapshot Application**: Happens automatically during `astraeus_begin_frame()` on the main thread

### Lifetime Rules

1. **Input Data**: Copied internally during `astraeus_ingest_data()`. Caller may free immediately after return.
2. **Job Status**: Valid until engine shutdown. Job IDs are never reused.
3. **Snapshots**: Double-buffered. Latest snapshot always available, even during ingest.

### Concurrency Model

```
┌─────────────────┐
│  Ingest Thread  │  (optional, can be any thread)
└────────┬────────┘
         │ astraeus_ingest_data()
         ▼
    ┌──────────────┐
    │   Decoder    │
    └──────┬───────┘
           │
           ▼
    ┌──────────────┐
    │ SnapshotStore│  (double-buffered, thread-safe)
    │  Write Buffer│
    └──────────────┘
           │
           │ Atomic swap
           ▼
    ┌──────────────┐
    │ SnapshotStore│
    │  Read Buffer │
    └──────┬───────┘
           │
           ▼
┌─────────────────┐
│  Render Thread  │  (main thread)
│ begin_frame()   │
│   ↓             │
│ WorldSync       │  (applies snapshot to World)
└─────────────────┘
```

## Performance Characteristics

### Ingestion Performance

| Entity Count | Data Size | Decode Time | Throughput |
|--------------|-----------|-------------|------------|
| 100 | ~20 KB | <0.1 ms | 200+ MB/s |
| 1,000 | ~200 KB | <0.5 ms | 400+ MB/s |
| 10,000 | ~2 MB | <3 ms | 700+ MB/s |
| 100,000 | ~20 MB | <30 ms | 700+ MB/s |

**Notes:**
- Measured on Intel Core i7-9700K @ 3.60GHz
- Single-threaded decode
- Times include snapshot store write and buffer swap

### Memory Usage

| Component | Memory per Entity | Total (10K entities) |
|-----------|-------------------|----------------------|
| EntitySnapshot | 68 bytes | 680 KB |
| EntityMetadata | 128 bytes | 1.28 MB |
| Double Buffer | 2× | 3.92 MB total |

## Error Handling

### Validation

The decoder validates:
1. **Magic number**: Must be `0x41535430`
2. **Version**: Must be `1`
3. **Size**: File size must match: `Header + (entity_count × EntityData) + (metadata_count × MetadataData)`
4. **Entity IDs**: Must be non-zero
5. **Metadata indices**: Must be < metadata_count

### Error Codes

Errors are reported via `IngestStatus.has_error` and `IngestStatus.last_error`:

| Error | Description |
|-------|-------------|
| "No decoder registered for format X" | Format ID not registered |
| "Decoder failed to process data" | Validation or decode error |
| "Invalid magic number" | Not a FixedBinary file |
| "Unsupported version" | Version mismatch |
| "Invalid size" | File size doesn't match header |

## Best Practices

1. **Validate Before Ingest**: Check file magic and version before ingesting
2. **Batch Updates**: Ingest complete snapshots, not individual entities
3. **Target Rate**: Keep ingest rate at 10-30 Hz for smooth updates
4. **Entity IDs**: Use stable IDs across frames for trail rendering
5. **Metadata**: Keep metadata strings short for cache efficiency
6. **Error Handling**: Always check `IngestStatus.has_error` after completion

## Future Formats

### JSON Format (Planned)

Human-readable JSON format for debugging and tool integration:

```json
{
  "timestamp": 1.5,
  "frame_number": 15,
  "entities": [
    {
      "id": 1,
      "position": [10.0, 5.0, -3.5],
      "rotation": [0.0, 0.0, 0.0],
      "scale": [1.0, 1.0, 1.0],
      "color": [1.0, 0.5, 0.3, 1.0],
      "active": true,
      "metadata": {
        "name": "Entity_0001",
        "team": "Team_1",
        "type": "Particle"
      }
    }
  ]
}
```

### Custom Formats

Register custom decoders in C++:

```cpp
class MyCustomDecoder : public astraeus::Decoder {
public:
    bool decode(const void* data, uint32_t size, 
                SnapshotStore* store, TimeSync* time_sync) override {
        // Custom decode logic
    }
    
    const char* get_name() const override { return "MyCustomFormat"; }
    bool validate(const void* data, uint32_t size) const override { /* ... */ }
};

// Register during initialization
schema_registry->register_schema(
    255,  // Custom format ID
    "MyCustomFormat",
    "1.0",
    custom_entity_size,
    std::make_shared<MyCustomDecoder>()
);
```

## See Also

- [ARCHITECTURE.md](../ARCHITECTURE.md) - Overall engine architecture
- [INTEGRATION_MATRIX.md](../INTEGRATION_MATRIX.md) - C API to Java bindings mapping
- [sample_data_generator.cpp](../../assets/sample_data_generator.cpp) - Sample file generator
- [ingest_demo.cpp](../../engine/examples/ingest_demo.cpp) - Complete ingestion demo
