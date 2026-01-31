// Compile this file exactly once in the whole build.

#define TINYGLTF_IMPLEMENTATION

// Make tinygltf NOT try to include stb itself (we include it explicitly)
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#include "tiny_gltf.h"
