cmake_minimum_required(VERSION 3.15)
project(Astraeus VERSION 0.1.0 LANGUAGES C CXX)

# ==================================================
# Global configuration
# ==================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output layout
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ==================================================
# Build options
# ==================================================

option(ASTRAEUS_BUILD_SHARED     "Build shared library (DLL/.so)" ON)
option(ASTRAEUS_BUILD_EXAMPLES   "Build example programs"        ON)
option(ASTRAEUS_BUILD_TESTS      "Build tests"                   OFF)
option(ASTRAEUS_ENABLE_VALIDATION "Enable validation layers"     ON)

# Backend selection (sane defaults)
if(WIN32)
    option(ASTRAEUS_ENABLE_WGL "Enable WGL backend" ON)
    option(ASTRAEUS_ENABLE_EGL "Enable EGL backend (ANGLE)" OFF)
else()
    option(ASTRAEUS_ENABLE_WGL "Enable WGL backend" OFF)
    option(ASTRAEUS_ENABLE_EGL "Enable EGL backend" ON)
endif()

# ==================================================
# Engine sources
# ==================================================

set(ASTRAEUS_ENGINE_SOURCES
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

if(ASTRAEUS_ENABLE_WGL)
    list(APPEND ASTRAEUS_ENGINE_SOURCES
            engine/renderer/backend/wgl/WGLContext.cpp
    )
endif()

if(ASTRAEUS_ENABLE_EGL)
    list(APPEND ASTRAEUS_ENGINE_SOURCES
            engine/renderer/backend/egl/EGLContext.cpp
    )
endif()

# ==================================================
# Engine target
# ==================================================

if(ASTRAEUS_BUILD_SHARED)
    add_library(astraeus_engine SHARED ${ASTRAEUS_ENGINE_SOURCES})
    target_compile_definitions(astraeus_engine PRIVATE ASTRAEUS_BUILDING_DLL)
else()
    add_library(astraeus_engine STATIC ${ASTRAEUS_ENGINE_SOURCES})
endif()

set_target_properties(astraeus_engine PROPERTIES
        OUTPUT_NAME astraeus
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(astraeus_engine
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine>
        $<INSTALL_INTERFACE:include>
)

# ==================================================
# Dependencies
# ==================================================

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(astraeus_engine
        PRIVATE
        OpenGL::GL
        Threads::Threads
)

if(WIN32)
    # GL loader + window system (via vcpkg)
    find_package(glad CONFIG REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)

    target_link_libraries(astraeus_engine
            PRIVATE
            glad::glad
            glfw
            gdi32
            user32
    )
else()
    # Linux EGL path
    if(ASTRAEUS_ENABLE_EGL)
        find_package(EGL REQUIRED)
        target_link_libraries(astraeus_engine PRIVATE EGL::EGL)
    endif()
endif()

# ==================================================
# Warnings
# ==================================================

if(MSVC)
    target_compile_options(astraeus_engine PRIVATE /W4)
else()
    target_compile_options(astraeus_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ==================================================
# Installation
# ==================================================

install(TARGETS astraeus_engine
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

install(FILES engine/api/EngineAPI.h
        DESTINATION include/astraeus
)

# ==================================================
# Examples
# ==================================================

if(ASTRAEUS_BUILD_EXAMPLES)

    add_executable(simple_example examples/simple_example.c)
    target_link_libraries(simple_example PRIVATE astraeus_engine)
    set_target_properties(simple_example PROPERTIES LINKER_LANGUAGE CXX)

    add_executable(entity_visualization_test examples/entity_visualization_test.c)
    target_link_libraries(entity_visualization_test PRIVATE astraeus_engine)
    set_target_properties(entity_visualization_test PROPERTIES LINKER_LANGUAGE CXX)

    add_executable(ingest_demo examples/ingest_demo.cpp ${ASTRAEUS_ENGINE_SOURCES}) # Include engine sources directly due to symbol linkage errors on windows
    target_include_directories(ingest_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/engine)
    target_link_libraries(ingest_demo PRIVATE Threads::Threads OpenGL::GL)
    if(WIN32)
        target_link_libraries(ingest_demo PRIVATE glad::glad glfw gdi32 user32)
    endif()

endif()

# ==================================================
# Summary
# ==================================================

message(STATUS "--------------------------------------------")
message(STATUS "Astraeus ${PROJECT_VERSION}")
message(STATUS "  Shared library : ${ASTRAEUS_BUILD_SHARED}")
message(STATUS "  Examples       : ${ASTRAEUS_BUILD_EXAMPLES}")
message(STATUS "  Validation     : ${ASTRAEUS_ENABLE_VALIDATION}")
message(STATUS "  WGL backend    : ${ASTRAEUS_ENABLE_WGL}")
message(STATUS "  EGL backend    : ${ASTRAEUS_ENABLE_EGL}")
message(STATUS "--------------------------------------------")
