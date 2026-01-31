#ifndef ASTRAEUS_DETERMINISTIC_SIM_GENERATOR_HPP
#define ASTRAEUS_DETERMINISTIC_SIM_GENERATOR_HPP

#include "FixedBinaryDecoder.hpp"
#include "core/util/Math.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <cstring>
#include <iostream>
#include <random>

#include "core/util/SafeC.hpp"

namespace astraeus {

/**
 * Entity configuration for generator.
 */
struct SimEntityConfig {
    uint32_t entity_id;
    std::string name;
    std::string team;
    std::string type;
    
    enum class MotionType {
        Stationary,
        Circular,
        Linear,
        Figure8
    };
    
    MotionType motion;
    float radius;        // For circular motion
    float speed;         // Angular or linear speed
    float center_x;      // Center of motion
    float center_y;
    float center_z;
    float direction_x;   // For linear motion
    float direction_y;
    float direction_z;
    float color_r;
    float color_g;
    float color_b;
    float color_a;
    
    SimEntityConfig()
        : entity_id(0)
        , motion(MotionType::Stationary)
        , radius(1.0f)
        , speed(1.0f)
        , center_x(0), center_y(0), center_z(0)
        , direction_x(1), direction_y(0), direction_z(0)
        , color_r(1), color_g(1), color_b(1), color_a(1)
    {}
};

/**
 * DeterministicSimGenerator generates synthetic simulation data
 * for testing the ingest pipeline.
 * 
 * Features:
 * - Configurable entity count
 * - Multiple motion patterns (circular, linear, figure-8)
 * - Deterministic (same seed = same output)
 * - Metadata (name, team, type)
 */
class DeterministicSimGenerator {
public:
    DeterministicSimGenerator();
    ~DeterministicSimGenerator();
    
    /**
     * Initialize generator with entity count.
     */
    bool initialize(uint32_t entity_count, uint32_t seed = 12345);
    
    /**
     * Shutdown.
     */
    void shutdown();
    
    /**
     * Generate next frame at given time.
     * Returns encoded binary data in FixedBinary format.
     */
    std::vector<uint8_t> generate_frame(double timestamp);
    
    /**
     * Get current frame number.
     */
    uint64_t get_frame_number() const { return frame_number_; }
    
    /**
     * Get entity count.
     */
    uint32_t get_entity_count() const { return entity_count_; }
    
    /**
     * Add custom entity configuration.
     */
    void add_entity_config(const SimEntityConfig& config);
    
    /**
     * Generate default entity configurations (called by initialize).
     */
    void generate_default_entities(uint32_t count, uint32_t seed);
    
private:
    bool is_initialized_;
    uint32_t entity_count_;
    uint64_t frame_number_;
    std::vector<SimEntityConfig> entity_configs_;
    
    // Compute entity state at given time
    void compute_entity_state(const SimEntityConfig& config, double time,
                             FixedBinaryDecoder::EntityData& out_entity) const;
};

// Inline implementations

inline DeterministicSimGenerator::DeterministicSimGenerator()
    : is_initialized_(false)
    , entity_count_(0)
    , frame_number_(0)
{
}

inline DeterministicSimGenerator::~DeterministicSimGenerator() {
    shutdown();
}

inline bool DeterministicSimGenerator::initialize(uint32_t entity_count, uint32_t seed) {
    if (is_initialized_) {
        return true;
    }
    
    entity_count_ = entity_count;
    frame_number_ = 0;
    
    // Generate default entity configurations
    generate_default_entities(entity_count, seed);
    
    is_initialized_ = true;
    std::cout << "[DeterministicSimGenerator] Initialized with " << entity_count 
              << " entities (seed=" << seed << ")" << std::endl;
    return true;
}

inline void DeterministicSimGenerator::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    entity_configs_.clear();
    is_initialized_ = false;
    std::cout << "[DeterministicSimGenerator] Shutdown" << std::endl;
}

inline void DeterministicSimGenerator::generate_default_entities(uint32_t count, uint32_t seed) {
    entity_configs_.clear();
    entity_configs_.reserve(count);
    
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist_color(0.3f, 1.0f);
    std::uniform_real_distribution<float> dist_pos(-50.0f, 50.0f);
    std::uniform_real_distribution<float> dist_radius(1.0f, 10.0f);
    std::uniform_real_distribution<float> dist_speed(0.5f, 2.0f);
    std::uniform_int_distribution<int> dist_motion(0, 3);
    std::uniform_int_distribution<int> dist_team(0, 4);
    
    const char* teams[] = {"Red", "Blue", "Green", "Yellow", "Purple"};
    const char* types[] = {"Fighter", "Bomber", "Scout", "Transport"};
    
    for (uint32_t i = 0; i < count; i++) {
        SimEntityConfig config;
        config.entity_id = i + 1; // Start from 1
        
        // Generate name
        char name_buf[64];
        snprintf(name_buf, sizeof(name_buf), "Entity_%05u", config.entity_id);
        config.name = name_buf;
        
        // Random team and type
        int team_idx = dist_team(rng);
        config.team = teams[team_idx];
        config.type = types[i % 4];
        
        // Random color based on team
        if (team_idx == 0) { // Red
            config.color_r = 1.0f;
            config.color_g = dist_color(rng) * 0.3f;
            config.color_b = dist_color(rng) * 0.3f;
        } else if (team_idx == 1) { // Blue
            config.color_r = dist_color(rng) * 0.3f;
            config.color_g = dist_color(rng) * 0.3f;
            config.color_b = 1.0f;
        } else if (team_idx == 2) { // Green
            config.color_r = dist_color(rng) * 0.3f;
            config.color_g = 1.0f;
            config.color_b = dist_color(rng) * 0.3f;
        } else if (team_idx == 3) { // Yellow
            config.color_r = 1.0f;
            config.color_g = 1.0f;
            config.color_b = dist_color(rng) * 0.3f;
        } else { // Purple
            config.color_r = 1.0f;
            config.color_g = dist_color(rng) * 0.3f;
            config.color_b = 1.0f;
        }
        config.color_a = 1.0f;
        
        // Random motion pattern
        int motion = dist_motion(rng);
        switch (motion) {
            case 0:
                config.motion = SimEntityConfig::MotionType::Circular;
                config.radius = dist_radius(rng);
                config.speed = dist_speed(rng);
                break;
            case 1: {
                config.motion = SimEntityConfig::MotionType::Linear;
                config.speed = dist_speed(rng) * 2.0f;
                config.direction_x = dist_pos(rng);
                config.direction_y = dist_pos(rng) * 0.3f;
                config.direction_z = dist_pos(rng);
                // Normalize
                float len = math::sqrt(config.direction_x * config.direction_x +
                                    config.direction_y * config.direction_y +
                                    config.direction_z * config.direction_z);
                if (len > 0.001f) {
                    config.direction_x /= len;
                    config.direction_y /= len;
                    config.direction_z /= len;
                }
                break;
            }
            case 2:
                config.motion = SimEntityConfig::MotionType::Figure8;
                config.radius = dist_radius(rng);
                config.speed = dist_speed(rng);
                break;
            default:
                config.motion = SimEntityConfig::MotionType::Stationary;
                break;
        }
        
        // Random center position
        config.center_x = dist_pos(rng);
        config.center_y = dist_pos(rng) * 0.5f; // Less vertical spread
        config.center_z = dist_pos(rng);
        
        entity_configs_.push_back(config);
    }
}

inline void DeterministicSimGenerator::add_entity_config(const SimEntityConfig& config) {
    entity_configs_.push_back(config);
}

inline void DeterministicSimGenerator::compute_entity_state(
    const SimEntityConfig& config, 
    double time,
    FixedBinaryDecoder::EntityData& out_entity) const {
    
    out_entity.entity_id = config.entity_id;
    out_entity.active = 1;
    
    // Compute position based on motion type
    switch (config.motion) {
        case SimEntityConfig::MotionType::Stationary:
            out_entity.pos_x = config.center_x;
            out_entity.pos_y = config.center_y;
            out_entity.pos_z = config.center_z;
            break;
            
        case SimEntityConfig::MotionType::Circular: {
            float angle = static_cast<float>(time * config.speed);
            out_entity.pos_x = config.center_x + config.radius * std::cos(angle);
            out_entity.pos_y = config.center_y;
            out_entity.pos_z = config.center_z + config.radius * std::sin(angle);
            break;
        }
        
        case SimEntityConfig::MotionType::Linear: {
            float dist = static_cast<float>(time * config.speed);
            out_entity.pos_x = config.center_x + config.direction_x * dist;
            out_entity.pos_y = config.center_y + config.direction_y * dist;
            out_entity.pos_z = config.center_z + config.direction_z * dist;
            break;
        }
        
        case SimEntityConfig::MotionType::Figure8: {
            float t = static_cast<float>(time * config.speed);
            out_entity.pos_x = config.center_x + config.radius * std::cos(t);
            out_entity.pos_y = config.center_y + config.radius * std::sin(2.0f * t) * 0.5f;
            out_entity.pos_z = config.center_z + config.radius * std::sin(t);
            break;
        }
    }
    
    // Rotation (simple spin based on time)
    out_entity.rot_x = 0.0f;
    out_entity.rot_y = static_cast<float>(time * config.speed * 0.5);
    out_entity.rot_z = 0.0f;
    
    // Scale (constant)
    out_entity.scale_x = 1.0f;
    out_entity.scale_y = 1.0f;
    out_entity.scale_z = 1.0f;
    
    // Color
    out_entity.color_r = config.color_r;
    out_entity.color_g = config.color_g;
    out_entity.color_b = config.color_b;
    out_entity.color_a = config.color_a;
    
    // Metadata index matches entity index in our simple case
    out_entity.metadata_index = config.entity_id - 1;
}

inline std::vector<uint8_t> DeterministicSimGenerator::generate_frame(double timestamp) {
    if (!is_initialized_) {
        return std::vector<uint8_t>();
    }
    
    // Calculate buffer size
    size_t header_size = sizeof(FixedBinaryDecoder::Header);
    size_t entities_size = entity_configs_.size() * sizeof(FixedBinaryDecoder::EntityData);
    size_t metadata_size = entity_configs_.size() * sizeof(FixedBinaryDecoder::MetadataData);
    size_t total_size = header_size + entities_size + metadata_size;
    
    std::vector<uint8_t> buffer(total_size);
    uint8_t* ptr = buffer.data();
    
    // Write header
    FixedBinaryDecoder::Header* header = reinterpret_cast<FixedBinaryDecoder::Header*>(ptr);
    header->magic = FixedBinaryDecoder::MAGIC;
    header->version = FixedBinaryDecoder::VERSION;
    header->timestamp = timestamp;
    header->frame_number = frame_number_;
    header->entity_count = static_cast<uint32_t>(entity_configs_.size());
    header->metadata_count = static_cast<uint32_t>(entity_configs_.size());
    ptr += sizeof(FixedBinaryDecoder::Header);
    
    // Write entities
    FixedBinaryDecoder::EntityData* entities = 
        reinterpret_cast<FixedBinaryDecoder::EntityData*>(ptr);
    
    for (size_t i = 0; i < entity_configs_.size(); i++) {
        compute_entity_state(entity_configs_[i], timestamp, entities[i]);
    }
    ptr += entities_size;
    
    // Write metadata
    FixedBinaryDecoder::MetadataData* metadata = 
        reinterpret_cast<FixedBinaryDecoder::MetadataData*>(ptr);
    
    for (size_t i = 0; i < entity_configs_.size(); i++) {
        const SimEntityConfig& config = entity_configs_[i];
        util::str_copy(metadata[i].name, sizeof(metadata[i].name) - 1, config.name.c_str() );
        util::str_copy(metadata[i].team, sizeof(metadata[i].team) - 1, config.team.c_str());
        util::str_copy(metadata[i].type, sizeof(metadata[i].type) - 1 , config.type.c_str());
        metadata[i].name[sizeof(metadata[i].name) - 1] = '\0';
        metadata[i].team[sizeof(metadata[i].team) - 1] = '\0';
        metadata[i].type[sizeof(metadata[i].type) - 1] = '\0';
    }
    
    frame_number_++;
    
    return buffer;
}

} // namespace astraeus

#endif // ASTRAEUS_DETERMINISTIC_SIM_GENERATOR_HPP
