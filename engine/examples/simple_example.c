#include "api/EngineAPI.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
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
 * Simple example demonstrating the Astraeus engine C API.
 */
int main() {
    printf("Astraeus Engine Example\n");
    printf("========================\n\n");

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

    // Verify engine is valid
    if (!astraeus_is_valid(engine)) {
        fprintf(stderr, "Engine is not valid\n");
        astraeus_destroy_engine(engine);
        return 1;
    }
    printf("Engine created successfully\n\n");

    // Set up camera
    printf("Configuring camera...\n");
    astraeus_set_camera(engine,
                       0.0f, 5.0f, 10.0f,  // eye position
                       0.0f, 0.0f, 0.0f,   // look-at target
                       0.0f, 1.0f, 0.0f);  // up vector
    astraeus_set_camera_projection(engine, 60.0f, 0.1f, 1000.0f);
    printf("Camera configured\n\n");

    // Create some entities
    printf("Creating entities...\n");
    uint32_t entity1 = astraeus_create_entity(engine);
    astraeus_set_entity_transform(engine, entity1,
                                 0.0f, 0.0f, 0.0f,    // position
                                 0.0f, 0.0f, 0.0f,    // rotation
                                 1.0f, 1.0f, 1.0f);   // scale

    uint32_t entity2 = astraeus_create_entity(engine);
    astraeus_set_entity_transform(engine, entity2,
                                 5.0f, 0.0f, 0.0f,    // position
                                 0.0f, 0.0f, 0.0f,    // rotation
                                 1.0f, 1.0f, 1.0f);   // scale

    uint32_t entity3 = astraeus_create_entity(engine);
    astraeus_set_entity_transform(engine, entity3,
                                 -5.0f, 0.0f, 0.0f,   // position
                                 0.0f, 0.0f, 0.0f,    // rotation
                                 1.0f, 1.0f, 1.0f);   // scale

    printf("Created entities: %u, %u, %u\n\n", entity1, entity2, entity3);

    // Simulate a few frames
    printf("Running simulation for 10 frames...\n");
    for (int i = 0; i < 10; i++) {
        double delta_time = 0.016; // ~60fps

        // Begin frame
        astraeus_begin_frame(engine, delta_time);

        // Rendering would happen here in a real application
        // For now, we just end the frame

        // End frame
        astraeus_end_frame(engine);

        // Get frame stats
        FrameStats stats;
        astraeus_get_frame_stats(engine, &stats);

        printf("dt=%.3fms, render=%.3fms, entities=%u\n",
               stats.delta_time_ms,
               stats.render_time_ms,
               stats.entity_count);

        // Small delay to simulate real-time
        sleep_ms(16);
    }
    printf("\n");

    // Test picking (will return no hit since we have no actual rendering)
    printf("Testing picking at screen coordinates (960, 540)...\n");
    PickResult pick_result;
    astraeus_pick(engine, 960, 540, &pick_result);
    if (pick_result.hit) {
        printf("Hit entity %u at depth %.3f, world position (%.2f, %.2f, %.2f)\n",
               pick_result.entity_id,
               pick_result.depth,
               pick_result.world_x,
               pick_result.world_y,
               pick_result.world_z);
    } else {
        printf("No hit (expected - no rendering backend implemented yet)\n");
    }
    printf("\n");

    // Test resizing
    printf("Resizing viewport to 2560x1440...\n");
    astraeus_resize_viewport(engine, 2560, 1440);
    printf("Viewport resized\n\n");

    // Clean up one entity
    printf("Destroying entity %u...\n", entity2);
    astraeus_destroy_entity(engine, entity2);
    printf("Entity destroyed\n\n");

    // Destroy the engine
    printf("Shutting down engine...\n");
    astraeus_destroy_engine(engine);
    printf("Engine destroyed\n\n");

    printf("Example completed successfully!\n");
    return 0;
}
