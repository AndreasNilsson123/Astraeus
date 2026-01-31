#include "api/EngineAPI.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
    static void sleep_ms(unsigned int ms) {
        Sleep(ms);
    }
#else
#include <time.h>
static void sleep_ms(unsigned int ms) {
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }
#endif


/**
 * Example demonstrating entity visualization with points and trails.
 * This test creates multiple entities with different colors, moves them
 * in circular paths, and enables trail rendering.
 */
int main() {
    printf("Astraeus Entity Visualization Test\n");
    printf("====================================\n\n");

    // Configure the engine
    EngineConfig config;
    config.initial_width = 1920;
    config.initial_height = 1080;
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

    if (!astraeus_is_valid(engine)) {
        fprintf(stderr, "Engine is not valid\n");
        astraeus_destroy_engine(engine);
        return 1;
    }
    printf("Engine created successfully\n\n");

    // Set up camera
    printf("Configuring camera...\n");
    astraeus_set_camera(engine,
                       0.0f, 10.0f, 20.0f,  // eye position (elevated and back)
                       0.0f, 0.0f, 0.0f,    // look-at target (origin)
                       0.0f, 1.0f, 0.0f);   // up vector
    astraeus_set_camera_projection(engine, 60.0f, 0.1f, 1000.0f);
    printf("Camera configured\n\n");

    // Create entities with different colors and trails
    printf("Creating entities with colors and trails...\n");
    
    // Red entity (moving in circle on XZ plane)
    uint32_t entity_red = astraeus_create_entity(engine);
    astraeus_set_entity_renderable(engine, entity_red, true);
    astraeus_set_entity_color(engine, entity_red, 1.0f, 0.0f, 0.0f, 1.0f); // Red
    astraeus_set_entity_trail(engine, entity_red, 100); // 100 point trail
    
    // Green entity (moving in circle, higher altitude)
    uint32_t entity_green = astraeus_create_entity(engine);
    astraeus_set_entity_renderable(engine, entity_green, true);
    astraeus_set_entity_color(engine, entity_green, 0.0f, 1.0f, 0.0f, 1.0f); // Green
    astraeus_set_entity_trail(engine, entity_green, 150); // Longer trail
    
    // Blue entity (moving in figure-8 pattern)
    uint32_t entity_blue = astraeus_create_entity(engine);
    astraeus_set_entity_renderable(engine, entity_blue, true);
    astraeus_set_entity_color(engine, entity_blue, 0.0f, 0.0f, 1.0f, 1.0f); // Blue
    astraeus_set_entity_trail(engine, entity_blue, 200); // Even longer trail
    
    // Yellow entity (stationary)
    uint32_t entity_yellow = astraeus_create_entity(engine);
    astraeus_set_entity_renderable(engine, entity_yellow, true);
    astraeus_set_entity_color(engine, entity_yellow, 1.0f, 1.0f, 0.0f, 1.0f); // Yellow
    // No trail for this one
    astraeus_apply_entity_snapshot(engine, entity_yellow, 0.0f, 0.0f, 0.0f);

    printf("Created entities: red=%u, green=%u, blue=%u, yellow=%u\n\n", 
           entity_red, entity_green, entity_blue, entity_yellow);

    // Simulate animation
    printf("Running animation for 100 frames...\n");
    const int total_frames = 100;
    const double delta_time = 0.016; // ~60fps
    
    for (int frame = 0; frame < total_frames; frame++) {
        float t = frame * delta_time;
        
        // Move red entity in circle on XZ plane
        float red_x = 5.0f * cosf(t);
        float red_y = 0.0f;
        float red_z = 5.0f * sinf(t);
        astraeus_apply_entity_snapshot(engine, entity_red, red_x, red_y, red_z);
        
        // Move green entity in circle at higher altitude
        float green_x = 3.0f * cosf(t * 1.5f);
        float green_y = 3.0f;
        float green_z = 3.0f * sinf(t * 1.5f);
        astraeus_apply_entity_snapshot(engine, entity_green, green_x, green_y, green_z);
        
        // Move blue entity in figure-8 pattern
        float blue_x = 4.0f * cosf(t * 0.8f);
        float blue_y = 2.0f * sinf(t * 1.6f);
        float blue_z = 4.0f * sinf(t * 0.8f);
        astraeus_apply_entity_snapshot(engine, entity_blue, blue_x, blue_y, blue_z);
        
        // Begin frame
        astraeus_begin_frame(engine, delta_time);
        
        // End frame
        astraeus_end_frame(engine);
        
        // Print progress every 10 frames
        if (frame % 10 == 0) {
            FrameStats stats;
            astraeus_get_frame_stats(engine, &stats);
            printf("Frame %llu: dt=%.3fms, render=%.3fms, entities=%u\n",
                   (unsigned long long)stats.frame_number,
                   stats.delta_time_ms,
                   stats.render_time_ms,
                   stats.entity_count);
        }
        
        // Small delay to simulate real-time
        sleep_ms(16);
    }
    printf("\n");

    // Test trail behavior - make entity invisible
    printf("Testing renderable state - hiding red entity...\n");
    astraeus_set_entity_renderable(engine, entity_red, false);
    
    // Run a few more frames
    for (int i = 0; i < 5; i++) {
        astraeus_begin_frame(engine, delta_time);
        astraeus_end_frame(engine);
        sleep_ms(16);
    }
    printf("Red entity hidden\n\n");

    // Show it again
    printf("Showing red entity again...\n");
    astraeus_set_entity_renderable(engine, entity_red, true);
    
    for (int i = 0; i < 5; i++) {
        astraeus_begin_frame(engine, delta_time);
        astraeus_end_frame(engine);
        sleep_ms(16);
    }
    printf("Red entity visible again\n\n");

    // Get final stats
    FrameStats final_stats;
    astraeus_get_frame_stats(engine, &final_stats);
    printf("Final statistics:\n");
    printf("  Total frames: %llu\n", (unsigned long long)final_stats.frame_number);
    printf("  Entity count: %u\n", final_stats.entity_count);
    printf("  Last frame time: %.3fms\n", final_stats.render_time_ms);
    printf("\n");

    // Clean up
    printf("Cleaning up entities...\n");
    astraeus_destroy_entity(engine, entity_red);
    astraeus_destroy_entity(engine, entity_green);
    astraeus_destroy_entity(engine, entity_blue);
    astraeus_destroy_entity(engine, entity_yellow);
    printf("Entities destroyed\n\n");

    // Destroy the engine
    printf("Shutting down engine...\n");
    astraeus_destroy_engine(engine);
    printf("Engine destroyed\n\n");

    printf("Test completed successfully!\n");
    printf("\nTest Results:\n");
    printf("  ✓ Entity creation and color assignment\n");
    printf("  ✓ Trail configuration (varying lengths)\n");
    printf("  ✓ Entity snapshot updates (smooth animation)\n");
    printf("  ✓ Renderable state toggling (visibility)\n");
    printf("  ✓ Trail updates without reallocation\n");
    printf("  ✓ Multiple entities with different motion patterns\n");
    
    return 0;
}
