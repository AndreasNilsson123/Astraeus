#ifndef ASTRAEUS_PLUGIN_MANAGER_HPP
#define ASTRAEUS_PLUGIN_MANAGER_HPP

#include "Plugin.h"
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
typedef HMODULE LibraryHandle;
#else
#include <dlfcn.h>
typedef void* LibraryHandle;
#endif

namespace astraeus {

// Forward declarations
class MaterialLibrary;
class IngestManager;

/**
 * Plugin context implementation.
 * Provides access to engine services for plugins.
 */
class PluginContext {
public:
    MaterialLibrary* material_library;
    IngestManager* ingest_manager;
    
    PluginContext()
        : material_library(nullptr)
        , ingest_manager(nullptr)
    {}
};

/**
 * Loaded plugin instance.
 */
struct LoadedPlugin {
    std::string path;
    LibraryHandle library;
    PluginInfo info;
    PluginHandle handle;
    PluginShutdownFunc shutdown_func;
    PluginUpdateFunc update_func;
    
    LoadedPlugin()
        : library(nullptr)
        , handle(nullptr)
        , shutdown_func(nullptr)
        , update_func(nullptr)
    {
        std::memset(&info, 0, sizeof(info));
    }
};

/**
 * PluginManager manages loading, initialization, and lifecycle of plugins.
 * Plugins are native shared libraries with C ABI entry points.
 */
class PluginManager {
public:
    PluginManager();
    ~PluginManager();

    /**
     * Initialize plugin manager.
     * @param material_lib Material library for material plugin registration
     * @param ingest_mgr Ingest manager for decoder plugin registration
     */
    bool initialize(MaterialLibrary* material_lib, IngestManager* ingest_mgr);

    /**
     * Shutdown plugin manager and unload all plugins.
     */
    void shutdown();

    /**
     * Load a plugin from shared library.
     * @param plugin_path Path to plugin shared library
     * @return true on success, false on failure
     */
    bool load_plugin(const char* plugin_path);

    /**
     * Unload a plugin by name.
     * @param plugin_name Name of plugin to unload
     * @return true on success, false if not found
     */
    bool unload_plugin(const char* plugin_name);

    /**
     * Get plugin info by name.
     * @param plugin_name Name of plugin
     * @return Plugin info, or nullptr if not found
     */
    const PluginInfo* get_plugin_info(const char* plugin_name) const;

    /**
     * Update all plugins that have update functions.
     * Called once per frame.
     * @param delta_time Time since last frame in seconds
     */
    void update(float delta_time);

    /**
     * Get number of loaded plugins.
     */
    size_t get_plugin_count() const;

    /**
     * Get list of loaded plugin names.
     */
    std::vector<std::string> get_plugin_names() const;

private:
    PluginContext context_;
    std::unordered_map<std::string, std::unique_ptr<LoadedPlugin>> plugins_;
    bool is_initialized_;
    
    // Platform-specific library loading
    LibraryHandle load_library(const char* path);
    void* get_symbol(LibraryHandle lib, const char* symbol_name);
    void unload_library(LibraryHandle lib);
    
    // Plugin validation
    bool validate_plugin_info(const PluginInfo* info);
};

// =============================================================================
// INLINE IMPLEMENTATIONS
// =============================================================================

inline PluginManager::PluginManager()
    : is_initialized_(false) {
}

inline PluginManager::~PluginManager() {
    shutdown();
}

inline bool PluginManager::initialize(MaterialLibrary* material_lib, IngestManager* ingest_mgr) {
    if (is_initialized_) {
        return true;
    }
    
    context_.material_library = material_lib;
    context_.ingest_manager = ingest_mgr;
    
    is_initialized_ = true;
    std::cout << "[PluginManager] Initialized" << std::endl;
    return true;
}

inline void PluginManager::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    std::cout << "[PluginManager] Shutting down..." << std::endl;
    
    // Shutdown all plugins in reverse load order
    std::vector<std::string> plugin_names;
    for (const auto& pair : plugins_) {
        plugin_names.push_back(pair.first);
    }
    
    for (auto it = plugin_names.rbegin(); it != plugin_names.rend(); ++it) {
        unload_plugin(it->c_str());
    }
    
    plugins_.clear();
    is_initialized_ = false;
    
    std::cout << "[PluginManager] Shutdown complete" << std::endl;
}

inline bool PluginManager::load_plugin(const char* plugin_path) {
    if (!is_initialized_ || !plugin_path) {
        std::cerr << "[PluginManager] Cannot load plugin: not initialized or invalid path" << std::endl;
        return false;
    }
    
    std::cout << "[PluginManager] Loading plugin: " << plugin_path << std::endl;
    
    // Load library
    LibraryHandle lib = load_library(plugin_path);
    if (!lib) {
        std::cerr << "[PluginManager] Failed to load library: " << plugin_path << std::endl;
        return false;
    }
    
    // Get init function
    auto init_func = reinterpret_cast<PluginInitFunc>(get_symbol(lib, "astraeus_plugin_init"));
    if (!init_func) {
        std::cerr << "[PluginManager] Plugin missing astraeus_plugin_init: " << plugin_path << std::endl;
        unload_library(lib);
        return false;
    }
    
    // Get shutdown function
    auto shutdown_func = reinterpret_cast<PluginShutdownFunc>(get_symbol(lib, "astraeus_plugin_shutdown"));
    if (!shutdown_func) {
        std::cerr << "[PluginManager] Plugin missing astraeus_plugin_shutdown: " << plugin_path << std::endl;
        unload_library(lib);
        return false;
    }
    
    // Optional: get update function
    auto update_func = reinterpret_cast<PluginUpdateFunc>(get_symbol(lib, "astraeus_plugin_update"));
    
    // Initialize plugin
    PluginInfo info;
    std::memset(&info, 0, sizeof(info));
    PluginHandle handle = init_func(&info, reinterpret_cast<PluginContextHandle>(&context_));
    
    if (!handle) {
        std::cerr << "[PluginManager] Plugin initialization failed: " << plugin_path << std::endl;
        unload_library(lib);
        return false;
    }
    
    // Validate plugin info
    if (!validate_plugin_info(&info)) {
        std::cerr << "[PluginManager] Plugin validation failed: " << plugin_path << std::endl;
        shutdown_func(handle);
        unload_library(lib);
        return false;
    }
    
    // Check for duplicate
    std::string plugin_name(info.name);
    if (plugins_.find(plugin_name) != plugins_.end()) {
        std::cerr << "[PluginManager] Plugin already loaded: " << plugin_name << std::endl;
        shutdown_func(handle);
        unload_library(lib);
        return false;
    }
    
    // Create loaded plugin
    auto loaded = std::make_unique<LoadedPlugin>();
    loaded->path = plugin_path;
    loaded->library = lib;
    loaded->info = info;
    loaded->handle = handle;
    loaded->shutdown_func = shutdown_func;
    loaded->update_func = update_func;
    
    plugins_[plugin_name] = std::move(loaded);
    
    std::cout << "[PluginManager] Loaded plugin: " << plugin_name 
              << " v" << info.version_major << "." << info.version_minor << "." << info.version_patch
              << " (" << info.description << ")" << std::endl;
    
    return true;
}

inline bool PluginManager::unload_plugin(const char* plugin_name) {
    if (!plugin_name) {
        return false;
    }
    
    auto it = plugins_.find(plugin_name);
    if (it == plugins_.end()) {
        return false;
    }
    
    std::cout << "[PluginManager] Unloading plugin: " << plugin_name << std::endl;
    
    LoadedPlugin* plugin = it->second.get();
    
    // Shutdown plugin
    if (plugin->shutdown_func && plugin->handle) {
        plugin->shutdown_func(plugin->handle);
    }
    
    // Unload library
    if (plugin->library) {
        unload_library(plugin->library);
    }
    
    plugins_.erase(it);
    return true;
}

inline const PluginInfo* PluginManager::get_plugin_info(const char* plugin_name) const {
    if (!plugin_name) {
        return nullptr;
    }
    
    auto it = plugins_.find(plugin_name);
    if (it == plugins_.end()) {
        return nullptr;
    }
    
    return &it->second->info;
}

inline void PluginManager::update(float delta_time) {
    for (const auto& pair : plugins_) {
        LoadedPlugin* plugin = pair.second.get();
        if (plugin->update_func && plugin->handle) {
            plugin->update_func(plugin->handle, delta_time);
        }
    }
}

inline size_t PluginManager::get_plugin_count() const {
    return plugins_.size();
}

inline std::vector<std::string> PluginManager::get_plugin_names() const {
    std::vector<std::string> names;
    names.reserve(plugins_.size());
    
    for (const auto& pair : plugins_) {
        names.push_back(pair.first);
    }
    
    return names;
}

inline LibraryHandle PluginManager::load_library(const char* path) {
#ifdef _WIN32
    return LoadLibraryA(path);
#else
    return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
#endif
}

inline void* PluginManager::get_symbol(LibraryHandle lib, const char* symbol_name) {
#ifdef _WIN32
    return reinterpret_cast<void*>(GetProcAddress(lib, symbol_name));
#else
    return dlsym(lib, symbol_name);
#endif
}

inline void PluginManager::unload_library(LibraryHandle lib) {
#ifdef _WIN32
    FreeLibrary(lib);
#else
    dlclose(lib);
#endif
}

inline bool PluginManager::validate_plugin_info(const PluginInfo* info) {
    if (!info) {
        return false;
    }
    
    // Check API version
    if (info->api_version != ASTRAEUS_PLUGIN_API_VERSION) {
        std::cerr << "[PluginManager] Plugin API version mismatch: expected "
                  << ASTRAEUS_PLUGIN_API_VERSION << ", got " << info->api_version << std::endl;
        return false;
    }
    
    // Check name
    if (info->name[0] == '\0') {
        std::cerr << "[PluginManager] Plugin has no name" << std::endl;
        return false;
    }
    
    return true;
}

} // namespace astraeus

// =============================================================================
// C API IMPLEMENTATIONS
// =============================================================================

extern "C" {

RegisterMaterialFunc plugin_context_get_material_registrar(PluginContextHandle ctx) {
    // TODO: Return actual material registration function
    // This will be implemented when material system supports dynamic registration
    return nullptr;
}

RegisterIngestDecoderFunc plugin_context_get_ingest_registrar(PluginContextHandle ctx) {
    // TODO: Return actual ingest decoder registration function
    // This will be implemented when ingest system supports dynamic registration
    return nullptr;
}

void plugin_context_log(PluginContextHandle ctx, const char* message) {
    if (message) {
        std::cout << "[Plugin] " << message << std::endl;
    }
}

} // extern "C"

#endif // ASTRAEUS_PLUGIN_MANAGER_HPP
