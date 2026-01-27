#ifndef ASTRAEUS_DETERMINISTIC_SIM_GENERATOR_HPP
#define ASTRAEUS_DETERMINISTIC_SIM_GENERATOR_HPP

#include "FixedBinaryDecoder.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

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

} // namespace astraeus

#endif // ASTRAEUS_DETERMINISTIC_SIM_GENERATOR_HPP
