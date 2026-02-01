include_guard(GLOBAL)

set(ASTRAEUS_ENGINE_SOURCES
        ${ASTRAEUS_ROOT_DIR}/api/EngineAPI_stub.cpp
        ${ASTRAEUS_ROOT_DIR}/api/EngineAPI_RenderSession.cpp
        ${ASTRAEUS_ROOT_DIR}/renderer/RenderGraph.cpp
        ${ASTRAEUS_ROOT_DIR}/renderer/passes/post/PostProcessPass.cpp
        ${ASTRAEUS_ROOT_DIR}/scene/spatial/BVH.cpp
        ${ASTRAEUS_ROOT_DIR}/scene/spatial/SpatialIndex.cpp
)

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

target_include_directories(astraeus_engine
        PUBLIC
        $<BUILD_INTERFACE:${ASTRAEUS_ROOT_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Platform sources + libs + defs
astraeus_apply_platform(astraeus_engine)

# Link dependencies
target_link_libraries(astraeus_engine
        PUBLIC
        glad
        OpenGL::GL
        Threads::Threads
        stb_headers
        tinygltf
)

# EGL optional
if(TARGET astraeus_egl)
    target_link_libraries(astraeus_engine PRIVATE astraeus_egl)
endif()

astraeus_apply_warnings(astraeus_engine)
