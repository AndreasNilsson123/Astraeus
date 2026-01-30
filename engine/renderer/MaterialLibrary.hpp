#ifndef ASTRAEUS_MATERIAL_LIBRARY_HPP
#define ASTRAEUS_MATERIAL_LIBRARY_HPP

#include "Material.hpp"
#include "UnlitMaterial.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <iostream>

namespace astraeus {

/**
 * MaterialLibrary - Manages materials and material instances.
 * Provides centralized material registration and lookup.
 */
class MaterialLibrary {
public:
    MaterialLibrary() = default;
    ~MaterialLibrary() = default;

    /**
     * Initialize the library with default materials
     */
    bool initialize(RenderDevice* device) {
        device_ = device;
        
        // Create default unlit material
        auto unlit = std::make_unique<UnlitMaterial>();
        if (!unlit->initialize(device)) {
            std::cerr << "[MaterialLibrary] Failed to initialize default unlit material" << std::endl;
            return false;
        }
        
        register_material("unlit", std::move(unlit));
        std::cout << "[MaterialLibrary] Initialized with default materials" << std::endl;
        return true;
    }

    /**
     * Shutdown the library and cleanup materials
     */
    void shutdown() {
        // Shutdown all materials
        for (auto& pair : materials_) {
            if (pair.second) {
                pair.second->shutdown();
            }
        }
        
        materials_.clear();
        device_ = nullptr;
    }

    /**
     * Register a material with a unique name
     */
    void register_material(const std::string& name, std::unique_ptr<Material> material) {
        if (materials_.find(name) != materials_.end()) {
            std::cerr << "[MaterialLibrary] Material '" << name << "' already registered" << std::endl;
            return;
        }
        
        materials_[name] = std::move(material);
        std::cout << "[MaterialLibrary] Registered material: " << name << std::endl;
    }

    /**
     * Get a material by name
     */
    Material* get_material(const std::string& name) {
        auto it = materials_.find(name);
        if (it != materials_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    /**
     * Create a material instance from a base material
     */
    std::unique_ptr<MaterialInstance> create_instance(const std::string& material_name) {
        Material* base = get_material(material_name);
        if (!base) {
            std::cerr << "[MaterialLibrary] Material '" << material_name << "' not found" << std::endl;
            return nullptr;
        }
        
        return std::make_unique<MaterialInstance>(base);
    }

    /**
     * Get the default unlit material
     */
    Material* get_default_unlit() {
        return get_material("unlit");
    }

private:
    RenderDevice* device_ = nullptr;
    std::unordered_map<std::string, std::unique_ptr<Material>> materials_;
};

} // namespace astraeus

#endif // ASTRAEUS_MATERIAL_LIBRARY_HPP
