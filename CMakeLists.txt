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
    engine/renderer/passes/ClearPass.cpp
    engine/renderer/passes/TrianglePass.cpp
    engine/scene/World.cpp
    engine/ingest/IngestManager.cpp
    engine/assets/AssetManager.cpp
    engine/geometry/Mesh.cpp
)

set(ENGINE_HEADERS
    engine/api/EngineAPI.h
    engine/core/EngineContext.hpp
    engine/renderer/RenderDevice.hpp
    engine/renderer/RenderGraph.hpp
    engine/renderer/opengl/GLRenderDevice.hpp
    engine/renderer/passes/ClearPass.hpp
    engine/renderer/passes/TrianglePass.hpp
    engine/scene/World.hpp
    engine/ingest/IngestManager.hpp
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

# Find and link OpenGL and EGL
find_package(OpenGL REQUIRED)
target_link_libraries(astraeus_engine PRIVATE OpenGL::GL OpenGL::OpenGL EGL)

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
    
    # Render example (with PNG output)
    find_package(PNG REQUIRED)
    add_executable(render_example examples/render_example.c)
    target_link_libraries(render_example PRIVATE astraeus_engine PNG::PNG)
    target_include_directories(render_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(render_example PROPERTIES LINKER_LANGUAGE CXX)
    
    install(TARGETS simple_example render_example
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
