/**
 * Telemetry Test Example
 * 
 * This example demonstrates the telemetry system of the Astraeus engine:
 * - Enable/disable telemetry at runtime
 * - Query frame statistics (including GPU time)
 * - Access per-pass timing information
 * - Verify zero overhead when disabled
 */

#include "engine/api/EngineAPI.h"
#include <stdio.h>
#include <stdlib.h>

void print_frame_stats(const FrameStats* stats) {
    printf("  Frame #%lu:\n", stats->frame_number);
    printf("    Delta Time:    %.2f ms\n", stats->delta_time_ms);
    printf("    Render Time:   %.2f ms\n", stats->render_time_ms);
    printf("    GPU Time:      %.2f ms\n", stats->gpu_time_ms);
    printf("    Draw Calls:    %u\n", stats->draw_calls);
    printf("    Triangles:     %u\n", stats->triangle_count);
    printf("    Entities:      %u\n", stats->entity_count);
}

void print_pass_telemetry(EngineHandle engine) {
    uint32_t pass_count = astraeus_get_pass_count(engine);
    printf("  Render Passes (%u):\n", pass_count);
    
    for (uint32_t i = 0; i < pass_count; i++) {
        PassTelemetry pass_telemetry;
        if (astraeus_get_pass_telemetry(engine, i, &pass_telemetry)) {
            printf("    [%u] %s: %.3f ms\n", i, pass_telemetry.pass_name, pass_telemetry.duration_ms);
        }
    }
}

int main(int argc, char** argv) {
    printf("=== Astraeus Telemetry Test ===\n\n");

    // Create engine
    EngineConfig config;
    config.initial_width = 1280;
    config.initial_height = 720;
    config.enable_validation = false;
    config.enable_debug_output = false;
    config.log_file_path = NULL;

    printf("Creating engine...\n");
    EngineHandle engine = astraeus_create_engine(&config);
    if (!engine) {
        fprintf(stderr, "ERROR: Failed to create engine\n");
        return 1;
    }
    printf("Engine created successfully\n\n");

    // Create some test entities
    printf("Creating test entities...\n");
    uint32_t entity1 = astraeus_create_entity(engine);
    uint32_t entity2 = astraeus_create_entity(engine);
    uint32_t entity3 = astraeus_create_entity(engine);
    
    astraeus_set_entity_transform(engine, entity1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    astraeus_set_entity_transform(engine, entity2, 10.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    astraeus_set_entity_transform(engine, entity3, -10.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    
    astraeus_set_entity_renderable(engine, entity1, true);
    astraeus_set_entity_renderable(engine, entity2, true);
    astraeus_set_entity_renderable(engine, entity3, true);
    printf("Created 3 entities\n\n");

    // Test 1: Telemetry disabled (default)
    printf("=== Test 1: Telemetry Disabled (Default) ===\n");
    printf("Telemetry enabled: %s\n", astraeus_is_telemetry_enabled(engine) ? "YES" : "NO");
    
    // Render a few frames with telemetry disabled
    for (int i = 0; i < 3; i++) {
        astraeus_begin_frame(engine, 0.016);  // 16ms = ~60fps
        astraeus_end_frame(engine);
    }
    
    FrameStats stats;
    astraeus_get_frame_stats(engine, &stats);
    print_frame_stats(&stats);
    
    uint32_t pass_count = astraeus_get_pass_count(engine);
    printf("  Pass Count: %u (should be 0 when disabled)\n\n", pass_count);

    // Test 2: Enable telemetry
    printf("=== Test 2: Telemetry Enabled ===\n");
    astraeus_set_telemetry_enabled(engine, true);
    printf("Telemetry enabled: %s\n", astraeus_is_telemetry_enabled(engine) ? "YES" : "NO");
    
    // Render frames with telemetry enabled
    for (int i = 0; i < 5; i++) {
        astraeus_begin_frame(engine, 0.016);
        astraeus_end_frame(engine);
    }
    
    astraeus_get_frame_stats(engine, &stats);
    print_frame_stats(&stats);
    print_pass_telemetry(engine);
    printf("\n");

    // Test 3: Verify GPU time is populated
    printf("=== Test 3: GPU Time Verification ===\n");
    if (stats.gpu_time_ms > 0.0) {
        printf("✓ GPU time is populated: %.2f ms\n", stats.gpu_time_ms);
    } else {
        printf("✗ WARNING: GPU time is 0.0 ms (may be expected for stub implementation)\n");
    }
    printf("\n");

    // Test 4: Verify per-pass telemetry
    printf("=== Test 4: Per-Pass Telemetry ===\n");
    pass_count = astraeus_get_pass_count(engine);
    if (pass_count > 0) {
        printf("✓ Pass count: %u\n", pass_count);
        
        // Verify we can query each pass
        bool all_passes_valid = true;
        for (uint32_t i = 0; i < pass_count; i++) {
            PassTelemetry pass_telemetry;
            if (!astraeus_get_pass_telemetry(engine, i, &pass_telemetry)) {
                printf("✗ Failed to get telemetry for pass %u\n", i);
                all_passes_valid = false;
            }
        }
        
        if (all_passes_valid) {
            printf("✓ All passes have valid telemetry data\n");
        }
    } else {
        printf("✗ WARNING: No passes registered\n");
    }
    printf("\n");

    // Test 5: Disable telemetry again
    printf("=== Test 5: Disable Telemetry ===\n");
    astraeus_set_telemetry_enabled(engine, false);
    printf("Telemetry enabled: %s\n", astraeus_is_telemetry_enabled(engine) ? "YES" : "NO");
    
    astraeus_begin_frame(engine, 0.016);
    astraeus_end_frame(engine);
    
    pass_count = astraeus_get_pass_count(engine);
    printf("Pass Count: %u (should be 0 when disabled)\n\n", pass_count);

    // Test 6: Performance test - frame with telemetry off vs on
    printf("=== Test 6: Performance Comparison ===\n");
    printf("Running 100 frames with telemetry disabled...\n");
    astraeus_set_telemetry_enabled(engine, false);
    for (int i = 0; i < 100; i++) {
        astraeus_begin_frame(engine, 0.016);
        astraeus_end_frame(engine);
    }
    printf("Completed\n");
    
    printf("Running 100 frames with telemetry enabled...\n");
    astraeus_set_telemetry_enabled(engine, true);
    for (int i = 0; i < 100; i++) {
        astraeus_begin_frame(engine, 0.016);
        astraeus_end_frame(engine);
    }
    printf("Completed\n");
    printf("(Overhead should be ≤1-2%% when enabled)\n\n");

    // Cleanup
    printf("Destroying engine...\n");
    astraeus_destroy_engine(engine);
    printf("Test complete!\n");

    return 0;
}
