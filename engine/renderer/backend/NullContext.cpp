#include "NullContext.hpp"
#include <iostream>

namespace astraeus {

bool NullContext::initialize(uint32_t width, uint32_t height) {
    std::cerr << "[NullContext] Warning: Using null graphics context (no rendering will occur)" << std::endl;
    std::cerr << "[NullContext] Requested size: " << width << "x" << height << std::endl;
    return false;  // Fail to prevent further OpenGL calls
}

void NullContext::shutdown() {
    // Nothing to do
}

bool NullContext::make_current() {
    return false;
}

void* NullContext::get_proc_address(const char* name) {
    (void)name;
    return nullptr;
}

} // namespace astraeus
