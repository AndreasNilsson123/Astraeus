#ifndef ASTRAEUS_MESH_LOADER_HPP
#define ASTRAEUS_MESH_LOADER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../geometry/Mesh.hpp"
#include "../core/util/SafeC.hpp"

namespace astraeus {

/**
 * MeshLoader provides simple mesh loading from OBJ files.
 * Supports basic OBJ format with vertices, normals, and texture coordinates.
 */
class MeshLoader {
public:
    /**
     * Load a mesh from an OBJ file.
     * @param filepath Path to the OBJ file
     * @param out_mesh Output mesh object
     * @return true on success, false on failure
     */
    static bool load_obj(const std::string& filepath, Mesh& out_mesh) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[MeshLoader] Failed to open file: " << filepath << std::endl;
            return false;
        }

        std::vector<float> positions;   // Temporary position data (x, y, z)
        std::vector<float> normals;     // Temporary normal data (nx, ny, nz)
        std::vector<float> texcoords;   // Temporary texcoord data (u, v)
        
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        std::string line;
        uint32_t vertex_count = 0;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue; // Skip empty lines and comments
            }

            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                // Vertex position
                float x, y, z;
                iss >> x >> y >> z;
                positions.push_back(x);
                positions.push_back(y);
                positions.push_back(z);
            }
            else if (prefix == "vn") {
                // Vertex normal
                float nx, ny, nz;
                iss >> nx >> ny >> nz;
                normals.push_back(nx);
                normals.push_back(ny);
                normals.push_back(nz);
            }
            else if (prefix == "vt") {
                // Texture coordinate
                float u, v;
                iss >> u >> v;
                texcoords.push_back(u);
                texcoords.push_back(v);
            }
            else if (prefix == "f") {
                // Face (triangle)
                // Supports formats: f v1 v2 v3 or f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
                std::string vertex_str;
                std::vector<uint32_t> face_indices;

                while (iss >> vertex_str) {
                    int v_idx = 0, vt_idx = 0, vn_idx = 0;
                    
                    // Parse vertex indices (v/vt/vn format) using safe parser
                    if (vertex_str.find('/') != std::string::npos) {
                        util::parse_obj_vertex(vertex_str.c_str(), &v_idx, &vt_idx, &vn_idx);
                    } else {
                        v_idx = std::stoi(vertex_str);
                    }

                    // OBJ indices are 1-based, convert to 0-based
                    if (v_idx < 0) v_idx = static_cast<int>(positions.size() / 3) + v_idx + 1;
                    if (vt_idx < 0) vt_idx = static_cast<int>(texcoords.size() / 2) + vt_idx + 1;
                    if (vn_idx < 0) vn_idx = static_cast<int>(normals.size() / 3) + vn_idx + 1;

                    // Create vertex
                    Vertex vertex;
                    
                    // Position (required)
                    if (v_idx > 0 && v_idx <= static_cast<int>(positions.size() / 3)) {
                        int pos_idx = (v_idx - 1) * 3;
                        vertex.x = positions[pos_idx + 0];
                        vertex.y = positions[pos_idx + 1];
                        vertex.z = positions[pos_idx + 2];
                    } else {
                        vertex.x = vertex.y = vertex.z = 0.0f;
                    }

                    // Normal (optional)
                    if (vn_idx > 0 && vn_idx <= static_cast<int>(normals.size() / 3)) {
                        int norm_idx = (vn_idx - 1) * 3;
                        vertex.nx = normals[norm_idx + 0];
                        vertex.ny = normals[norm_idx + 1];
                        vertex.nz = normals[norm_idx + 2];
                    } else {
                        // Default normal (up)
                        vertex.nx = 0.0f;
                        vertex.ny = 1.0f;
                        vertex.nz = 0.0f;
                    }

                    // Texture coordinates (optional)
                    if (vt_idx > 0 && vt_idx <= static_cast<int>(texcoords.size() / 2)) {
                        int tex_idx = (vt_idx - 1) * 2;
                        vertex.u = texcoords[tex_idx + 0];
                        vertex.v = texcoords[tex_idx + 1];
                    } else {
                        vertex.u = vertex.v = 0.0f;
                    }

                    vertices.push_back(vertex);
                    face_indices.push_back(vertex_count++);
                }

                // Triangulate if more than 3 vertices (simple fan triangulation)
                if (face_indices.size() >= 3) {
                    for (size_t i = 1; i < face_indices.size() - 1; i++) {
                        indices.push_back(face_indices[0]);
                        indices.push_back(face_indices[i]);
                        indices.push_back(face_indices[i + 1]);
                    }
                }
            }
        }

        file.close();

        if (vertices.empty()) {
            std::cerr << "[MeshLoader] No vertices loaded from: " << filepath << std::endl;
            return false;
        }

        // Set mesh data
        out_mesh.set_vertices(vertices);
        out_mesh.set_indices(indices);

        std::cout << "[MeshLoader] Loaded mesh: " << filepath 
                  << " (" << vertices.size() << " vertices, " 
                  << indices.size() / 3 << " triangles)" << std::endl;

        return true;
    }
};

} // namespace astraeus

#endif // ASTRAEUS_MESH_LOADER_HPP
