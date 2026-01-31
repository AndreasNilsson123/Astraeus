#ifndef ASTRAEUS_TEXTURE_HPP
#define ASTRAEUS_TEXTURE_HPP

#include <cstdint>
#include <vector>
#include <string>

namespace astraeus {

/**
 * Texture format enumeration
 */
enum class TextureFormat {
    Unknown,
    R8,          // 8-bit single channel
    RG8,         // 8-bit two channel
    RGB8,        // 8-bit RGB
    RGBA8,       // 8-bit RGBA
    R16F,        // 16-bit float single channel
    RG16F,       // 16-bit float two channel
    RGB16F,      // 16-bit float RGB
    RGBA16F,     // 16-bit float RGBA
    R32F,        // 32-bit float single channel
    RG32F,       // 32-bit float two channel
    RGB32F,      // 32-bit float RGB
    RGBA32F      // 32-bit float RGBA
};

/**
 * Texture wrapping mode
 */
enum class TextureWrap {
    Repeat,
    ClampToEdge,
    ClampToBorder,
    MirroredRepeat
};

/**
 * Texture filtering mode
 */
enum class TextureFilter {
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};

/**
 * Texture - CPU-side texture data
 */
struct Texture {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 0;              // Number of color channels (1-4)
    TextureFormat format = TextureFormat::RGBA8;
    std::vector<uint8_t> data;          // Raw pixel data
    
    // Sampling parameters
    TextureWrap wrap_s = TextureWrap::Repeat;
    TextureWrap wrap_t = TextureWrap::Repeat;
    TextureFilter min_filter = TextureFilter::Linear;
    TextureFilter mag_filter = TextureFilter::Linear;
    bool generate_mipmaps = true;
    
    /**
     * Check if texture data is valid
     */
    bool is_valid() const {
        return width > 0 && height > 0 && !data.empty();
    }
    
    /**
     * Get expected data size in bytes
     */
    size_t get_expected_size() const {
        return static_cast<size_t>(width) * height * channels;
    }
    
    /**
     * Get bytes per pixel
     */
    uint32_t get_bytes_per_pixel() const {
        switch (format) {
            case TextureFormat::R8: return 1;
            case TextureFormat::RG8: return 2;
            case TextureFormat::RGB8: return 3;
            case TextureFormat::RGBA8: return 4;
            case TextureFormat::R16F: return 2;
            case TextureFormat::RG16F: return 4;
            case TextureFormat::RGB16F: return 6;
            case TextureFormat::RGBA16F: return 8;
            case TextureFormat::R32F: return 4;
            case TextureFormat::RG32F: return 8;
            case TextureFormat::RGB32F: return 12;
            case TextureFormat::RGBA32F: return 16;
            default: return 0;
        }
    }
};

/**
 * GPU texture resource - represents a texture uploaded to the GPU
 */
struct GPUTexture {
    uint32_t texture_id = 0;            // OpenGL texture handle
    uint32_t width = 0;
    uint32_t height = 0;
    TextureFormat format = TextureFormat::RGBA8;
    uint32_t ref_count = 1;             // Reference count for sharing
    
    bool is_valid() const {
        return texture_id != 0;
    }
    
    void invalidate() {
        texture_id = 0;
        width = height = 0;
    }
};

} // namespace astraeus

#endif // ASTRAEUS_TEXTURE_HPP
