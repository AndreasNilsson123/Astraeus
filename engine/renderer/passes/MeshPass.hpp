#ifndef ASTRAEUS_MESH_PASS_HPP
#define ASTRAEUS_MESH_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"
#include "../MaterialLibrary.hpp"
#include "../../scene/World.hpp"
#include "../../assets/AssetManager.hpp"
#include "../../core/util/Math.hpp"

#include <glad/glad.h>

namespace astraeus {

/**
 * MeshPass: Renders meshes loaded via AssetManager.
 * Uses Material system for flexible rendering with multiple materials.
 */
class MeshPass : public RenderPass {
public:
    inline MeshPass(AssetManager* asset_manager, MaterialLibrary* material_library);
    ~MeshPass() override = default;

    inline bool initialize(RenderDevice* device) override;
    inline void execute(RenderDevice* device, World* world) override;
    inline void on_resize(uint32_t width, uint32_t height) override;
    inline const char* get_name() const override { return "Mesh"; }

private:
    inline void compute_mvp_matrix(const Transform& transform, const Camera& camera, 
                                   uint32_t width, uint32_t height, float* out_mvp);
    inline void multiply_matrices(const float* a, const float* b, float* out);
    inline void identity_matrix(float* out);
    inline void translation_matrix(float x, float y, float z, float* out);
    inline void scale_matrix(float sx, float sy, float sz, float* out);
    inline void perspective_matrix(float fov, float aspect, float near, float far, float* out);
    inline void look_at_matrix(float eye_x, float eye_y, float eye_z,
                              float center_x, float center_y, float center_z,
                              float up_x, float up_y, float up_z, float* out);

    GLRenderDevice* gl_device_;
    AssetManager* asset_manager_;
    MaterialLibrary* material_library_;
    uint32_t viewport_width_;
    uint32_t viewport_height_;
};

// Implementation

inline MeshPass::MeshPass(AssetManager* asset_manager, MaterialLibrary* material_library)
    : gl_device_(nullptr)
    , asset_manager_(asset_manager)
    , material_library_(material_library)
    , viewport_width_(1280)
    , viewport_height_(720)
{
}

inline bool MeshPass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }

    // Material library is initialized externally
    if (!material_library_) {
        std::cerr << "[MeshPass] Material library not provided" << std::endl;
        return false;
    }

    std::cout << "[MeshPass] Initialized" << std::endl;
    return true;
}

inline void MeshPass::execute(RenderDevice* device, World* world) {
    (void)device;

    if (!gl_device_ || !asset_manager_ || !world || !material_library_) {
        return;
    }

    gl_device_->push_debug_group("MeshPass");

    // Get default unlit material
    Material* default_material = material_library_->get_default_unlit();
    if (!default_material) {
        std::cerr << "[MeshPass] No default material available" << std::endl;
        gl_device_->pop_debug_group();
        return;
    }

    // Get camera
    const Camera& camera = world->get_camera();

    // Track last bound material to avoid redundant binds
    Material* last_material = nullptr;

    // Render all entities with mesh components
    const auto& entities = world->get_renderable_entities();
    for (uint32_t entity_id : entities) {
        // Check if entity has a transform
        const Transform* transform = world->get_entity_transform(entity_id);
        if (!transform) {
            continue;
        }

        // Check if entity has a renderable component
        const Renderable* renderable = world->get_entity_renderable(entity_id);
        if (!renderable || !renderable->visible) {
            continue;
        }

        // For now, assume entity_id corresponds to asset_id
        // In a real system, entities would have a MeshComponent with asset_id
        const GPUMesh* gpu_mesh = asset_manager_->get_gpu_mesh(entity_id);
        if (!gpu_mesh || !gpu_mesh->is_valid()) {
            continue;
        }

        // Get material (use default for now; future: per-entity materials)
        Material* material = default_material;

        // Bind material only if it changed (avoid redundant state changes)
        if (material != last_material) {
            material->bind(device);
            last_material = material;
        }

        // Compute MVP matrix
        float mvp[16];
        compute_mvp_matrix(*transform, camera, viewport_width_, viewport_height_, mvp);
        
        // Set MVP parameter
        MaterialParameter mvp_param;
        mvp_param.type = MaterialParameterType::Mat4;
        std::memcpy(mvp_param.data.mat4_value, mvp, sizeof(float) * 16);
        material->set_parameter("mvp", mvp_param);

        // Set color (use entity color if available, otherwise white)
        MaterialParameter color_param;
        color_param.type = MaterialParameterType::Vec4;
        const Color* color = world->get_entity_color(entity_id);
        if (color) {
            color_param.data.vec4_value[0] = color->r;
            color_param.data.vec4_value[1] = color->g;
            color_param.data.vec4_value[2] = color->b;
            color_param.data.vec4_value[3] = color->a;
        } else {
            color_param.data.vec4_value[0] = 1.0f;
            color_param.data.vec4_value[1] = 1.0f;
            color_param.data.vec4_value[2] = 1.0f;
            color_param.data.vec4_value[3] = 1.0f;
        }
        material->set_parameter("baseColor", color_param);

        // Apply all parameters
        material->apply_parameters(device);

        // Bind VAO and draw
        glBindVertexArray(gpu_mesh->vao);
        
        if (gpu_mesh->index_count > 0) {
            glDrawElements(GL_TRIANGLES, gpu_mesh->index_count, GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, gpu_mesh->vertex_count);
        }
        
        glBindVertexArray(0);
    }

    gl_device_->pop_debug_group();
}

inline void MeshPass::on_resize(uint32_t width, uint32_t height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

// Matrix math helpers

inline void MeshPass::identity_matrix(float* out) {
    for (int i = 0; i < 16; i++) {
        out[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
}

inline void MeshPass::translation_matrix(float x, float y, float z, float* out) {
    identity_matrix(out);
    out[12] = x;
    out[13] = y;
    out[14] = z;
}

inline void MeshPass::scale_matrix(float sx, float sy, float sz, float* out) {
    identity_matrix(out);
    out[0] = sx;
    out[5] = sy;
    out[10] = sz;
}

inline void MeshPass::perspective_matrix(float fov, float aspect, float near_dist, float far_dist, float* out) {
    // Validate parameters
    if (aspect < 1e-6f || near_dist <= 0.0f || far_dist <= near_dist) {
        identity_matrix(out);
        return;
    }
    
    float f = 1.0f / std::tan(fov * 0.5f);
    
    for (int i = 0; i < 16; i++) out[i] = 0.0f;
    
    out[0] = f / aspect;
    out[5] = f;
    out[10] = (far_dist + near_dist) / (near_dist - far_dist);
    out[11] = -1.0f;
    out[14] = (2.0f * far_dist * near_dist) / (near_dist - far_dist);
}

inline void MeshPass::look_at_matrix(float eye_x, float eye_y, float eye_z,
                                    float center_x, float center_y, float center_z,
                                    float up_x, float up_y, float up_z, float* out) {
    // Forward vector
    float fx = center_x - eye_x;
    float fy = center_y - eye_y;
    float fz = center_z - eye_z;
    float f_len = math::sqrt(fx*fx + fy*fy + fz*fz);
    
    // Avoid division by zero
    if (f_len < 1e-6f) {
        identity_matrix(out);
        return;
    }
    
    fx /= f_len; fy /= f_len; fz /= f_len;
    
    // Right vector (cross product of forward and up)
    float rx = fy * up_z - fz * up_y;
    float ry = fz * up_x - fx * up_z;
    float rz = fx * up_y - fy * up_x;
    float r_len = math::sqrt(rx*rx + ry*ry + rz*rz);
    
    // Avoid division by zero
    if (r_len < 1e-6f) {
        identity_matrix(out);
        return;
    }
    
    rx /= r_len; ry /= r_len; rz /= r_len;
    
    // Up vector (cross product of right and forward)
    float ux = ry * fz - rz * fy;
    float uy = rz * fx - rx * fz;
    float uz = rx * fy - ry * fx;
    
    identity_matrix(out);
    out[0] = rx;  out[4] = ry;  out[8]  = rz;  out[12] = -(rx*eye_x + ry*eye_y + rz*eye_z);
    out[1] = ux;  out[5] = uy;  out[9]  = uz;  out[13] = -(ux*eye_x + uy*eye_y + uz*eye_z);
    out[2] = -fx; out[6] = -fy; out[10] = -fz; out[14] = (fx*eye_x + fy*eye_y + fz*eye_z);
    out[3] = 0;   out[7] = 0;   out[11] = 0;   out[15] = 1;
}

inline void MeshPass::multiply_matrices(const float* a, const float* b, float* out) {
    float temp[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i*4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                temp[i*4 + j] += a[i*4 + k] * b[k*4 + j];
            }
        }
    }
    for (int i = 0; i < 16; i++) {
        out[i] = temp[i];
    }
}

inline void MeshPass::compute_mvp_matrix(const Transform& transform, const Camera& camera,
                                        uint32_t width, uint32_t height, float* out_mvp) {
    // Model matrix (translation and scale)
    float model[16];
    translation_matrix(transform.pos_x, transform.pos_y, transform.pos_z, model);
    
    float scale[16];
    scale_matrix(transform.scale_x, transform.scale_y, transform.scale_z, scale);
    
    float model_temp[16];
    multiply_matrices(model, scale, model_temp);
    
    // View matrix
    float eye_x, eye_y, eye_z;
    camera.get_position(eye_x, eye_y, eye_z);
    
    float target_x, target_y, target_z;
    camera.get_target(target_x, target_y, target_z);
    
    float view[16];
    look_at_matrix(eye_x, eye_y, eye_z,
                  target_x, target_y, target_z,
                  0.0f, 1.0f, 0.0f, view);  // Use up vector (0, 1, 0)
    
    // Projection matrix
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    float fov_radians = camera.get_fov() * 3.14159265359f / 180.0f;
    float proj[16];
    perspective_matrix(fov_radians, aspect, camera.get_near_plane(), camera.get_far_plane(), proj);
    
    // Compute MVP = Projection * View * Model
    float vp[16];
    multiply_matrices(proj, view, vp);
    multiply_matrices(vp, model_temp, out_mvp);
}

} // namespace astraeus

#endif // ASTRAEUS_MESH_PASS_HPP
