# D2 Data Ingestion Quick Reference

## API Overview

```c
// Ingest data (returns job ID)
uint64_t job_id = astraeus_ingest_data(
    engine,
    data,
    size,
    ASTRAEUS_FORMAT_FIXED_BINARY
);

// Poll status
IngestStatus status;
bool found = astraeus_get_ingest_status(engine, job_id, &status);
if (found && status.is_complete) {
    if (status.has_error) {
        printf("Error: %s\n", status.last_error);
    } else {
        printf("Success!\n");
    }
}

// Get statistics
double sim_time = astraeus_get_sim_time(engine);
uint64_t count = astraeus_get_snapshot_count(engine);
```

## Sample Files

| File | Location | Entities | Size |
|------|----------|----------|------|
| Quick test | `assets/sample_data_100.bin` | 100 | 20 KB |
| Medium load | `assets/sample_data_1000.bin` | 1,000 | 200 KB |
| Stress test | `assets/sample_data_10000.bin` | 10,000 | 2 MB |

## FixedBinary Format

```
Total Size = 28 + (N × 201) bytes

where N = entity count

Header:     28 bytes
Entity:     73 bytes each
Metadata:  128 bytes each
```

## Usage Example

```c
// 1. Load file
FILE* f = fopen("assets/sample_data_100.bin", "rb");
fseek(f, 0, SEEK_END);
size_t size = ftell(f);
fseek(f, 0, SEEK_SET);
void* data = malloc(size);
fread(data, 1, size, f);
fclose(f);

// 2. Ingest
uint64_t job = astraeus_ingest_data(engine, data, size, 0);

// 3. Poll until complete
IngestStatus s;
while (astraeus_get_ingest_status(engine, job, &s) && !s.is_complete) {
    // Wait
}

// 4. Render
astraeus_begin_frame(engine, 0.016);  // Applies snapshot
astraeus_end_frame(engine);

// 5. Cleanup
free(data);
```

## Performance

- 100 entities: <0.1 ms
- 1K entities: <0.5 ms  
- 10K entities: <3 ms
- Throughput: 200-700 MB/s

## Thread Safety

- ✅ `astraeus_ingest_data()` - Thread-safe
- ✅ `astraeus_get_ingest_status()` - Thread-safe
- ✅ Snapshot application - Main thread (automatic)

## Documentation

- **Format Spec**: `docs/DATA_INGEST_FORMAT.md`
- **Sample Guide**: `assets/README.md`
- **Example Code**: `engine/examples/simple_ingest_demo.cpp`
- **Completion Report**: `TASK_D2_COMPLETION.md`
- **Integration**: `docs/INTEGRATION_MATRIX.md`

## Generate Custom Samples

```bash
cd assets
./sample_data_generator

# Or compile
g++ -std=c++17 -o sample_data_generator sample_data_generator.cpp
./sample_data_generator
```

## Troubleshooting

**No entities visible?**
- Check `astraeus_get_snapshot_count() > 0`
- Verify camera position
- Ensure `begin_frame()` / `end_frame()` called

**Ingestion fails?**
- Verify format ID is 0 (FIXED_BINARY)
- Check file magic: `0x41535430` ("AST0")
- Validate file size matches header

**Build errors?**
- Requires C++17
- Sample generator: `g++ -std=c++17 ...`
- Engine: Requires OpenGL dev libraries

## Next Steps

**For Java Integration (J6):**
1. Add FFM bindings for 4 new functions
2. Create Java wrapper methods in NativeEngine
3. Test with sample files

**For UI Integration (J7):**
1. Create IngestPane with file picker
2. Add progress bar for job status
3. Display error messages
4. Integrate with AstraeusApp

---

**Status**: D2 Complete ✅  
**See**: `TASK_D2_COMPLETION.md` for full details
