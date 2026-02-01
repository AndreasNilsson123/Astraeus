/**
 * Simple Ingest API Demo
 * 
 * Demonstrates the C API for data ingestion using sample files.
 * This is a minimal example showing:
 * - Loading a binary file
 * - Calling astraeus_ingest_data()
 * - Polling status with astraeus_get_ingest_status()
 * - Rendering entities
 */

#include "api/EngineAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper to read binary file
void* read_file(const char* filename, size_t* out_size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    void* data = malloc(size);
    if (!data) {
        fclose(file);
        return NULL;
    }
    
    size_t read = fread(data, 1, size, file);
    fclose(file);
    
    if (read != size) {
        free(data);
        return NULL;
    }
    
    *out_size = size;
    return data;
}

int main(int argc, char** argv) {
    printf("==============================================\n");
    printf("  Simple Ingest API Demo\n");
    printf("==============================================\n\n");
    
    // Check arguments
    const char* filename = "assets/sample_data_100.bin";
    if (argc > 1) {
        filename = argv[1];
    }
    
    printf("Loading file: %s\n", filename);
    
    // Read file
    size_t file_size;
    void* file_data = read_file(filename, &file_size);
    if (!file_data) {
        fprintf(stderr, "Failed to read file\n");
        return 1;
    }
    printf("File loaded: %zu bytes\n\n", file_size);
    
    // Create engine
    printf("Creating engine...\n");
    EngineConfig config;
    config.initial_width = 1280;
    config.initial_height = 720;
    config.enable_validation = false;
    config.enable_debug_output = false;
    config.log_file_path = NULL;
    
    EngineHandle engine = astraeus_create_engine(&config);
    if (!engine || !astraeus_is_valid(engine)) {
        fprintf(stderr, "Failed to create engine\n");
        free(file_data);
        return 1;
    }
    printf("Engine created\n\n");
    
    // Configure camera
    astraeus_set_camera(engine,
                       0.0f, 20.0f, 40.0f,   // eye
                       0.0f, 0.0f, 0.0f,     // target
                       0.0f, 1.0f, 0.0f);    // up
    astraeus_set_camera_projection(engine, 60.0f, 0.1f, 1000.0f);
    
    // Ingest data
    printf("Ingesting data...\n");
    uint64_t job_id = astraeus_ingest_data(engine, file_data, (uint32_t)file_size, ASTRAEUS_FORMAT_FIXED_BINARY);
    if (job_id == 0) {
        fprintf(stderr, "Failed to start ingestion\n");
        astraeus_destroy_engine(engine);
        free(file_data);
        return 1;
    }
    printf("Ingestion started: Job ID = %llu\n", (unsigned long long)job_id);
    
    // Poll for completion
    IngestStatus status;
    bool completed = false;
    int poll_count = 0;
    
    while (!completed && poll_count < 100) {
        if (astraeus_get_ingest_status(engine, job_id, &status)) {
            printf("  Progress: %u / %u bytes", status.processed_bytes, status.total_bytes);
            
            if (status.is_complete) {
                if (status.has_error) {
                    printf(" - ERROR: %s\n", status.last_error);
                    astraeus_destroy_engine(engine);
                    free(file_data);
                    return 1;
                } else {
                    printf(" - COMPLETE!\n");
                    completed = true;
                }
            } else {
                printf(" - In progress...\n");
            }
        }
        poll_count++;
    }
    
    if (!completed) {
        fprintf(stderr, "Ingestion did not complete in time\n");
        astraeus_destroy_engine(engine);
        free(file_data);
        return 1;
    }
    
    // Get ingestion statistics
    double sim_time = astraeus_get_sim_time(engine);
    uint64_t snapshot_count = astraeus_get_snapshot_count(engine);
    
    printf("\nIngestion Statistics:\n");
    printf("  Simulation Time: %.3f seconds\n", sim_time);
    printf("  Snapshots: %llu\n", (unsigned long long)snapshot_count);
    printf("\n");
    
    // Render a few frames to see the entities
    printf("Rendering frames...\n");
    for (int frame = 0; frame < 10; frame++) {
        astraeus_begin_frame(engine, 0.016); // ~60 FPS
        astraeus_end_frame(engine);
        
        // Get frame stats
        FrameStats frame_stats;
        astraeus_get_frame_stats(engine, &frame_stats);
        
        if (frame % 3 == 0) {
            printf("  Frame %d: %u entities, render time: %.2f ms\n",
                   frame, frame_stats.entity_count, frame_stats.render_time_ms);
        }
    }
    
    printf("\n");
    
    // Cleanup
    printf("Cleaning up...\n");
    astraeus_destroy_engine(engine);
    free(file_data);
    
    printf("\n✓ Demo completed successfully!\n");
    printf("\nKey API Functions Demonstrated:\n");
    printf("  - astraeus_ingest_data()       : Load simulation data\n");
    printf("  - astraeus_get_ingest_status() : Poll for completion\n");
    printf("  - astraeus_get_sim_time()      : Get simulation time\n");
    printf("  - astraeus_get_snapshot_count(): Get snapshot count\n");
    printf("\n");
    
    return 0;
}
