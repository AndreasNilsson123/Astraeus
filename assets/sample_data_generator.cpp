/**
 * Sample Data Generator
 * 
 * Generates sample simulation data files in FixedBinary format
 * for testing the data ingestion pipeline.
 */

#include <fstream>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <vector>

// Match the FixedBinaryDecoder format
#pragma pack(push, 1)
struct EntityData {
    uint32_t entity_id;
    float pos_x, pos_y, pos_z;
    float rot_x, rot_y, rot_z;
    float scale_x, scale_y, scale_z;
    float color_r, color_g, color_b, color_a;
    uint32_t metadata_index;
    uint8_t active;
};

struct MetadataData {
    char name[64];
    char team[32];
    char type[32];
};

struct Header {
    uint32_t magic;        // 0x41535430 ("AST0")
    uint32_t version;      // Format version
    double timestamp;
    uint64_t frame_number;
    uint32_t entity_count;
    uint32_t metadata_count;
};
#pragma pack(pop)

constexpr uint32_t MAGIC = 0x41535430; // "AST0"
constexpr uint32_t VERSION = 1;

void generate_sample_file(const char* filename, uint32_t entity_count, double timestamp) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    
    // Write header
    Header header;
    header.magic = MAGIC;
    header.version = VERSION;
    header.timestamp = timestamp;
    header.frame_number = static_cast<uint64_t>(timestamp * 10.0); // Assuming 10 Hz
    header.entity_count = entity_count;
    header.metadata_count = entity_count; // One metadata per entity
    
    file.write(reinterpret_cast<const char*>(&header), sizeof(Header));
    
    // Generate entity data
    for (uint32_t i = 0; i < entity_count; i++) {
        EntityData entity;
        entity.entity_id = i + 1; // Entity IDs start at 1
        
        // Create interesting patterns
        float angle = (i / static_cast<float>(entity_count)) * 2.0f * 3.14159f;
        float radius = 10.0f + (i % 10) * 2.0f;
        
        // Circular motion
        entity.pos_x = radius * std::cos(angle + timestamp);
        entity.pos_y = (i % 5) * 2.0f - 4.0f; // Layered heights
        entity.pos_z = radius * std::sin(angle + timestamp);
        
        // No rotation for simplicity
        entity.rot_x = 0.0f;
        entity.rot_y = 0.0f;
        entity.rot_z = 0.0f;
        
        // Uniform scale
        entity.scale_x = 1.0f;
        entity.scale_y = 1.0f;
        entity.scale_z = 1.0f;
        
        // Color based on position
        entity.color_r = 0.5f + 0.5f * std::cos(angle);
        entity.color_g = 0.5f + 0.5f * std::sin(angle);
        entity.color_b = 0.7f;
        entity.color_a = 1.0f;
        
        entity.metadata_index = i;
        entity.active = 1;
        
        file.write(reinterpret_cast<const char*>(&entity), sizeof(EntityData));
    }
    
    // Generate metadata
    for (uint32_t i = 0; i < entity_count; i++) {
        MetadataData metadata;
        std::snprintf(metadata.name, sizeof(metadata.name), "Entity_%04u", i + 1);
        std::snprintf(metadata.team, sizeof(metadata.team), "Team_%u", (i % 5) + 1);
        std::snprintf(metadata.type, sizeof(metadata.type), "Particle");
        
        file.write(reinterpret_cast<const char*>(&metadata), sizeof(MetadataData));
    }
    
    file.close();
    std::cout << "Generated: " << filename << " with " << entity_count << " entities" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "==============================================\n";
    std::cout << "  Sample Data Generator\n";
    std::cout << "==============================================\n\n";
    
    // Generate different sample files
    
    // Small test file (100 entities)
    generate_sample_file("sample_data_100.bin", 100, 0.0);
    
    // Medium test file (1000 entities)
    generate_sample_file("sample_data_1000.bin", 1000, 0.0);
    
    // Large test file (10000 entities)
    generate_sample_file("sample_data_10000.bin", 10000, 0.0);
    
    // Time series (10 frames at different timestamps)
    for (int frame = 0; frame < 10; frame++) {
        char filename[64];
        std::snprintf(filename, sizeof(filename), "sample_data_frame_%02d.bin", frame);
        generate_sample_file(filename, 100, frame * 0.1); // 10 Hz
    }
    
    std::cout << "\nAll sample files generated successfully!\n";
    std::cout << "\nGenerated files:\n";
    std::cout << "  - sample_data_100.bin    (100 entities)\n";
    std::cout << "  - sample_data_1000.bin   (1,000 entities)\n";
    std::cout << "  - sample_data_10000.bin  (10,000 entities)\n";
    std::cout << "  - sample_data_frame_*.bin (10 frames, 100 entities each)\n";
    std::cout << "\nUse these files with:\n";
    std::cout << "  astraeus_ingest_data(engine, data, size, ASTRAEUS_FORMAT_FIXED_BINARY);\n";
    
    return 0;
}
