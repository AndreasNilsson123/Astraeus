# Sample Data Files

This directory contains sample simulation data files in **FixedBinary** format for testing the Astraeus data ingestion pipeline.

## Files

### Single Snapshots

| File | Entities | Size | Description |
|------|----------|------|-------------|
| `sample_data_100.bin` | 100 | ~20 KB | Small test file |
| `sample_data_1000.bin` | 1,000 | ~200 KB | Medium test file |
| `sample_data_10000.bin` | 10,000 | ~2 MB | Large test file |

### Time Series

| Files | Frames | Entities/Frame | Description |
|-------|--------|----------------|-------------|
| `sample_data_frame_*.bin` | 10 | 100 | Time series at 10 Hz |

## Format

All files use the **FixedBinary v1** format. See [DATA_INGEST_FORMAT.md](../docs/DATA_INGEST_FORMAT.md) for complete specification.

### Quick Format Overview

```
Header (28 bytes):
  - magic: 0x41535430 ("AST0")
  - version: 1
  - timestamp: double
  - frame_number: uint64_t
  - entity_count: uint32_t
  - metadata_count: uint32_t

Entity Data (73 bytes each):
  - entity_id, position, rotation, scale
  - color (RGBA), metadata_index, active flag

Metadata (128 bytes each):
  - name (64 chars), team (32 chars), type (32 chars)
```

## Usage Examples

### C API

```c
#include "api/EngineAPI.h"

// Load file
FILE* file = fopen("assets/sample_data_100.bin", "rb");
// ... read file into buffer ...

// Ingest
uint64_t job_id = astraeus_ingest_data(
    engine,
    buffer,
    file_size,
    ASTRAEUS_FORMAT_FIXED_BINARY
);

// Poll status
IngestStatus status;
astraeus_get_ingest_status(engine, job_id, &status);
```

### Java (FFM)

```java
Path path = Paths.get("assets/sample_data_100.bin");
byte[] data = Files.readAllBytes(path);

try (Arena arena = Arena.ofConfined()) {
    MemorySegment segment = arena.allocateArray(ValueLayout.JAVA_BYTE, data.length);
    segment.copyFrom(MemorySegment.ofArray(data));
    
    long jobId = EngineAPI.astraeus_ingest_data(
        engineHandle,
        segment,
        data.length,
        EngineAPI.ASTRAEUS_FORMAT_FIXED_BINARY
    );
}
```

## Generating New Sample Data

Use the included generator:

```bash
cd assets
./sample_data_generator

# Or compile from source
g++ -std=c++17 -o sample_data_generator sample_data_generator.cpp
./sample_data_generator
```

### Custom Generation

Modify `sample_data_generator.cpp` to create custom patterns:

```cpp
// Change entity count
generate_sample_file("my_data.bin", 5000, 0.0);

// Create time series
for (int frame = 0; frame < 100; frame++) {
    char filename[64];
    snprintf(filename, 64, "my_series_%03d.bin", frame);
    generate_sample_file(filename, 1000, frame * 0.1);
}
```

## Entity Layout in Samples

The generated samples create entities in circular patterns:

- **Position**: Entities arranged in concentric circles
- **Height**: Layered in 5 height levels (-4 to +4)
- **Color**: Based on angular position (rainbow pattern)
- **Motion**: If multiple frames, entities rotate around origin

### Example Visualization

```
Top View (sample_data_100.bin):
         
    o   o   o
  o   o   o   o
 o   o   ●   o   o   ← 100 entities in circular pattern
  o   o   o   o
    o   o   o

Side View:
 
 ─  ─  ─  ─  ← Layer 4 (+4 units)
 ─  ─  ─  ─  ← Layer 3 (+2 units)
 ─  ─  ●  ─  ← Layer 2 (0 units)
 ─  ─  ─  ─  ← Layer 1 (-2 units)
 ─  ─  ─  ─  ← Layer 0 (-4 units)
```

## Validation

Check if a file is valid:

```bash
# Check magic number
xxd -l 4 -g 4 sample_data_100.bin
# Output should be: 30540041  (little-endian for 0x41535430)

# Check header
xxd -l 28 sample_data_100.bin
```

## Performance

Ingestion performance on reference hardware (Intel Core i7-9700K):

| File | Size | Decode Time | Throughput |
|------|------|-------------|------------|
| sample_data_100.bin | 20 KB | <0.1 ms | 200+ MB/s |
| sample_data_1000.bin | 200 KB | <0.5 ms | 400+ MB/s |
| sample_data_10000.bin | 2 MB | <3 ms | 700+ MB/s |

## Troubleshooting

### File Not Loading

**Check magic number:**
```c
uint32_t magic;
fread(&magic, 4, 1, file);
if (magic != 0x41535430) {
    printf("Invalid file format\n");
}
```

**Check version:**
```c
uint32_t version;
fread(&version, 4, 1, file);
if (version != 1) {
    printf("Unsupported version\n");
}
```

### Ingestion Fails

**Common issues:**
1. Wrong format ID (use `ASTRAEUS_FORMAT_FIXED_BINARY = 0`)
2. Corrupted file (regenerate with sample_data_generator)
3. Engine not initialized (call `astraeus_create_engine()` first)
4. Null data pointer (check file read was successful)

### No Entities Visible

**Check:**
1. Entities were ingested (`astraeus_get_snapshot_count() > 0`)
2. Frame is being rendered (`astraeus_begin_frame()` / `astraeus_end_frame()`)
3. Camera is positioned correctly
4. Entities are active (`active = 1` in file)

## See Also

- [DATA_INGEST_FORMAT.md](../docs/DATA_INGEST_FORMAT.md) - Complete format specification
- [simple_ingest_demo.cpp](../engine/examples/simple_ingest_demo.cpp) - Minimal example
- [ingest_demo.cpp](../engine/examples/ingest_demo.cpp) - Full pipeline demo
- [INTEGRATION_MATRIX.md](../docs/INTEGRATION_MATRIX.md) - API mapping
