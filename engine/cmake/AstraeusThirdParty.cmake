include_guard(GLOBAL)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# ---- GLAD ----
add_library(glad STATIC
        ${ASTRAEUS_ROOT_DIR}/third_party/glad/src/glad.c
)
set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(glad PUBLIC ${ASTRAEUS_ROOT_DIR}/third_party/glad/include)

if(WIN32)
    # Add glad_wgl if present
    set(GLAD_WGL_SRC "")
    if (EXISTS ${ASTRAEUS_ROOT_DIR}/third_party/glad_wgl/src/glad_wgl.c)
        set(GLAD_WGL_SRC ${ASTRAEUS_ROOT_DIR}/third_party/glad_wgl/src/glad_wgl.c)
    elseif(EXISTS ${ASTRAEUS_ROOT_DIR}/third_party/glad_wgl/src/wgl.c)
        set(GLAD_WGL_SRC ${ASTRAEUS_ROOT_DIR}/third_party/glad_wgl/src/wgl.c)
    endif()

    if(GLAD_WGL_SRC)
        target_sources(glad PRIVATE ${GLAD_WGL_SRC})
        target_include_directories(glad PUBLIC ${ASTRAEUS_ROOT_DIR}/third_party/glad_wgl/include)
    endif()

    # glad/wgl ultimately depends on opengl32
    target_link_libraries(glad PUBLIC opengl32)
endif()

# ---- stb headers ----
add_library(stb_headers INTERFACE)
target_include_directories(stb_headers INTERFACE ${ASTRAEUS_ROOT_DIR}/third_party/stb)

# ---- tinygltf (COMPILED STATIC LIB) ----
# This is the robust fix: compile tinygltf_impl.cpp exactly once.
add_library(tinygltf STATIC
        ${ASTRAEUS_ROOT_DIR}/third_party/tinygltf/tinygltf_impl.cpp
)
set_target_properties(tinygltf PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(tinygltf
        PUBLIC
        ${ASTRAEUS_ROOT_DIR}/third_party/tinygltf
        ${ASTRAEUS_ROOT_DIR}/third_party/stb
)

# If you want to suppress the "missing-field-initializers" warning:
# - GCC/Clang: -Wno-missing-field-initializers
# - MSVC: no direct equivalent needed; often ignore or use /wdXXXX if you know the warning code.
target_compile_options(tinygltf PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wno-missing-field-initializers>
)

# ---- EGL (Linux/unix via pkg-config) ----
if(ASTRAEUS_ENABLE_EGL AND NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(EGL REQUIRED egl)

    add_library(astraeus_egl INTERFACE)
    target_include_directories(astraeus_egl INTERFACE ${EGL_INCLUDE_DIRS})
    target_link_libraries(astraeus_egl INTERFACE ${EGL_LIBRARIES})
endif()
