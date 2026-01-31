// Example: Using the enhanced asset pipeline with glTF support
// This example demonstrates loading assets with the new AssetDatabase

#define ASTRAEUS_GLTF_LOADER_IMPLEMENTATION
#include <iostream>
#include <core/EngineContext.hpp>
#include <assets/AssetManager.hpp>
#include <assets/GLTFLoader.hpp>

using namespace astraeus;

int main() {
    std::cout << "=== Enhanced Asset Pipeline Example ===" << std::endl;
    
    // Initialize engine
    EngineContext::Config config;
    config.initial_width = 1280;
    config.initial_height = 720;
    config.enable_validation = false;
    config.enable_debug_output = false;
    
    EngineContext engine(config);
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }
    
    AssetManager* asset_mgr = engine.get_asset_manager();
    if (!asset_mgr) {
        std::cerr << "Asset manager not available" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Loading OBJ Asset ===" << std::endl;
    
    // Load OBJ model (legacy format)
    uint32_t cube_id = asset_mgr->load_model("examples/cube.obj");
    if (cube_id == 0) {
        std::cerr << "Failed to load cube.obj" << std::endl;
        // Continue anyway to demonstrate other features
    } else {
        std::cout << "Loaded cube.obj with ID: " << cube_id << std::endl;
        
        // Get metadata
        const AssetMetadata* meta = asset_mgr->get_asset_metadata(cube_id);
        if (meta) {
            std::cout << "  URI: " << meta->uri << std::endl;
            std::cout << "  Hash: " << meta->hash << std::endl;
            std::cout << "  Size: " << meta->size_bytes << " bytes" << std::endl;
            std::cout << "  Load time: " << meta->load_time_ms << " ms" << std::endl;
            std::cout << "  References: " << meta->reference_count << std::endl;
        }
        
        // Process upload
        asset_mgr->process_uploads();
        
        // Check if ready
        if (asset_mgr->is_ready(cube_id)) {
            std::cout << "  Asset uploaded to GPU and ready for rendering" << std::endl;
        }
    }
    
    std::cout << "\n=== Testing Cache ===" << std::endl;
    
    // Load same asset again - should return same ID
    uint32_t cube_id_2 = asset_mgr->load_model("examples/cube.obj");
    if (cube_id != 0 && cube_id == cube_id_2) {
        std::cout << "SUCCESS: Cache working - same ID returned" << std::endl;
        
        // Check reference count increased
        const AssetMetadata* meta = asset_mgr->get_asset_metadata(cube_id);
        if (meta && meta->reference_count == 2) {
            std::cout << "SUCCESS: Reference count increased to " 
                      << meta->reference_count << std::endl;
        }
    }
    
    std::cout << "\n=== Asset Statistics ===" << std::endl;
    
    std::cout << "Total assets: " << asset_mgr->get_asset_count() << std::endl;
    std::cout << "Total memory: " << asset_mgr->get_total_memory_usage() 
              << " bytes" << std::endl;
    
    std::cout << "\n=== glTF Direct Loading (No GPU) ===" << std::endl;
    
    // You can also load glTF directly without GPU upload
    // This is useful for asset inspection, conversion, etc.
    std::cout << "Note: Direct glTF loading works but requires actual glTF files" << std::endl;
    std::cout << "Example code (commented out):" << std::endl;
    std::cout << "  GLTFModel model;" << std::endl;
    std::cout << "  if (GLTFLoader::load_gltf(\"model.gltf\", model)) {" << std::endl;
    std::cout << "      // Access primitives, materials, textures" << std::endl;
    std::cout << "      for (auto& prim : model.primitives) {" << std::endl;
    std::cout << "          // Process primitive" << std::endl;
    std::cout << "      }" << std::endl;
    std::cout << "  }" << std::endl;
    
    /*
    // Uncomment if you have a glTF file to test:
    
    GLTFModel model;
    if (GLTFLoader::load_gltf("assets/samples/test.gltf", model)) {
        std::cout << "Loaded glTF model:" << std::endl;
        std::cout << "  Primitives: " << model.primitives.size() << std::endl;
        std::cout << "  Materials: " << model.materials.size() << std::endl;
        std::cout << "  Textures: " << model.textures.size() << std::endl;
        
        // Iterate primitives
        for (size_t i = 0; i < model.primitives.size(); ++i) {
            const GLTFPrimitive& prim = model.primitives[i];
            std::cout << "\nPrimitive " << i << ": " << prim.name << std::endl;
            std::cout << "  Vertices: " << prim.mesh.get_vertex_count() << std::endl;
            std::cout << "  Triangles: " << prim.mesh.get_index_count() / 3 << std::endl;
            std::cout << "  Material: " << prim.material_index << std::endl;
            
            // Show material if available
            if (prim.material_index >= 0 && 
                prim.material_index < model.materials.size()) {
                const GLTFMaterial& mat = model.materials[prim.material_index];
                std::cout << "  Material name: " << mat.name << std::endl;
                std::cout << "  Base color: (" 
                          << mat.base_color_factor[0] << ", "
                          << mat.base_color_factor[1] << ", "
                          << mat.base_color_factor[2] << ", "
                          << mat.base_color_factor[3] << ")" << std::endl;
                std::cout << "  Metallic: " << mat.metallic_factor << std::endl;
                std::cout << "  Roughness: " << mat.roughness_factor << std::endl;
            }
        }
    }
    */
    
    std::cout << "\n=== Cleanup ===" << std::endl;
    
    // Unload assets
    if (cube_id != 0) {
        asset_mgr->unload_asset(cube_id);    // ref_count: 2 -> 1
        asset_mgr->unload_asset(cube_id_2);  // ref_count: 1 -> 0, deleted
        std::cout << "Assets unloaded" << std::endl;
    }
    
    std::cout << "\nFinal statistics:" << std::endl;
    std::cout << "Total assets: " << asset_mgr->get_asset_count() << std::endl;
    std::cout << "Total memory: " << asset_mgr->get_total_memory_usage() 
              << " bytes" << std::endl;
    
    std::cout << "\n=== Example Complete ===" << std::endl;
    std::cout << "\nKey Features Demonstrated:" << std::endl;
    std::cout << "✓ Asset loading with automatic type detection" << std::endl;
    std::cout << "✓ URI + hash based caching" << std::endl;
    std::cout << "✓ Reference counting" << std::endl;
    std::cout << "✓ Metadata tracking (size, load time, references)" << std::endl;
    std::cout << "✓ Asset statistics" << std::endl;
    std::cout << "✓ Safe unloading" << std::endl;
    std::cout << "\nFor glTF-specific features, see:" << std::endl;
    std::cout << "- gltf_loader_test.cpp (unit tests)" << std::endl;
    std::cout << "- docs/GLTF_LOADER_GUIDE.md (usage guide)" << std::endl;
    
    engine.shutdown();
    return 0;
}
