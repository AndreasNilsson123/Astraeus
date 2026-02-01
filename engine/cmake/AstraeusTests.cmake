include_guard(GLOBAL)

if(NOT ASTRAEUS_BUILD_TESTS)
    return()
endif()

# Enable testing
enable_testing()

# Add GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
add_subdirectory(${ASTRAEUS_ROOT_DIR}/third_party/googletest EXCLUDE_FROM_ALL)

# Common test utilities and mocks
set(ASTRAEUS_TEST_COMMON_SOURCES
    ${ASTRAEUS_ROOT_DIR}/tests/mocks/MockRenderDevice.hpp
)

# Helper function to add a test executable
function(astraeus_add_test test_name)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SOURCES LIBS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    add_executable(${test_name} ${ARG_SOURCES})
    
    # Link to the static engine variant for testing
    if(TARGET astraeus_engine_static)
        target_link_libraries(${test_name} PRIVATE astraeus_engine_static)
    else()
        target_link_libraries(${test_name} PRIVATE astraeus_engine)
    endif()
    
    # Link GoogleTest
    target_link_libraries(${test_name} PRIVATE 
        gtest 
        gtest_main 
        gmock
        ${ARG_LIBS}
    )
    
    # Add to CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # Set test properties for better output
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endfunction()

# ============================================================
# CORE MODULE TESTS
# ============================================================
astraeus_add_test(test_engine_lifecycle
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/core/test_engine_lifecycle.cpp
)

astraeus_add_test(test_telemetry
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/core/test_telemetry.cpp
)

astraeus_add_test(test_event_bus
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/core/test_event_bus.cpp
)

astraeus_add_test(test_command_buffer
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/core/test_command_buffer.cpp
)

# ============================================================
# SCENE MODULE TESTS
# ============================================================
astraeus_add_test(test_world_entities
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/scene/test_world_entities.cpp
)

astraeus_add_test(test_transform_system
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/scene/test_transform_system.cpp
)

astraeus_add_test(test_camera_system
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/scene/test_camera_system.cpp
)

astraeus_add_test(test_spatial_index
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/scene/test_spatial_index.cpp
)

# ============================================================
# INGEST MODULE TESTS
# ============================================================
astraeus_add_test(test_snapshot_store
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/ingest/test_snapshot_store.cpp
)

astraeus_add_test(test_time_sync
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/ingest/test_time_sync.cpp
)

astraeus_add_test(test_ingest_manager
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/ingest/test_ingest_manager.cpp
)

astraeus_add_test(test_decoder
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/ingest/test_decoder.cpp
)

# ============================================================
# RENDERER MODULE TESTS
# ============================================================
astraeus_add_test(test_render_graph
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/renderer/test_render_graph.cpp
)

astraeus_add_test(test_render_pass
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/renderer/test_render_pass.cpp
)

# ============================================================
# API/ABI CONTRACT TESTS
# ============================================================
astraeus_add_test(test_abi_contract
    SOURCES ${ASTRAEUS_ROOT_DIR}/tests/api/test_abi_contract.cpp
)

# ============================================================
# SANITIZER SUPPORT
# ============================================================
option(ASTRAEUS_ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(ASTRAEUS_ENABLE_UBSAN "Enable Undefined Behavior Sanitizer" OFF)
option(ASTRAEUS_ENABLE_TSAN "Enable Thread Sanitizer" OFF)

if(ASTRAEUS_ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
    message(STATUS "Address Sanitizer enabled")
endif()

if(ASTRAEUS_ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
    message(STATUS "Undefined Behavior Sanitizer enabled")
endif()

if(ASTRAEUS_ENABLE_TSAN)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
    message(STATUS "Thread Sanitizer enabled")
endif()
