include_guard(GLOBAL)

# ==================================================
# Engine sources (shared by both SHARED + STATIC variants)
# ==================================================
set(ASTRAEUS_ENGINE_SOURCES
        ${ASTRAEUS_ROOT_DIR}/api/EngineAPI_stub.cpp
        ${ASTRAEUS_ROOT_DIR}/api/EngineAPI_RenderSession.cpp
        ${ASTRAEUS_ROOT_DIR}/renderer/RenderGraph.cpp
        ${ASTRAEUS_ROOT_DIR}/renderer/passes/post/PostProcessPass.cpp
        ${ASTRAEUS_ROOT_DIR}/scene/spatial/BVH.cpp
        ${ASTRAEUS_ROOT_DIR}/scene/spatial/SpatialIndex.cpp
)

# ==================================================
# Common include dirs + deps (apply to both targets)
# ==================================================
set(ASTRAEUS_ENGINE_PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${ASTRAEUS_ROOT_DIR}>
        $<INSTALL_INTERFACE:include>
)

set(ASTRAEUS_ENGINE_PUBLIC_LIBS
        glad
        OpenGL::GL
        Threads::Threads
        stb_headers
        tinygltf
)

# ==================================================
# Primary engine target (for Java/FFM runtime)
# ==================================================
if(ASTRAEUS_BUILD_SHARED)
    add_library(astraeus_engine SHARED ${ASTRAEUS_ENGINE_SOURCES})
    target_compile_definitions(astraeus_engine PRIVATE ASTRAEUS_BUILDING_DLL)
else()
    add_library(astraeus_engine STATIC ${ASTRAEUS_ENGINE_SOURCES})
    target_compile_definitions(astraeus_engine PUBLIC ASTRAEUS_API_STATIC)
endif()

set_target_properties(astraeus_engine PROPERTIES
        OUTPUT_NAME astraeus
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(astraeus_engine PUBLIC ${ASTRAEUS_ENGINE_PUBLIC_INCLUDES})
target_link_libraries(astraeus_engine PUBLIC ${ASTRAEUS_ENGINE_PUBLIC_LIBS})

# Platform sources + libs + defs
astraeus_apply_platform(astraeus_engine)

# EGL optional
if(TARGET astraeus_egl)
    target_link_libraries(astraeus_engine PRIVATE astraeus_egl)
endif()

astraeus_apply_warnings(astraeus_engine)

# ==================================================
# Static engine variant for native examples/tests on Windows
# (Avoids requiring C++ symbol exports from the DLL import library.)
# Only built when the primary engine is SHARED.
# ==================================================
if(ASTRAEUS_BUILD_SHARED)
    add_library(astraeus_engine_static STATIC ${ASTRAEUS_ENGINE_SOURCES})
    target_compile_definitions(astraeus_engine_static PUBLIC ASTRAEUS_API_STATIC)

    set_target_properties(astraeus_engine_static PROPERTIES
            OUTPUT_NAME astraeus_static
            VERSION ${PROJECT_VERSION}
    )

    target_include_directories(astraeus_engine_static PUBLIC ${ASTRAEUS_ENGINE_PUBLIC_INCLUDES})
    target_link_libraries(astraeus_engine_static PUBLIC ${ASTRAEUS_ENGINE_PUBLIC_LIBS})

    # Platform sources + libs + defs
    astraeus_apply_platform(astraeus_engine_static)

    # EGL optional (keep same behavior as shared target)
    if(TARGET astraeus_egl)
        target_link_libraries(astraeus_engine_static PRIVATE astraeus_egl)
    endif()

    astraeus_apply_warnings(astraeus_engine_static)
endif()
