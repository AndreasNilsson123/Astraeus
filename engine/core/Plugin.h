#ifndef ASTRAEUS_PLUGIN_H
#define ASTRAEUS_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * Plugin API version.
 * Plugins must match this version to load.
 */
#define ASTRAEUS_PLUGIN_API_VERSION 1

/**
 * Plugin types.
 */
typedef enum {
    PLUGIN_TYPE_MATERIAL = 0,     // Material shader plugin
    PLUGIN_TYPE_INGEST = 1,       // Data ingest decoder plugin
    PLUGIN_TYPE_RENDER_PASS = 2,  // Custom render pass plugin
    PLUGIN_TYPE_TOOL = 3          // Tool/utility plugin
} PluginType;

/**
 * Plugin info structure.
 * Filled by plugin during initialization.
 */
typedef struct {
    uint32_t api_version;          // Must be ASTRAEUS_PLUGIN_API_VERSION
    PluginType plugin_type;         // Type of plugin
    char name[64];                  // Plugin name
    char description[256];          // Plugin description
    uint32_t version_major;         // Plugin version major
    uint32_t version_minor;         // Plugin version minor
    uint32_t version_patch;         // Plugin version patch
    char author[128];               // Plugin author
} PluginInfo;

/**
 * Plugin context (opaque handle).
 * Passed to plugin functions for accessing engine services.
 */
typedef struct PluginContext* PluginContextHandle;

/**
 * Plugin handle (opaque).
 * Returned by plugin initialization, passed to other functions.
 */
typedef void* PluginHandle;

/**
 * Material registration function type.
 * Called by material plugins to register new material types.
 */
typedef bool (*RegisterMaterialFunc)(PluginContextHandle ctx,
                                      const char* material_name,
                                      const char* vertex_shader,
                                      const char* fragment_shader);

/**
 * Ingest decoder registration function type.
 * Called by ingest plugins to register new data formats.
 */
typedef bool (*RegisterIngestDecoderFunc)(PluginContextHandle ctx,
                                           uint32_t format_id,
                                           const char* format_name);

/**
 * Plugin initialization function.
 * Every plugin must export this function.
 * 
 * @param out_info Plugin info to fill
 * @param context Plugin context for accessing engine
 * @return Plugin handle on success, NULL on failure
 */
typedef PluginHandle (*PluginInitFunc)(PluginInfo* out_info, PluginContextHandle context);

/**
 * Plugin shutdown function.
 * Every plugin must export this function.
 * 
 * @param handle Plugin handle from init
 */
typedef void (*PluginShutdownFunc)(PluginHandle handle);

/**
 * Plugin update function (optional).
 * Called once per frame if plugin needs updates.
 * 
 * @param handle Plugin handle from init
 * @param delta_time Time since last frame in seconds
 */
typedef void (*PluginUpdateFunc)(PluginHandle handle, float delta_time);

/**
 * Material plugin: Execute material shader.
 * Called during rendering to apply material.
 * 
 * @param handle Plugin handle
 * @param material_id Material instance ID
 * @param entity_id Entity being rendered
 */
typedef void (*MaterialExecuteFunc)(PluginHandle handle, uint32_t material_id, uint32_t entity_id);

/**
 * Ingest plugin: Decode data.
 * Called to decode ingested data.
 * 
 * @param handle Plugin handle
 * @param data Data buffer
 * @param size Data size in bytes
 * @param out_entity_count Output: number of entities decoded
 * @return true on success, false on failure
 */
typedef bool (*IngestDecodeFunc)(PluginHandle handle, 
                                  const void* data, 
                                  uint32_t size,
                                  uint32_t* out_entity_count);

/**
 * Get plugin context functions.
 * Plugin uses these to access engine services.
 */

// Get material registration function from context
RegisterMaterialFunc plugin_context_get_material_registrar(PluginContextHandle ctx);

// Get ingest decoder registration function from context
RegisterIngestDecoderFunc plugin_context_get_ingest_registrar(PluginContextHandle ctx);

// Log message from plugin
void plugin_context_log(PluginContextHandle ctx, const char* message);

#ifdef __cplusplus
}
#endif

#endif // ASTRAEUS_PLUGIN_H
