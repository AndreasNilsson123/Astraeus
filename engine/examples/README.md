# Astraeus Examples

This directory contains example programs demonstrating how to use the Astraeus engine.

## Building Examples

Examples are built automatically when you build the main project with `ASTRAEUS_BUILD_EXAMPLES=ON` (which is the default):

```bash
cd build
cmake ..
cmake --build .
```

The compiled examples will be in `build/bin/`.

## Running Examples

Make sure the library path includes the Astraeus library location:

**Linux/macOS:**
```bash
export LD_LIBRARY_PATH=build/lib:$LD_LIBRARY_PATH
./build/bin/simple_example
```

**Windows:**
```cmd
set PATH=%PATH%;build\lib
build\bin\simple_example.exe
```

## Available Examples

### simple_example.c

A basic example demonstrating:
- Engine initialization and shutdown
- Camera configuration
- Entity creation and management
- Frame rendering loop
- Picking (entity selection)
- Viewport resizing
- Entity destruction

This example shows the complete C API usage pattern.

## Creating Your Own Example

To add a new example:

1. Create your C/C++ file in the `examples/` directory
2. Add it to `CMakeLists.txt`:
   ```cmake
   add_executable(my_example examples/my_example.c)
   target_link_libraries(my_example PRIVATE astraeus_engine)
   target_include_directories(my_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
   ```
3. Rebuild the project
4. Run your example from the build directory

## API Reference

See the main [README.md](../../README.md) for API documentation, or refer to [engine/api/EngineAPI.h](../api/EngineAPI.h) for the complete C API reference.
