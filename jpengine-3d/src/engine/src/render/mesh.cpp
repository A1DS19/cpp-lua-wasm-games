#include "engine/src/render/mesh.hpp"

#include "engine/src/engine.hpp"
#include "engine/src/graphics/vertex-layout.hpp"
#include "utils/asset-path.hpp"
#include "utils/file-utils.hpp"

#include <GL/glew.h>
#include <cstdint>
#include <memory>
#include <vector>

#include <cgltf.h>

namespace engine {

void Mesh::bind() {
    glBindVertexArray(vao_);
}

void Mesh::draw() {
    if (index_count_ > 0) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_count_), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertex_count_));
    }
}

Mesh::Mesh(const VertexLayout& layout, const std::vector<float> vertices,
           const std::vector<uint32_t> indices) {
    vertex_layout_ = layout;
    auto& graphics_api = engine::Engine::get_instance().get_graphics_api();

    vbo_ = graphics_api.create_vertex_buffer(vertices);
    ebo_ = graphics_api.create_index_buffer(indices);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    for (auto& element : vertex_layout_.elements_) {
        glVertexAttribPointer(element.index_, static_cast<GLint>(element.size_), element.type_,
                              GL_FALSE, static_cast<GLsizei>(vertex_layout_.stride_),
                              reinterpret_cast<void*>(static_cast<uintptr_t>(element.offset_)));
        glEnableVertexAttribArray(element.index_);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    vertex_count_ = (vertices.size() * sizeof(float)) / vertex_layout_.stride_;
    index_count_ = indices.size();
}

Mesh::Mesh(const VertexLayout& layout, const std::vector<float> vertices) {
    vertex_layout_ = layout;
    auto& graphics_api = engine::Engine::get_instance().get_graphics_api();

    vbo_ = graphics_api.create_vertex_buffer(vertices);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    for (auto& element : vertex_layout_.elements_) {
        glVertexAttribPointer(element.index_, static_cast<GLint>(element.size_), element.type_,
                              GL_FALSE, static_cast<GLsizei>(vertex_layout_.stride_),
                              reinterpret_cast<void*>(static_cast<uintptr_t>(element.offset_)));
        glEnableVertexAttribArray(element.index_);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    vertex_count_ = (vertices.size() * sizeof(float)) / vertex_layout_.stride_;
}

std::shared_ptr<Mesh> Mesh::load(const std::string& path) {
    auto contents = utils::read_asset_text(path);

    if (contents.empty()) {
        return nullptr;
    }

    auto read_floats = [](const cgltf_accessor* acc, cgltf_size i, float* out, int n) {
        std::fill(out, out + n, 0.0F);
        return cgltf_accessor_read_float(acc, i, out, n) == 1;
    };

    auto read_index = [](const cgltf_accessor* acc, cgltf_size i) {
        cgltf_uint out = 0;
        cgltf_bool ok = cgltf_accessor_read_uint(acc, i, &out, 1);
        return ok ? static_cast<uint32_t>(out) : 0;
    };

    cgltf_options options = {};
    cgltf_data* data = nullptr;
    cgltf_result res = cgltf_parse(&options, contents.data(), contents.size(), &data);

    if (res != cgltf_result_success) {
        return nullptr;
    }

    // Third arg is the *path of the .gltf file*. cgltf takes dirname() of it
    // to resolve the relative URIs inside the JSON (e.g. "Suzanne.bin").
    res = cgltf_load_buffers(&options, data, utils::asset_path(path).string().c_str());

    if (res != cgltf_result_success) {
        cgltf_free(data);
        return nullptr;
    }

    std::shared_ptr<Mesh> result = nullptr;
    for (cgltf_size mi = 0; mi < data->meshes_count; mi++) {
        auto mesh = data->meshes[mi];
        for (cgltf_size pi = 0; pi < mesh.primitives_count; ++pi) {
            auto& primitive = mesh.primitives[pi];
            if (primitive.type != cgltf_primitive_type_triangles) {
                continue;
            }

            VertexLayout vertex_layout;
            cgltf_accessor* accesors[4] = {nullptr, nullptr, nullptr, nullptr};

            for (cgltf_size ai = 0; ai < primitive.attributes_count; ai++) {
                auto& attr = primitive.attributes[ai];
                auto acc = attr.data;
                if (!acc) {
                    continue;
                }

                VertexElement element;
                element.type_ = GL_FLOAT;
                switch (attr.type) {
                    case cgltf_attribute_type_position: {
                        accesors[VertexElement::position_index_] = acc;
                        element.index_ = VertexElement::position_index_;
                        element.size_ = 3;
                        break;
                    }

                    case cgltf_attribute_type_color: {
                        if (attr.index != 0) {
                            continue;
                        }
                        accesors[VertexElement::color_index_] = acc;
                        element.index_ = VertexElement::color_index_;
                        element.size_ = 3;
                        break;
                    }

                    case cgltf_attribute_type_texcoord: {
                        if (attr.index != 0) {
                            continue;
                        }
                        accesors[VertexElement::uv_index_] = acc;
                        element.index_ = VertexElement::uv_index_;
                        element.size_ = 2;
                        break;
                    }

                    case cgltf_attribute_type_normal: {
                        accesors[VertexElement::normal_index_] = acc;
                        element.index_ = VertexElement::normal_index_;
                        element.size_ = 3;
                        break;
                    }

                    default:
                        continue;
                }

                if (element.size_ > 0) {
                    element.offset_ = vertex_layout.stride_;
                    vertex_layout.stride_ += element.size_ * sizeof(float);
                    vertex_layout.elements_.push_back(element);
                }
            }

            if (!accesors[VertexElement::position_index_]) {
                continue;
            }

            auto vertex_count = accesors[VertexElement::position_index_]->count;
            std::vector<float> vertices;
            vertices.resize((vertex_layout.stride_ / sizeof(float)) * vertex_count);

            for (cgltf_size vi = 0; vi < vertex_count; ++vi) {
                for (auto& el : vertex_layout.elements_) {
                    if (!accesors[el.index_]) {
                        continue;
                    }

                    auto index = (vi * vertex_layout.stride_ + el.offset_) / sizeof(float);
                    float* out_data = &vertices[index];
                    read_floats(accesors[el.index_], vi, out_data, el.size_);

                    // glTF UV origin is top-left; our textures are loaded with
                    // stbi_set_flip_vertically_on_load(true) for the GL-native
                    // bottom-left origin. Flip V to reconcile the two.
                    if (el.index_ == VertexElement::uv_index_ && el.size_ >= 2) {
                        out_data[1] = 1.0F - out_data[1];
                    }
                }
            }

            if (primitive.indices) {
                auto index_count = primitive.indices->count;
                std::vector<uint32_t> indices(index_count);
                for (cgltf_size i = 0; i < index_count; ++i) {
                    indices[i] = read_index(primitive.indices, i);
                }

                result = std::make_shared<Mesh>(vertex_layout, vertices, indices);
            } else {
                result = std::make_shared<Mesh>(vertex_layout, vertices);
            }

            if (result) {
                break;
            }
        }

        if (result) {
            break;
        }
    }

    cgltf_free(data);
    return result;
}

} // namespace engine
