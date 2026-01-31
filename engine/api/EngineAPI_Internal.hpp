#pragma once
#include "EngineAPI.h"
#include "core/EngineContext.hpp"
#include <memory>
#include <cstring>

struct AstraeusEngine {
    std::unique_ptr<astraeus::EngineContext> context;
    FrameStats current_stats;
    ViewportConfig viewport;
    bool is_initialized;

    AstraeusEngine() : is_initialized(false) {
        std::memset(&current_stats, 0, sizeof(FrameStats));
        std::memset(&viewport, 0, sizeof(ViewportConfig));
    }
};

static inline bool is_valid_engine(EngineHandle engine) {
    return engine != nullptr && engine->is_initialized && engine->context != nullptr;
}
