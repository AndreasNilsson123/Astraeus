#ifndef ASTRAEUS_STATIC_MESH_PASS_HPP
#define ASTRAEUS_STATIC_MESH_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"
#include "../MaterialLibrary.hpp"
#include "../../scene/World.hpp"
#include "../../assets/AssetManager.hpp"
#include "../../core/util/Math.hpp"

#include "platform/GL/GLHeaders.hpp"

#include <vector>
#include <unordered_map>
#include <algorithm>

namespace astraeus {

/**
 * Draw call descriptor for batching
 */
struct DrawCall {
    uint32_t entity_id;
    uint32_t mesh_id;
    Material* material;
    const Transform* transform;
    const Color* color;
    
    DrawCall(uint32_t eid, uint32_t mid, Material* mat, const Transform* t, const Color* c)
        : entity_id(eid), mesh_id(mid), material(mat), transform(t), color(c) {}
};

/**
 * StaticMeshPass: Optimized rendering pass for static meshes.
 * Batches draw calls by material and mesh to minimize state changes.
 * 
 * Features:
 * - Material batching: Groups objects by material to reduce binds
 * - Mesh batching: Groups objects by mesh within each material group
 * - Minimal state changes: Only updates uniforms that differ per object
 */
class StaticMeshPass : public RenderPass {
public:
    inline StaticMeshPass(AssetManager* asset_manager, MaterialLibrary* material_library);
    ~StaticMeshPass() override = default;

    inline bool initialize(RenderDevice* device) override;
    inline void execute(RenderDevice* device, World* world) override;
    inline void on_resize(uint32_t width, uint32_t height) override;
    inline const char* get_name() const override { return "StaticMesh"; }

private:
    inline void build_draw_list(World* world, std::vector<DrawCall>& out_draw_calls);
    inline void sort_draw_calls(std::vector<DrawCall>& draw_calls);
    inline void render_batched(const std::vector<DrawCall>& draw_calls, const Camera& camera);
    
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
    
    // Statistics
    uint32_t draw_calls_submitted_;
    uint32_t material_binds_;
    uint32_t vao_binds_;
};

// ============================================================================
// Implementation
// ============================================================================

inline StaticMeshPass::StaticMeshPass(AssetManager* asset_manager, MaterialLibrary* material_library)
    : gl_device_(nullptr)
    , asset_manager_(asset_manager)
    , material_library_(material_library)
    , viewport_width_(1280)
    , viewport_height_(720)
    , draw_calls_submitted_(0)
    , material_binds_(0)
    , vao_binds_(0)
{
}

inline bool StaticMeshPass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }

    if (!material_library_) {
        std::cerr << "[StaticMeshPass] Material library not provided" << std::endl;
        return false;
    }

    std::cout << "[StaticMeshPass] Initialized (batched rendering)" << std::endl;
    return true;
}

inline void StaticMeshPass::execute(RenderDevice* device, World* world) {
    (void)device;

    if (!gl_device_ || !asset_manager_ || !world || !material_library_) {
        return;
    }

    // Reset statistics
    draw_calls_submitted_ = 0;
    material_binds_ = 0;
    vao_binds_ = 0;

    gl_device_->push_debug_group("StaticMeshPass");

    // Build draw list
    std::vector<DrawCall> draw_calls;
    build_draw_list(world, draw_calls);

    if (draw_calls.empty()) {
        gl_device_->pop_debug_group();
        return;
    }

    // Sort for optimal batching
    sort_draw_calls(draw_calls);

    // Render batched
    const Camera& camera = world->get_camera();
    render_batched(draw_calls, camera);

    // Optional: Log statistics (can be disabled in release builds)
    #ifdef ASTRAEUS_DEBUG_BATCHING
    std::cout << "[StaticMeshPass] Rendered " << draw_calls.size() << " objects in "
              << draw_calls_submitted_ << " draw calls, "
              << material_binds_ << " material binds, "
              << vao_binds_ << " VAO binds" << std::endl;
    #endif

    gl_device_->pop_debug_group();
}

inline void StaticMeshPass::on_resize(uint32_t width, uint32_t height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

inline void StaticMeshPass::build_draw_list(World* world, std::vector<DrawCall>& out_draw_calls) {
    Material* default_material = material_library_->get_default_unlit();
    if (!default_material) {
        std::cerr << "[StaticMeshPass] No default material available" << std::endl;
        return;
    }

    const auto& entities = world->get_renderable_entities();
    out_draw_calls.reserve(entities.size());

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

        // Get color
        const Color* color = world->get_entity_color(entity_id);

        // Add to draw list
        out_draw_calls.emplace_back(entity_id, entity_id, material, transform, color);
    }
}

inline void StaticMeshPass::sort_draw_calls(std::vector<DrawCall>& draw_calls) {
    // Sort by material first, then by mesh
    // This minimizes state changes
    std::sort(draw_calls.begin(), draw_calls.end(),
              [](const DrawCall& a, const DrawCall& b) {
                  if (a.material != b.material) {
                      return a.material < b.material;
                  }
                  return a.mesh_id < b.mesh_id;
              });
}

inline void StaticMeshPass::render_batched(const std::vector<DrawCall>& draw_calls, const Camera& camera) {
    Material* current_material = nullptr;
    uint32_t current_vao = 0;

    for (const auto& call : draw_calls) {
        // Bind material if changed
        if (call.material != current_material) {
            call.material->bind(gl_device_);
            current_material = call.material;
            material_binds_++;
        }

        // Get GPU mesh
        const GPUMesh* gpu_mesh = asset_manager_->get_gpu_mesh(call.mesh_id);
        if (!gpu_mesh || !gpu_mesh->is_valid()) {
            continue;
        }

        // Bind VAO if changed
        if (gpu_mesh->vao != current_vao) {
            glBindVertexArray(gpu_mesh->vao);
            current_vao = gpu_mesh->vao;
            vao_binds_++;
        }

        // Compute and set MVP matrix
        float mvp[16];
        compute_mvp_matrix(*call.transform, camera, viewport_width_, viewport_height_, mvp);
        
        MaterialParameter mvp_param;
        mvp_param.type = MaterialParameterType::Mat4;
        std::memcpy(mvp_param.data.mat4_value, mvp, sizeof(float) * 16);
        current_material->set_parameter("mvp", mvp_param);

        // Set color
        MaterialParameter color_param;
        color_param.type = MaterialParameterType::Vec4;
        if (call.color) {
            color_param.data.vec4_value[0] = call.color->r;
            color_param.data.vec4_value[1] = call.color->g;
            color_param.data.vec4_value[2] = call.color->b;
            color_param.data.vec4_value[3] = call.color->a;
        } else {
            color_param.data.vec4_value[0] = 1.0f;
            color_param.data.vec4_value[1] = 1.0f;
            color_param.data.vec4_value[2] = 1.0f;
            color_param.data.vec4_value[3] = 1.0f;
        }
        current_material->set_parameter("baseColor", color_param);

        // Apply parameters (only updates changed uniforms)
        current_material->apply_parameters(gl_device_);

        // Draw
        if (gpu_mesh->index_count > 0) {
            glDrawElements(GL_TRIANGLES, gpu_mesh->index_count, GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, gpu_mesh->vertex_count);
        }
        
        draw_calls_submitted_++;
    }

    // Unbind VAO
    if (current_vao != 0) {
        glBindVertexArray(0);
    }
}

// ============================================================================
// Matrix math helpers
// ============================================================================

inline void StaticMeshPass::identity_matrix(float* out) {
    for (int i = 0; i < 16; i++) {
        out[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
}

inline void StaticMeshPass::translation_matrix(float x, float y, float z, float* out) {
    identity_matrix(out);
    out[12] = x;
    out[13] = y;
    out[14] = z;
}

inline void StaticMeshPass::scale_matrix(float sx, float sy, float sz, float* out) {
    identity_matrix(out);
    out[0] = sx;
    out[5] = sy;
    out[10] = sz;
}

inline void StaticMeshPass::perspective_matrix(float fov, float aspect, float near_dist, float far_dist, float* out) {
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

inline void StaticMeshPass::look_at_matrix(float eye_x, float eye_y, float eye_z,
                                     float center_x, float center_y, float center_z,
                                     float up_x, float up_y, float up_z, float* out) {
    // Forward vector
    float fx = center_x - eye_x;
    float fy = center_y - eye_y;
    float fz = center_z - eye_z;
    float f_len = math::sqrt(fx*fx + fy*fy + fz*fz);
    
    if (f_len < 1e-6f) {
        identity_matrix(out);
        return;
    }
    
    fx /= f_len; fy /= f_len; fz /= f_len;
    
    // Right vector
    float rx = fy * up_z - fz * up_y;
    float ry = fz * up_x - fx * up_z;
    float rz = fx * up_y - fy * up_x;
    float r_len = math::sqrt(rx*rx + ry*ry + rz*rz);
    
    if (r_len < 1e-6f) {
        identity_matrix(out);
        return;
    }
    
    rx /= r_len; ry /= r_len; rz /= r_len;
    
    // Up vector
    float ux = ry * fz - rz * fy;
    float uy = rz * fx - rx * fz;
    float uz = rx * fy - ry * fx;
    
    identity_matrix(out);
    out[0] = rx;  out[4] = ry;  out[8]  = rz;  out[12] = -(rx*eye_x + ry*eye_y + rz*eye_z);
    out[1] = ux;  out[5] = uy;  out[9]  = uz;  out[13] = -(ux*eye_x + uy*eye_y + uz*eye_z);
    out[2] = -fx; out[6] = -fy; out[10] = -fz; out[14] = (fx*eye_x + fy*eye_y + fz*eye_z);
    out[3] = 0;   out[7] = 0;   out[11] = 0;   out[15] = 1;
}

inline void StaticMeshPass::multiply_matrices(const float* a, const float* b, float* out) {
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

inline void StaticMeshPass::compute_mvp_matrix(const Transform& transform, const Camera& camera,
                                         uint32_t width, uint32_t height, float* out_mvp) {
    // Model matrix
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
                  0.0f, 1.0f, 0.0f, view);
    
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

#endif // ASTRAEUS_STATIC_MESH_PASS_HPP
