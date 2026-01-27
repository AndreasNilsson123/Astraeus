cmake_minimum_required(VERSION 3.15)
project(Astraeus VERSION 0.1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(ASTRAEUS_BUILD_SHARED "Build shared library" ON)
option(ASTRAEUS_ENABLE_VALIDATION "Enable validation layers" ON)
option(ASTRAEUS_BUILD_TESTS "Build tests" OFF)
option(ASTRAEUS_BUILD_EXAMPLES "Build examples" ON)

# Platform-specific graphics backend options
if(WIN32)
    option(ASTRAEUS_ENABLE_EGL "Enable EGL backend (requires ANGLE or Mesa on Windows)" OFF)
    option(ASTRAEUS_ENABLE_WGL "Enable WGL backend for Windows" ON)
else()
    option(ASTRAEUS_ENABLE_EGL "Enable EGL backend for Linux/Unix" ON)
    option(ASTRAEUS_ENABLE_WGL "Enable WGL backend for Windows" OFF)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Source files
set(ENGINE_SOURCES
    engine/api/EngineAPI.cpp
    engine/core/EngineContext.cpp
    engine/renderer/RenderDevice.cpp
    engine/renderer/RenderGraph.cpp
    engine/renderer/opengl/GLRenderDevice.cpp
    engine/renderer/backend/GraphicsContext.cpp
    engine/renderer/backend/NullContext.cpp
    engine/renderer/passes/ClearPass.cpp
    engine/renderer/passes/GridPass.cpp
    engine/renderer/passes/AxesPass.cpp
    engine/renderer/passes/PointSpritePass.cpp
    engine/renderer/passes/TrailPass.cpp
    engine/scene/World.cpp
    engine/scene/Camera.cpp
    engine/ingest/IngestManager.cpp
    engine/ingest/SnapshotStore.cpp
    engine/ingest/TimeSync.cpp
    engine/ingest/SchemaRegistry.cpp
    engine/ingest/FixedBinaryDecoder.cpp
    engine/ingest/WorldSync.cpp
    engine/ingest/DeterministicSimGenerator.cpp
    engine/assets/AssetManager.cpp
    engine/geometry/Mesh.cpp
)

# Add backend-specific sources
if(ASTRAEUS_ENABLE_EGL)
    list(APPEND ENGINE_SOURCES engine/renderer/backend/egl/EGLContext.cpp)
endif()

if(ASTRAEUS_ENABLE_WGL)
    list(APPEND ENGINE_SOURCES engine/renderer/backend/wgl/WGLContext.cpp)
endif()

set(ENGINE_HEADERS
    engine/api/EngineAPI.h
    engine/core/EngineContext.hpp
    engine/renderer/RenderDevice.hpp
    engine/renderer/RenderGraph.hpp
    engine/renderer/opengl/GLRenderDevice.hpp
    engine/renderer/passes/ClearPass.hpp
    engine/renderer/passes/GridPass.hpp
    engine/renderer/passes/AxesPass.hpp
    engine/renderer/passes/PointSpritePass.hpp
    engine/renderer/passes/TrailPass.hpp
    engine/scene/World.hpp
    engine/scene/Camera.hpp
    engine/ingest/IngestManager.hpp
    engine/ingest/SnapshotStore.hpp
    engine/ingest/SnapshotView.h
    engine/ingest/TimeSync.hpp
    engine/ingest/SchemaRegistry.hpp
    engine/ingest/Decoder.h
    engine/ingest/DataChannel.h
    engine/ingest/FixedBinaryDecoder.hpp
    engine/ingest/WorldSync.hpp
    engine/ingest/DeterministicSimGenerator.hpp
    engine/assets/AssetManager.hpp
    engine/geometry/Mesh.hpp
)

# Create library
if(ASTRAEUS_BUILD_SHARED)
    add_library(astraeus_engine SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})
    target_compile_definitions(astraeus_engine PRIVATE ASTRAEUS_BUILD_SHARED)
else()
    add_library(astraeus_engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
endif()

# Include directories
target_include_directories(astraeus_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine>
        $<INSTALL_INTERFACE:include>
)

# Configure backend-specific compile definitions
if(ASTRAEUS_ENABLE_EGL)
    target_compile_definitions(astraeus_engine PRIVATE ASTRAEUS_ENABLE_EGL)
endif()

if(ASTRAEUS_ENABLE_WGL)
    target_compile_definitions(astraeus_engine PRIVATE ASTRAEUS_ENABLE_WGL)
endif()

# Link OpenGL library (always needed for GL calls)
find_package(OpenGL REQUIRED)
target_link_libraries(astraeus_engine PRIVATE OpenGL::GL)

# Link EGL if enabled (Linux/Unix or Windows with ANGLE)
if(ASTRAEUS_ENABLE_EGL)
    if(WIN32)
        # On Windows, EGL requires ANGLE or other EGL provider
        message(STATUS "EGL enabled on Windows - make sure ANGLE or other EGL provider is available")
        # User should set EGL library paths if needed
        find_library(EGL_LIBRARY NAMES EGL)
        if(EGL_LIBRARY)
            target_link_libraries(astraeus_engine PRIVATE ${EGL_LIBRARY})
        else()
            message(WARNING "EGL library not found - EGL backend may not work")
        endif()
    else()
        # On Linux/Unix, try to find EGL via pkg-config or find_package
        find_package(EGL)
        if(EGL_FOUND)
            target_link_libraries(astraeus_engine PRIVATE EGL::EGL)
            message(STATUS "EGL found and will be linked")
        else()
            # Fallback to direct linking
            target_link_libraries(astraeus_engine PRIVATE EGL)
            message(STATUS "Using direct EGL linkage (fallback)")
        endif()
    endif()
endif()

# Link Windows-specific libraries for WGL
if(ASTRAEUS_ENABLE_WGL AND WIN32)
    target_link_libraries(astraeus_engine PRIVATE gdi32 user32)
endif()

# Compiler flags
if(MSVC)
    target_compile_options(astraeus_engine PRIVATE /W4)
else()
    target_compile_options(astraeus_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set output name
set_target_properties(astraeus_engine PROPERTIES
    OUTPUT_NAME "astraeus"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Installation rules
install(TARGETS astraeus_engine
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES engine/api/EngineAPI.h
    DESTINATION include/astraeus
)

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build examples if requested
if(ASTRAEUS_BUILD_EXAMPLES)
    # Simple example
    add_executable(simple_example examples/simple_example.c)
    target_link_libraries(simple_example PRIVATE astraeus_engine)
    target_include_directories(simple_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(simple_example PROPERTIES LINKER_LANGUAGE CXX)
    
    # Entity visualization test
    add_executable(entity_visualization_test examples/entity_visualization_test.c)
    target_link_libraries(entity_visualization_test PRIVATE astraeus_engine)
    target_include_directories(entity_visualization_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(entity_visualization_test PROPERTIES LINKER_LANGUAGE CXX)
    
    # Render example (with PNG output)
    find_package(PNG QUIET)
    if(PNG_FOUND)
        add_executable(render_example examples/render_example.c)
        target_link_libraries(render_example PRIVATE astraeus_engine PNG::PNG)
        target_include_directories(render_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        set_target_properties(render_example PROPERTIES LINKER_LANGUAGE CXX)
        
        install(TARGETS render_example RUNTIME DESTINATION bin)
    else()
        message(STATUS "PNG library not found, skipping render_example")
    endif()
    
    # Ingest demo (Task B3)
    add_executable(ingest_demo examples/ingest_demo.cpp)
    target_link_libraries(ingest_demo PRIVATE astraeus_engine pthread)
    target_include_directories(ingest_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    
    install(TARGETS simple_example entity_visualization_test ingest_demo
        RUNTIME DESTINATION bin
    )
endif()

# Print configuration
message(STATUS "Astraeus Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared library: ${ASTRAEUS_BUILD_SHARED}")
message(STATUS "  Validation: ${ASTRAEUS_ENABLE_VALIDATION}")
message(STATUS "  Build tests: ${ASTRAEUS_BUILD_TESTS}")
message(STATUS "  Build examples: ${ASTRAEUS_BUILD_EXAMPLES}")
message(STATUS "")
message(STATUS "Graphics Backend Configuration:")
if(ASTRAEUS_ENABLE_EGL)
    message(STATUS "  ✓ EGL backend enabled (Linux/Unix or Windows with ANGLE)")
else()
    message(STATUS "  ✗ EGL backend disabled")
endif()
if(ASTRAEUS_ENABLE_WGL)
    message(STATUS "  ✓ WGL backend enabled (Windows)")
else()
    message(STATUS "  ✗ WGL backend disabled")
endif()
if(NOT ASTRAEUS_ENABLE_EGL AND NOT ASTRAEUS_ENABLE_WGL)
    message(STATUS "  ⚠ Warning: No graphics backend enabled - using null backend (no rendering)")
endif()
