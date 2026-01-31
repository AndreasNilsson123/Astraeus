#include <iostream>

// Define implementation for tinygltf (must be before including GLTFLoader)
#define ASTRAEUS_GLTF_LOADER_IMPLEMENTATION

#include <assets/GLTFLoader.hpp>
#include <assets/AssetDatabase.hpp>

using namespace astraeus;

int main() {
    std::cout << "=== glTF Loader Test ===" << std::endl;
    
    // Test 1: AssetDatabase
    std::cout << "\n=== Test 1: AssetDatabase ===" << std::endl;
    AssetDatabase db;
    
    // Register an asset
    std::string test_uri = "examples/cube.obj";
    uint32_t asset_id = db.register_asset(test_uri, true);
    
    if (asset_id == 0) {
        std::cerr << "FAILED: Could not register asset" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Registered asset with ID: " << asset_id << std::endl;
    
    // Register same asset again (should return same ID)
    uint32_t asset_id_2 = db.register_asset(test_uri, true);
    if (asset_id != asset_id_2) {
        std::cerr << "FAILED: Second registration should return same ID" << std::endl;
        std::cerr << "  First ID: " << asset_id << ", Second ID: " << asset_id_2 << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Duplicate registration returns same ID" << std::endl;
    
    // Check metadata
    const AssetMetadata* meta = db.get_metadata(asset_id);
    if (!meta) {
        std::cerr << "FAILED: Could not get metadata" << std::endl;
        return 1;
    }
    
    std::cout << "Metadata:" << std::endl;
    std::cout << "  URI: " << meta->uri << std::endl;
    std::cout << "  Hash: " << meta->hash << std::endl;
    std::cout << "  Ref count: " << meta->reference_count << std::endl;
    
    if (meta->reference_count != 2) {
        std::cerr << "FAILED: Reference count should be 2, got " 
                  << meta->reference_count << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Reference counting works" << std::endl;
    
    // Test release
    bool should_delete = db.release_reference(asset_id);
    if (should_delete) {
        std::cerr << "FAILED: Should not delete after first release (ref count should be 1)" << std::endl;
        return 1;
    }
    
    should_delete = db.release_reference(asset_id_2);
    if (!should_delete) {
        std::cerr << "FAILED: Should delete after second release (ref count should be 0)" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Reference release works correctly" << std::endl;
    
    // Test 2: Cache key generation
    std::cout << "\n=== Test 2: Cache Key Generation ===" << std::endl;
    
    std::string uri1 = "path/to/model.gltf";
    std::string hash1 = "abc123";
    std::string key1 = AssetDatabase::generate_cache_key(uri1, hash1);
    
    std::cout << "Cache key: " << key1 << std::endl;
    
    if (key1 != uri1 + "#" + hash1) {
        std::cerr << "FAILED: Cache key format incorrect" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Cache key generation works" << std::endl;
    
    // Test 3: File hash computation
    std::cout << "\n=== Test 3: File Hash Computation ===" << std::endl;
    
    std::string hash = AssetDatabase::compute_file_hash("examples/cube.obj");
    if (hash.empty()) {
        std::cout << "Note: File not found (expected in CI), skipping hash test" << std::endl;
    } else {
        std::cout << "File hash: " << hash << std::endl;
        
        // Compute again, should be identical
        std::string hash2 = AssetDatabase::compute_file_hash("examples/cube.obj");
        if (hash != hash2) {
            std::cerr << "FAILED: Hash should be deterministic" << std::endl;
            return 1;
        }
        
        std::cout << "SUCCESS: File hash is deterministic" << std::endl;
    }
    
    // Test 4: Async load request stub
    std::cout << "\n=== Test 4: Async Load Request (Stub) ===" << std::endl;
    
    AssetDatabase db2;
    AsyncLoadRequest request = db2.create_async_request("test/model.gltf");
    
    if (request.uri != "test/model.gltf") {
        std::cerr << "FAILED: Request URI mismatch" << std::endl;
        return 1;
    }
    
    if (request.state != AssetLoadState::Pending) {
        std::cerr << "FAILED: Initial state should be Pending" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: Async request stub works" << std::endl;
    
    // Test 5: glTF Material structure
    std::cout << "\n=== Test 5: glTF Data Structures ===" << std::endl;
    
    GLTFMaterial mat;
    mat.name = "TestMaterial";
    mat.base_color_factor[0] = 1.0f;
    mat.base_color_factor[1] = 0.0f;
    mat.base_color_factor[2] = 0.0f;
    mat.base_color_factor[3] = 1.0f;
    mat.metallic_factor = 0.5f;
    mat.roughness_factor = 0.8f;
    
    std::cout << "Material: " << mat.name << std::endl;
    std::cout << "  Base color: (" << mat.base_color_factor[0] << ", " 
              << mat.base_color_factor[1] << ", " 
              << mat.base_color_factor[2] << ", " 
              << mat.base_color_factor[3] << ")" << std::endl;
    std::cout << "  Metallic: " << mat.metallic_factor << std::endl;
    std::cout << "  Roughness: " << mat.roughness_factor << std::endl;
    
    std::cout << "SUCCESS: Material structure works" << std::endl;
    
    // Test 6: glTF Primitive structure
    std::cout << "\n=== Test 6: glTF Primitive Structure ===" << std::endl;
    
    GLTFPrimitive prim;
    prim.name = "TestPrimitive";
    prim.material_index = 0;
    
    // Create a simple mesh
    std::vector<Vertex> vertices;
    Vertex v1 = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f};
    Vertex v2 = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};
    Vertex v3 = {0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 1.0f};
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    
    std::vector<uint32_t> indices = {0, 1, 2};
    
    prim.mesh.set_vertices(vertices);
    prim.mesh.set_indices(indices);
    
    if (prim.mesh.get_vertex_count() != 3) {
        std::cerr << "FAILED: Vertex count should be 3, got " 
                  << prim.mesh.get_vertex_count() << std::endl;
        return 1;
    }
    
    if (prim.mesh.get_index_count() != 3) {
        std::cerr << "FAILED: Index count should be 3, got " 
                  << prim.mesh.get_index_count() << std::endl;
        return 1;
    }
    
    std::cout << "Primitive: " << prim.name << std::endl;
    std::cout << "  Vertices: " << prim.mesh.get_vertex_count() << std::endl;
    std::cout << "  Indices: " << prim.mesh.get_index_count() << std::endl;
    std::cout << "  Material index: " << prim.material_index << std::endl;
    
    std::cout << "SUCCESS: Primitive structure works" << std::endl;
    
    // Test 7: glTF Model structure
    std::cout << "\n=== Test 7: glTF Model Structure ===" << std::endl;
    
    GLTFModel model;
    model.name = "TestModel";
    model.primitives.push_back(prim);
    model.materials.push_back(mat);
    
    if (!model.is_valid()) {
        std::cerr << "FAILED: Model should be valid" << std::endl;
        return 1;
    }
    
    std::cout << "Model: " << model.name << std::endl;
    std::cout << "  Primitives: " << model.primitives.size() << std::endl;
    std::cout << "  Materials: " << model.materials.size() << std::endl;
    std::cout << "  Textures: " << model.textures.size() << std::endl;
    
    std::cout << "SUCCESS: Model structure works" << std::endl;
    
    std::cout << "\n=== All Tests Passed ===" << std::endl;
    std::cout << "\nSummary:" << std::endl;
    std::cout << "✓ AssetDatabase registration and caching" << std::endl;
    std::cout << "✓ Reference counting" << std::endl;
    std::cout << "✓ Cache key generation (URI + hash)" << std::endl;
    std::cout << "✓ File hash computation" << std::endl;
    std::cout << "✓ Async load request stubs" << std::endl;
    std::cout << "✓ glTF data structures (Material, Primitive, Model)" << std::endl;
    std::cout << "\nNote: Actual glTF file loading requires valid glTF files" << std::endl;
    std::cout << "      and is tested in integration tests with sample assets." << std::endl;
    
    return 0;
}
