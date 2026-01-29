#include <iostream>
#include <core/EngineContext.hpp>
#include <assets/AssetManager.hpp>
#include <renderer/passes/MeshPass.hpp>

using namespace astraeus;

int main() {
    std::cout << "=== Asset Loading Test ===" << std::endl;

    // Create engine context
    EngineContext::Config config;
    config.initial_width = 800;
    config.initial_height = 600;
    config.enable_validation = false;
    config.enable_debug_output = false;

    EngineContext engine(config);
    
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }

    std::cout << "\n=== Loading Assets ===" << std::endl;

    // Get asset manager
    AssetManager* asset_manager = engine.get_asset_manager();
    if (!asset_manager) {
        std::cerr << "Asset manager not available" << std::endl;
        return 1;
    }

    // Load a test cube mesh
    const char* cube_path = "examples/cube.obj";
    uint32_t cube_asset_id = asset_manager->load_model(cube_path);
    
    if (cube_asset_id == 0) {
        std::cerr << "Failed to load cube model" << std::endl;
        return 1;
    }

    std::cout << "Cube asset loaded with ID: " << cube_asset_id << std::endl;

    // Load the same cube again (should use cache)
    uint32_t cube_asset_id_2 = asset_manager->load_model(cube_path);
    std::cout << "Second load of cube: " << cube_asset_id_2 << std::endl;

    if (cube_asset_id != cube_asset_id_2) {
        std::cerr << "ERROR: Asset cache not working - different IDs returned!" << std::endl;
        return 1;
    }

    std::cout << "\n=== Processing GPU Uploads ===" << std::endl;

    // Process uploads (simulate a few frames)
    for (int i = 0; i < 5; i++) {
        asset_manager->process_uploads();
        
        if (asset_manager->is_ready(cube_asset_id)) {
            std::cout << "Asset " << cube_asset_id << " is ready for rendering" << std::endl;
            break;
        }
    }

    // Verify GPU mesh is available
    const GPUMesh* gpu_mesh = asset_manager->get_gpu_mesh(cube_asset_id);
    if (!gpu_mesh || !gpu_mesh->is_valid()) {
        std::cerr << "ERROR: GPU mesh not available or invalid" << std::endl;
        return 1;
    }

    std::cout << "GPU Mesh details:" << std::endl;
    std::cout << "  VAO: " << gpu_mesh->vao << std::endl;
    std::cout << "  VBO: " << gpu_mesh->vbo << std::endl;
    std::cout << "  IBO: " << gpu_mesh->ibo << std::endl;
    std::cout << "  Vertex count: " << gpu_mesh->vertex_count << std::endl;
    std::cout << "  Index count: " << gpu_mesh->index_count << std::endl;
    std::cout << "  Ref count: " << gpu_mesh->ref_count << std::endl;

    std::cout << "\n=== Testing Reference Counting ===" << std::endl;

    // Unload first reference
    asset_manager->unload_asset(cube_asset_id);
    
    // Should still be available (ref count = 1)
    const GPUMesh* gpu_mesh_after_unload_1 = asset_manager->get_gpu_mesh(cube_asset_id);
    if (gpu_mesh_after_unload_1 && gpu_mesh_after_unload_1->is_valid()) {
        std::cout << "Asset still available after first unload (ref_count should be 1)" << std::endl;
    } else {
        std::cerr << "ERROR: Asset should still be available after first unload!" << std::endl;
        return 1;
    }

    // Unload second reference
    asset_manager->unload_asset(cube_asset_id_2);
    
    // Should be deleted now
    const GPUMesh* gpu_mesh_after_unload_2 = asset_manager->get_gpu_mesh(cube_asset_id);
    if (gpu_mesh_after_unload_2 == nullptr) {
        std::cout << "Asset correctly deleted after all references unloaded" << std::endl;
    } else {
        std::cerr << "ERROR: Asset should be deleted after all references unloaded!" << std::endl;
        return 1;
    }

    std::cout << "\n=== Testing No Dangling Handles ===" << std::endl;
    
    // Try to use the deleted asset
    asset_manager->unload_asset(cube_asset_id); // Should not crash
    std::cout << "No crash when unloading already deleted asset - PASS" << std::endl;

    std::cout << "\n=== All Tests Passed ===" << std::endl;

    engine.shutdown();
    return 0;
}
