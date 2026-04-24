#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
namespace engine {

class ShaderProgram;
class Material;
class Mesh;
class GraphicsApi {
public:
    std::shared_ptr<ShaderProgram> create_shader_program(const std::string& vertex_source,
                                                         const std::string& fragment_source);
    void bind_shader_program(ShaderProgram* pshader_program);
    void bind_material(Material* pmaterial);
    uint32_t create_vertex_buffer(const std::vector<float>& vertices);
    uint32_t create_index_buffer(const std::vector<uint32_t>& indices);
    void bind_mesh(Mesh* pmesh);
    void draw_mesh(Mesh* pmesh);
};

} // namespace engine
