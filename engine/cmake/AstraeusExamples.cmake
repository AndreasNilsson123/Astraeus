include_guard(GLOBAL)

function(astraeus_add_example name source)
    add_executable(${name} ${ASTRAEUS_ROOT_DIR}/${source})

    # If we build the engine as SHARED, link native examples/tests to the STATIC variant
    # to avoid relying on exported C++ symbols from the DLL import lib on Windows.
    if(TARGET astraeus_engine_static)
        target_link_libraries(${name} PRIVATE astraeus_engine_static)
    else()
        target_link_libraries(${name} PRIVATE astraeus_engine)
    endif()

    # If source is .c but uses C++ link, force it
    get_filename_component(_ext "${source}" EXT)
    if(_ext STREQUAL ".c")
        set_target_properties(${name} PROPERTIES LINKER_LANGUAGE CXX)
    endif()
endfunction()


if(ASTRAEUS_BUILD_EXAMPLES)
    astraeus_add_example(simple_example examples/simple_example.c)
    astraeus_add_example(entity_visualization_test examples/entity_visualization_test.c)
    astraeus_add_example(pointer_stability_test examples/pointer_stability_test.c)

    astraeus_add_example(ingest_demo examples/ingest_demo.cpp)
    astraeus_add_example(asset_test examples/asset_test.cpp)
    astraeus_add_example(asset_unit_test examples/asset_unit_test.cpp)
    astraeus_add_example(gltf_loader_test examples/gltf_loader_test.cpp)
    astraeus_add_example(asset_pipeline_example examples/asset_pipeline_example.cpp)

    astraeus_add_example(scene_hierarchy_test examples/scene_hierarchy_test.cpp)
    astraeus_add_example(spatial_query_test examples/spatial_query_test.cpp)
    astraeus_add_example(material_system_test examples/material_system_test.cpp)
    astraeus_add_example(lighting_system_test examples/lighting_system_test.cpp)

    astraeus_add_example(camera_system_test examples/camera_system_test.cpp)
    astraeus_add_example(camera_component_test examples/camera_component_test.cpp)

    astraeus_add_example(mesh_rendering_test examples/mesh_rendering_test.cpp)
    
    # Compile-only test to verify RenderGraph.hpp is self-contained
    astraeus_add_example(render_graph_header_test examples/render_graph_header_test.cpp)
    
    # Fast Math tests and benchmarks
    astraeus_add_example(fastmath_test examples/fastmath_test.cpp)
    astraeus_add_example(fastmath_benchmark examples/fastmath_benchmark.cpp)
    astraeus_add_example(fastmath_usage_example examples/fastmath_usage_example.cpp)
endif()
