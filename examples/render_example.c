#include "engine/api/EngineAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

/**
 * Save RGBA buffer to PNG file
 */
int save_png(const char* filename, const uint8_t* data, uint32_t width, uint32_t height) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return 0;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        return 0;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, NULL);
        fclose(fp);
        return 0;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return 0;
    }

    png_init_io(png, fp);
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    // Write image data row by row (flip Y)
    for (uint32_t y = 0; y < height; y++) {
        uint32_t row = height - 1 - y; // Flip Y
        png_write_row(png, (png_bytep)(data + row * width * 4));
    }

    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);

    return 1;
}

/**
 * Animated rendering example that saves frames to disk.
 */
int main() {
    printf("Astraeus Animated Rendering Example\n");
    printf("====================================\n\n");

    // Configure the engine
    EngineConfig config;
    config.initial_width = 800;
    config.initial_height = 600;
    config.enable_validation = true;
    config.enable_debug_output = false;
    config.log_file_path = NULL;

    // Create the engine
    printf("Creating engine...\n");
    EngineHandle engine = astraeus_create_engine(&config);
    if (!engine) {
        fprintf(stderr, "Failed to create engine\n");
        return 1;
    }

    printf("Engine created successfully\n\n");

    // Set up camera for a nice view of the triangle
    printf("Configuring camera...\n");
    astraeus_set_camera(engine,
                       0.0f, 0.0f, 3.0f,   // eye position (back from origin)
                       0.0f, 0.0f, 0.0f,   // look-at target
                       0.0f, 1.0f, 0.0f);  // up vector
    astraeus_set_camera_projection(engine, 60.0f, 0.1f, 100.0f);
    printf("Camera configured\n\n");

    // Render frames
    printf("Rendering %d frames...\n", 60);
    for (int frame = 0; frame < 60; frame++) {
        double delta_time = 1.0 / 60.0; // 60fps

        // Begin frame
        astraeus_begin_frame(engine, delta_time);

        // End frame (rendering happens here via RenderGraph)
        astraeus_end_frame(engine);

        // Get frame stats
        FrameStats stats;
        astraeus_get_frame_stats(engine, &stats);

        // Save a few keyframes
        if (frame == 0 || frame == 15 || frame == 30 || frame == 45 || frame == 59) {
            // Get color buffer
            PixelBufferView color_buffer = astraeus_get_color_buffer(engine);
            
            if (color_buffer.data) {
                char filename[64];
                snprintf(filename, sizeof(filename), "frame_%03d.png", frame);
                
                if (save_png(filename, (const uint8_t*)color_buffer.data, 
                            color_buffer.width, color_buffer.height)) {
                    printf("  Frame %d: Saved %s (%.2fms render, %u draw calls, %u triangles)\n",
                           frame, filename,
                           stats.render_time_ms,
                           stats.draw_calls,
                           stats.triangle_count);
                } else {
                    fprintf(stderr, "  Frame %d: Failed to save %s\n", frame, filename);
                }
            }
        }
    }
    printf("\n");

    printf("Final statistics:\n");
    FrameStats final_stats;
    astraeus_get_frame_stats(engine, &final_stats);
    printf("  Total frames: %lu\n", final_stats.frame_number);
    printf("  Last frame render time: %.2fms\n", final_stats.render_time_ms);
    printf("  Draw calls per frame: %u\n", final_stats.draw_calls);
    printf("  Triangles per frame: %u\n", final_stats.triangle_count);
    printf("\n");

    // Clean up
    printf("Shutting down engine...\n");
    astraeus_destroy_engine(engine);
    printf("Engine destroyed\n\n");

    printf("Rendering complete! Check frame_*.png files.\n");
    return 0;
}
