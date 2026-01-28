#ifndef ASTRAEUS_NULL_CONTEXT_HPP
#define ASTRAEUS_NULL_CONTEXT_HPP

#include <iostream>
#include <cstdint>

namespace astraeus {

// Forward declare GraphicsContext to avoid circular dependency
class GraphicsContext;

/**
 * Null graphics context - provides no actual OpenGL context.
 * Used as a fallback when no backend is available or for testing.
 */
class NullContext : public GraphicsContext {
public:
    NullContext() = default;
    ~NullContext() override = default;

    inline bool initialize(uint32_t width, uint32_t height) override {
        std::cerr << "[NullContext] Warning: Using null graphics context (no rendering will occur)" << std::endl;
        std::cerr << "[NullContext] Requested size: " << width << "x" << height << std::endl;
        return false;  // Fail to prevent further OpenGL calls
    }

    inline void shutdown() override {
        // Nothing to do
    }

    inline bool make_current() override {
        return false;
    }

    inline void* get_proc_address(const char* name) override {
        (void)name;
        return nullptr;
    }

    const char* get_backend_name() const override { return "Null (no backend)"; }
};

} // namespace astraeus

#endif // ASTRAEUS_NULL_CONTEXT_HPP
