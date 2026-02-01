#ifndef ASTRAEUS_MOCK_RENDER_DEVICE_HPP
#define ASTRAEUS_MOCK_RENDER_DEVICE_HPP

#include "renderer/RenderDevice.hpp"
#include <vector>
#include <string>

#include "assets/Texture.hpp"

namespace astraeus::testing {

    /**
 * Mock RenderDevice for GPU-free unit testing.
 * Records all operations for verification without requiring OpenGL context.
 */
    class MockRenderDevice : public RenderDevice {
    public:
        struct Operation {
            enum Type {
                CREATE_BUFFER,
                DELETE_BUFFER,
                CREATE_TEXTURE,
                DELETE_TEXTURE,
                CREATE_SHADER,
                DELETE_SHADER,
                CREATE_FRAMEBUFFER,
                DELETE_FRAMEBUFFER,
                SET_VIEWPORT,
                CLEAR,
                DRAW
            };

            Type type;
            std::string details;
            uint32_t id;
        };

        explicit MockRenderDevice() : RenderDevice(Config{}), next_id_(1) {
        }

        ~MockRenderDevice() override = default;

        bool initialize() override {
            initialized_ = true;
            return true;
        }

        void shutdown() override {
            initialized_ = false;
            operations_.clear();
        }

        bool is_initialized() const {
            return initialized_;
        }

        uint32_t create_buffer(size_t size, const void* data) {
            uint32_t id = next_id_++;
            operations_.push_back({Operation::CREATE_BUFFER, "size=" + std::to_string(size), id});
            return id;
        }

        void delete_buffer(uint32_t buffer_id) {
            operations_.push_back({Operation::DELETE_BUFFER, "", buffer_id});
        }

        uint32_t create_texture(uint32_t width, uint32_t height, TextureFormat format) {
            uint32_t id = next_id_++;
            operations_.push_back({Operation::CREATE_TEXTURE,
                "w=" + std::to_string(width) + ",h=" + std::to_string(height), id});
            return id;
        }

        void delete_texture(uint32_t texture_id) {
            operations_.push_back({Operation::DELETE_TEXTURE, "", texture_id});
        }

        void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
            viewport_width_ = width;
            viewport_height_ = height;
            operations_.push_back({Operation::SET_VIEWPORT,
                "w=" + std::to_string(width) + ",h=" + std::to_string(height), 0});
        }

        void clear(float r, float g, float b, float a) {
            operations_.push_back({Operation::CLEAR, "", 0});
        }

        void draw(uint32_t vertex_count) {
            operations_.push_back({Operation::DRAW, "vertices=" + std::to_string(vertex_count), 0});
        }

        // Test helpers
        const std::vector<Operation>& get_operations() const { return operations_; }
        void clear_operations() { operations_.clear(); }
        size_t operation_count() const { return operations_.size(); }

        uint32_t get_viewport_width() const { return viewport_width_; }
        uint32_t get_viewport_height() const { return viewport_height_; }

    private:
        bool initialized_ = false;
        uint32_t next_id_;
        uint32_t viewport_width_ = 0;
        uint32_t viewport_height_ = 0;
        std::vector<Operation> operations_;
    };

}

#endif // ASTRAEUS_MOCK_RENDER_DEVICE_HPP
