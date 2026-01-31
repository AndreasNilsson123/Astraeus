#ifndef ASTRAEUS_VERTEX_FORMAT_HPP
#define ASTRAEUS_VERTEX_FORMAT_HPP

#include <cstdint>
#include <vector>

namespace astraeus {

/**
 * Supported vertex attribute types
 */
enum class VertexAttributeType {
    Position,      // 3D position (x, y, z)
    Normal,        // 3D normal vector (nx, ny, nz)
    TexCoord,      // 2D texture coordinate (u, v)
    Color,         // RGBA color (r, g, b, a)
    Tangent,       // 3D tangent vector (tx, ty, tz)
    Bitangent      // 3D bitangent vector (bx, by, bz)
};

/**
 * Vertex attribute descriptor
 */
struct VertexAttribute {
    VertexAttributeType type;
    uint32_t offset;        // Offset in bytes from start of vertex
    uint32_t component_count; // Number of components (2, 3, or 4)
    uint32_t gl_location;   // OpenGL attribute location
    
    VertexAttribute(VertexAttributeType t, uint32_t off, uint32_t comp, uint32_t loc)
        : type(t), offset(off), component_count(comp), gl_location(loc) {}
};

/**
 * Vertex format descriptor - defines the layout of vertex data
 */
class VertexFormat {
public:
    VertexFormat() : stride_(0) {}
    
    /**
     * Add an attribute to the vertex format
     */
    void add_attribute(VertexAttributeType type, uint32_t component_count, uint32_t gl_location) {
        attributes_.emplace_back(type, stride_, component_count, gl_location);
        stride_ += component_count * sizeof(float);
    }
    
    /**
     * Get the list of attributes
     */
    const std::vector<VertexAttribute>& get_attributes() const {
        return attributes_;
    }
    
    /**
     * Get the stride (size) of a single vertex
     */
    uint32_t get_stride() const {
        return stride_;
    }
    
    /**
     * Check if this format has a specific attribute type
     */
    bool has_attribute(VertexAttributeType type) const {
        for (const auto& attr : attributes_) {
            if (attr.type == type) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * Get attribute by type
     */
    const VertexAttribute* get_attribute(VertexAttributeType type) const {
        for (const auto& attr : attributes_) {
            if (attr.type == type) {
                return &attr;
            }
        }
        return nullptr;
    }
    
private:
    std::vector<VertexAttribute> attributes_;
    uint32_t stride_;
};

/**
 * Standard vertex formats
 */
namespace StandardFormats {
    /**
     * Position + Normal + TexCoord (P3N3T2)
     * Layout: [x, y, z, nx, ny, nz, u, v]
     * Total: 32 bytes per vertex
     */
    inline VertexFormat create_P3N3T2() {
        VertexFormat format;
        format.add_attribute(VertexAttributeType::Position, 3, 0);
        format.add_attribute(VertexAttributeType::Normal, 3, 1);
        format.add_attribute(VertexAttributeType::TexCoord, 2, 2);
        return format;
    }
    
    /**
     * Position + Normal + TexCoord + Color (P3N3T2C4)
     * Layout: [x, y, z, nx, ny, nz, u, v, r, g, b, a]
     * Total: 48 bytes per vertex
     */
    inline VertexFormat create_P3N3T2C4() {
        VertexFormat format;
        format.add_attribute(VertexAttributeType::Position, 3, 0);
        format.add_attribute(VertexAttributeType::Normal, 3, 1);
        format.add_attribute(VertexAttributeType::TexCoord, 2, 2);
        format.add_attribute(VertexAttributeType::Color, 4, 3);
        return format;
    }
    
    /**
     * Position + Color (P3C4)
     * Layout: [x, y, z, r, g, b, a]
     * Total: 28 bytes per vertex
     */
    inline VertexFormat create_P3C4() {
        VertexFormat format;
        format.add_attribute(VertexAttributeType::Position, 3, 0);
        format.add_attribute(VertexAttributeType::Color, 4, 1);
        return format;
    }
    
    /**
     * Position + Normal + TexCoord + Tangent + Bitangent (P3N3T2TB3)
     * For normal mapping
     * Layout: [x, y, z, nx, ny, nz, u, v, tx, ty, tz, bx, by, bz]
     * Total: 56 bytes per vertex
     */
    inline VertexFormat create_P3N3T2TB3() {
        VertexFormat format;
        format.add_attribute(VertexAttributeType::Position, 3, 0);
        format.add_attribute(VertexAttributeType::Normal, 3, 1);
        format.add_attribute(VertexAttributeType::TexCoord, 2, 2);
        format.add_attribute(VertexAttributeType::Tangent, 3, 3);
        format.add_attribute(VertexAttributeType::Bitangent, 3, 4);
        return format;
    }
}

/**
 * Extended vertex structures for different formats
 */

// Standard vertex (P3N3T2) - 32 bytes
struct Vertex_P3N3T2 {
    float x, y, z;       // Position
    float nx, ny, nz;    // Normal
    float u, v;          // UV coordinates
};

// Vertex with color (P3N3T2C4) - 48 bytes
struct Vertex_P3N3T2C4 {
    float x, y, z;       // Position
    float nx, ny, nz;    // Normal
    float u, v;          // UV coordinates
    float r, g, b, a;    // Color
};

// Simple colored vertex (P3C4) - 28 bytes
struct Vertex_P3C4 {
    float x, y, z;       // Position
    float r, g, b, a;    // Color
};

// Vertex with tangent space (P3N3T2TB3) - 56 bytes
struct Vertex_P3N3T2TB3 {
    float x, y, z;       // Position
    float nx, ny, nz;    // Normal
    float u, v;          // UV coordinates
    float tx, ty, tz;    // Tangent
    float bx, by, bz;    // Bitangent
};

} // namespace astraeus

#endif // ASTRAEUS_VERTEX_FORMAT_HPP
