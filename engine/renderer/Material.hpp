#ifndef ASTRAEUS_MATERIAL_HPP
#define ASTRAEUS_MATERIAL_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>

namespace astraeus {

// Forward declarations
class RenderDevice;

/**
 * Material parameter types
 */
enum class MaterialParameterType {
    Float,
    Vec2,
    Vec3,
    Vec4,
    Int,
    Texture2D,
    Mat4
};

/**
 * Material parameter - stores shader uniform data
 */
struct MaterialParameter {
    MaterialParameterType type;
    union {
        float float_value;
        float vec2_value[2];
        float vec3_value[3];
        float vec4_value[4];
        int int_value;
        uint32_t texture_id;
        float mat4_value[16];
    } data;

    MaterialParameter() : type(MaterialParameterType::Float) {
        data.float_value = 0.0f;
    }
};

/**
 * Pipeline state configuration for materials
 */
struct PipelineState {
    // Depth testing
    bool depth_test_enabled = true;
    bool depth_write_enabled = true;
    
    // Blending
    bool blend_enabled = false;
    uint32_t blend_src_factor = 0x0302; // GL_SRC_ALPHA
    uint32_t blend_dst_factor = 0x0303; // GL_ONE_MINUS_SRC_ALPHA
    
    // Culling
    bool cull_enabled = true;
    uint32_t cull_face = 0x0405; // GL_BACK
    
    // Primitive topology
    uint32_t primitive_type = 0x0004; // GL_TRIANGLES
};

/**
 * Material - Base class for all materials.
 * Defines shader program, pipeline state, and parameters.
 */
class Material {
public:
    virtual ~Material() = default;

    /**
     * Get the material name (for debugging/identification)
     */
    virtual const char* get_name() const = 0;

    /**
     * Initialize the material (compile shaders, etc.)
     */
    virtual bool initialize(RenderDevice* device) = 0;

    /**
     * Cleanup material resources
     */
    virtual void shutdown() = 0;

    /**
     * Bind the material for rendering
     */
    virtual void bind(RenderDevice* device) = 0;

    /**
     * Set a material parameter
     */
    virtual void set_parameter(const char* name, const MaterialParameter& param) = 0;

    /**
     * Apply all material parameters to the shader
     */
    virtual void apply_parameters(RenderDevice* device) = 0;

    /**
     * Get the pipeline state for this material
     */
    virtual const PipelineState& get_pipeline_state() const = 0;

    /**
     * Check if material is initialized
     */
    virtual bool is_initialized() const = 0;

protected:
    Material() = default;
};

/**
 * Material parameters storage - used by MaterialInstance
 */
class MaterialParameters {
public:
    MaterialParameters() = default;

    void set_float(const char* name, float value) {
        MaterialParameter param;
        param.type = MaterialParameterType::Float;
        param.data.float_value = value;
        parameters_[name] = param;
    }

    void set_vec2(const char* name, float x, float y) {
        MaterialParameter param;
        param.type = MaterialParameterType::Vec2;
        param.data.vec2_value[0] = x;
        param.data.vec2_value[1] = y;
        parameters_[name] = param;
    }

    void set_vec3(const char* name, float x, float y, float z) {
        MaterialParameter param;
        param.type = MaterialParameterType::Vec3;
        param.data.vec3_value[0] = x;
        param.data.vec3_value[1] = y;
        param.data.vec3_value[2] = z;
        parameters_[name] = param;
    }

    void set_vec4(const char* name, float x, float y, float z, float w) {
        MaterialParameter param;
        param.type = MaterialParameterType::Vec4;
        param.data.vec4_value[0] = x;
        param.data.vec4_value[1] = y;
        param.data.vec4_value[2] = z;
        param.data.vec4_value[3] = w;
        parameters_[name] = param;
    }

    void set_int(const char* name, int value) {
        MaterialParameter param;
        param.type = MaterialParameterType::Int;
        param.data.int_value = value;
        parameters_[name] = param;
    }

    void set_texture(const char* name, uint32_t texture_id) {
        MaterialParameter param;
        param.type = MaterialParameterType::Texture2D;
        param.data.texture_id = texture_id;
        parameters_[name] = param;
    }

    void set_mat4(const char* name, const float* matrix) {
        MaterialParameter param;
        param.type = MaterialParameterType::Mat4;
        for (int i = 0; i < 16; ++i) {
            param.data.mat4_value[i] = matrix[i];
        }
        parameters_[name] = param;
    }

    const std::unordered_map<std::string, MaterialParameter>& get_all() const {
        return parameters_;
    }

    void clear() {
        parameters_.clear();
    }

private:
    std::unordered_map<std::string, MaterialParameter> parameters_;
};

/**
 * MaterialInstance - Instance of a material with per-object parameters.
 * Allows multiple objects to share the same material definition but with different parameters.
 */
class MaterialInstance {
public:
    explicit MaterialInstance(Material* base_material)
        : base_material_(base_material)
    {
    }

    ~MaterialInstance() = default;

    /**
     * Get the base material
     */
    Material* get_base_material() const {
        return base_material_;
    }

    /**
     * Get the instance parameters
     */
    MaterialParameters& get_parameters() {
        return parameters_;
    }

    const MaterialParameters& get_parameters() const {
        return parameters_;
    }

    /**
     * Bind the material and apply instance parameters
     */
    void bind(RenderDevice* device) {
        if (base_material_) {
            base_material_->bind(device);
            
            // Apply instance-specific parameters
            for (const auto& pair : parameters_.get_all()) {
                base_material_->set_parameter(pair.first.c_str(), pair.second);
            }
            
            base_material_->apply_parameters(device);
        }
    }

private:
    Material* base_material_;
    MaterialParameters parameters_;
};

} // namespace astraeus

#endif // ASTRAEUS_MATERIAL_HPP
