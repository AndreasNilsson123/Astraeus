#ifndef ASTRAEUS_NULL_CONTEXT_HPP
#define ASTRAEUS_NULL_CONTEXT_HPP

#include "GraphicsContext.hpp"

namespace astraeus {

/**
 * Null graphics context - provides no actual OpenGL context.
 * Used as a fallback when no backend is available or for testing.
 */
class NullContext : public GraphicsContext {
public:
    NullContext() = default;
    ~NullContext() override = default;

    bool initialize(uint32_t width, uint32_t height) override;
    void shutdown() override;
    bool make_current() override;
    void* get_proc_address(const char* name) override;
    const char* get_backend_name() const override { return "Null (no backend)"; }
};

} // namespace astraeus

#endif // ASTRAEUS_NULL_CONTEXT_HPP
