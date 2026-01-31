cmake_minimum_required(VERSION 3.15)
project(Astraeus VERSION 0.1.0 LANGUAGES C CXX)

# ==================================================
# Global configuration
# ==================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform-specific compile definitions
if(WIN32)
    # Prevent Windows.h from defining min/max macros
    add_compile_definitions(NOMINMAX)
    # Reduce Windows.h header size
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()

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

# All engine modules are now header-only for maximum flexibility
# Note: Engine API and Core modules are now header-only (see engine/api/*.h, engine/core/*.hpp)
# Note: Renderer modules are now header-only (see engine/renderer/*.hpp)
# Note: Backend modules are now header-only (see engine/renderer/backend/*.hpp)
# Note: Render pass modules are now header-only (see engine/renderer/passes/*.hpp)
# Note: Ingest modules are now header-only (see engine/ingest/*.hpp)
# Note: Asset and geometry modules are now header-only (see engine/assets/*.hpp, engine/geometry/*.hpp)

# Platform abstraction module (compiled separately)
# Only these files may contain platform-specific code and includes
set(ASTRAEUS_PLATFORM_SOURCES
        platform/Platform.hpp
)

if(WIN32)
    list(APPEND ASTRAEUS_PLATFORM_SOURCES
            platform/Win32/Win32Platform.cpp
            platform/Win32/Win32Headers.hpp
    )
elseif(UNIX)
    list(APPEND ASTRAEUS_PLATFORM_SOURCES
            platform/Linux/LinuxPlatform.cpp
            platform/Linux/X11Headers.hpp
    )
endif()

# Minimal stub file to satisfy CMake library requirements (all implementations are inline/header-only)
set(ASTRAEUS_ENGINE_SOURCES
        api/EngineAPI_stub.cpp
        api/EngineAPI_RenderSession.cpp
        ${ASTRAEUS_PLATFORM_SOURCES}
)

# Note: WGL and EGL backend modules are now header-only
# No additional source files needed for backend compilation

# ==================================================
# Engine target
# ==================================================

if(ASTRAEUS_BUILD_SHARED)
    add_library(astraeus_engine SHARED ${ASTRAEUS_ENGINE_SOURCES})
    target_compile_definitions(astraeus_engine PRIVATE ASTRAEUS_BUILDING_DLL)
else()
    add_library(astraeus_engine STATIC ${ASTRAEUS_ENGINE_SOURCES})

    # IMPORTANT (Windows): consumers must NOT see dllimport when linking statically
    target_compile_definitions(astraeus_engine PUBLIC ASTRAEUS_API_STATIC)
endif()

set_target_properties(astraeus_engine PROPERTIES
        OUTPUT_NAME astraeus
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(astraeus_engine
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)


# ==================================================
# Dependencies
# ==================================================

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# ---- GLAD (third_party) ----
add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/src/glad.c)
set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include)

if (WIN32)
    # Support both common filenames
    set(GLAD_WGL_SRC "")
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad_wgl/src/glad_wgl.c)
        set(GLAD_WGL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad_wgl/src/glad_wgl.c)
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad_wgl/src/wgl.c)
        set(GLAD_WGL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad_wgl/src/wgl.c)
    endif()

    target_sources(glad PRIVATE ${GLAD_WGL_SRC})
    target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad_wgl/include)

    # WGL ultimately depends on opengl32 on Windows
    target_link_libraries(glad PUBLIC opengl32)
endif()


target_link_libraries(astraeus_engine
        PUBLIC
        glad
        OpenGL::GL
        Threads::Threads
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(astraeus_engine PUBLIC winmm)  # timeBeginPeriod
endif()

# Linux EGL path
if(ASTRAEUS_ENABLE_EGL)
    # Use pkg-config to find EGL
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(EGL REQUIRED egl)
    target_include_directories(astraeus_engine PRIVATE ${EGL_INCLUDE_DIRS})
    target_link_libraries(astraeus_engine PRIVATE ${EGL_LIBRARIES})
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

install(FILES api/EngineAPI.h
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

    add_executable(pointer_stability_test examples/pointer_stability_test.c)
    target_link_libraries(pointer_stability_test PRIVATE astraeus_engine)
    set_target_properties(pointer_stability_test PROPERTIES LINKER_LANGUAGE CXX)

    add_executable(ingest_demo examples/ingest_demo.cpp)
    target_link_libraries(ingest_demo PRIVATE astraeus_engine)

    add_executable(asset_test examples/asset_test.cpp)
    target_link_libraries(asset_test PRIVATE astraeus_engine)

    add_executable(asset_unit_test examples/asset_unit_test.cpp)
    target_link_libraries(asset_unit_test PRIVATE astraeus_engine)

    add_executable(gltf_loader_test examples/gltf_loader_test.cpp)
    target_link_libraries(gltf_loader_test PRIVATE astraeus_engine)

    add_executable(scene_hierarchy_test examples/scene_hierarchy_test.cpp)
    target_link_libraries(scene_hierarchy_test PRIVATE astraeus_engine)

    add_executable(material_system_test examples/material_system_test.cpp)
    target_link_libraries(material_system_test PRIVATE astraeus_engine)

    add_executable(camera_system_test examples/camera_system_test.cpp)
    target_link_libraries(camera_system_test PRIVATE astraeus_engine)

    add_executable(camera_component_test examples/camera_component_test.cpp)
    target_link_libraries(camera_component_test PRIVATE astraeus_engine)

    add_executable(mesh_rendering_test examples/mesh_rendering_test.cpp)
    target_link_libraries(mesh_rendering_test PRIVATE astraeus_engine)

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
