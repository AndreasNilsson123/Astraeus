// Compile-only test to verify RenderGraph.hpp is self-contained
// This test ensures the header can be included without requiring
// post-processing headers to be included beforehand.

#include "renderer/RenderGraph.hpp"

int main() {
    // This test only needs to compile, not run
    // If this compiles, the header is self-contained
    return 0;
}
