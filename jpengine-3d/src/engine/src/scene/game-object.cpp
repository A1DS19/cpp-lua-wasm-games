#define GLM_ENABLE_EXPERIMENTAL
#include "engine/src/scene/game-object.hpp"

#include "engine/src/engine.hpp"
#include "engine/src/graphics/graphics-api.hpp"
#include "engine/src/graphics/shader-program.hpp"
#include "engine/src/graphics/texture.hpp"
#include "engine/src/graphics/vertex-layout.hpp"
#include "engine/src/render/material.hpp"
#include "engine/src/render/mesh.hpp"
#include "engine/src/scene/components/animation-component.hpp"
#include "engine/src/scene/components/mesh-component.hpp"
#include "engine/src/scene/scene.hpp"
#include "utils/asset-path.hpp"
#include "utils/file-utils.hpp"

#include <GL/glew.h>
#include <algorithm>
#include <cgltf.h>
#include <cstdint>
#include <filesystem>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace engine {

void GameObject::update(float deltatime) {
    if (!active_) {
        return;
    }

    for (auto& component : components_) {
        component->update(deltatime);
    }

    for (auto it = children_.begin(); it != children_.end();) {
        if ((*it)->is_alive_) {
            (*it)->update(deltatime);
            ++it;
        } else {
            it = children_.erase(it);
        }
    }
}

void GameObject::mark_for_destroy() {
    is_alive_ = false;
}

bool GameObject::set_parent(GameObject* parent) {
    if (scene_ == nullptr) {
        return false;
    }
    return scene_->set_parent(this, parent);
}

[[nodiscard]] glm::mat4 GameObject::get_local_transform() const {
    auto mat = glm::mat4(1.0F);
    mat = glm::translate(mat, position_);
    mat = mat * glm::mat4_cast(rotation_);
    mat = glm::scale(mat, scale_);
    return mat;
}

[[nodiscard]] glm::mat4 GameObject::get_world_transform() const {
    if (parent_ != nullptr) {
        return parent_->get_world_transform() * get_local_transform();
    }
    return get_local_transform();
}

[[nodiscard]] glm::vec3 GameObject::get_world_position() const {
    glm::vec4 hom = get_world_transform() * glm::vec4(0.0F, 0.0F, 0.0F, 1.0F);
    return glm::vec3(hom) / hom.w;
}

[[nodiscard]] GameObject* GameObject::find_child_by_name(const std::string& name) {
    for (auto& child : children_) {
        if (child->get_name() == name) {
            return child.get();
        }
        if (auto* found = child->find_child_by_name(name); found != nullptr) {
            return found;
        }
    }
    return nullptr;
}

namespace {

// Cached default shader program for glTF-imported meshes. Compiled once on
// first use; reused by every primitive that doesn't already have a material.
std::shared_ptr<ShaderProgram> get_default_shader_program() {
    static std::shared_ptr<ShaderProgram> program = [] {
        const auto vert_src = utils::read_asset_text("shaders/vertex.glsl");
        const auto frag_src = utils::read_asset_text("shaders/fragment.glsl");
        return Engine::get_instance().get_graphics_api().create_shader_program(vert_src, frag_src);
    }();
    return program;
}

void parse_gltf_node(cgltf_node* node, GameObject* parent, const std::filesystem::path& folder) {
    const std::string name = (node->name != nullptr) ? node->name : "node";
    auto* object = Engine::get_instance().get_current_scene()->create_object(name, parent);

    // --- Transform ---------------------------------------------------------
    if (node->has_matrix != 0) {
        auto mat = glm::make_mat4(node->matrix);
        glm::vec3 translation;
        glm::vec3 scale;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::quat orientation;
        glm::decompose(mat, scale, orientation, translation, skew, perspective);

        object->set_position(translation);
        object->set_rotation(orientation);
        object->set_scale(scale);
    } else {
        if (node->has_translation != 0) {
            object->set_position(
                glm::vec3(node->translation[0], node->translation[1], node->translation[2]));
        }
        if (node->has_rotation != 0) {
            // glTF stores quaternions as (x, y, z, w); glm::quat is (w, x, y, z).
            object->set_rotation(glm::quat(node->rotation[3], node->rotation[0], node->rotation[1],
                                           node->rotation[2]));
        }
        if (node->has_scale != 0) {
            object->set_scale(glm::vec3(node->scale[0], node->scale[1], node->scale[2]));
        }
    }

    // --- Mesh primitives ---------------------------------------------------
    if (node->mesh != nullptr) {
        for (cgltf_size pi = 0; pi < node->mesh->primitives_count; ++pi) {
            auto& primitive = node->mesh->primitives[pi];
            if (primitive.type != cgltf_primitive_type_triangles) {
                continue;
            }

            auto read_floats = [](const cgltf_accessor* acc, cgltf_size i, float* out, int n) {
                std::fill(out, out + n, 0.0F);
                return cgltf_accessor_read_float(acc, i, out, n) == 1;
            };

            auto read_index = [](const cgltf_accessor* acc, cgltf_size i) {
                cgltf_uint out = 0;
                cgltf_bool ok = cgltf_accessor_read_uint(acc, i, &out, 1);
                return ok != 0 ? static_cast<uint32_t>(out) : 0U;
            };

            VertexLayout vertex_layout;
            cgltf_accessor* accessors[4] = {nullptr, nullptr, nullptr, nullptr};

            for (cgltf_size ai = 0; ai < primitive.attributes_count; ++ai) {
                auto& attr = primitive.attributes[ai];
                auto* acc = attr.data;
                if (acc == nullptr) {
                    continue;
                }

                VertexElement element;
                element.type_ = GL_FLOAT;

                switch (attr.type) {
                    case cgltf_attribute_type_position:
                        accessors[VertexElement::position_index_] = acc;
                        element.index_ = VertexElement::position_index_;
                        element.size_ = 3;
                        break;
                    case cgltf_attribute_type_color:
                        if (attr.index != 0)
                            continue;
                        accessors[VertexElement::color_index_] = acc;
                        element.index_ = VertexElement::color_index_;
                        element.size_ = 3;
                        break;
                    case cgltf_attribute_type_texcoord:
                        if (attr.index != 0)
                            continue;
                        accessors[VertexElement::uv_index_] = acc;
                        element.index_ = VertexElement::uv_index_;
                        element.size_ = 2;
                        break;
                    case cgltf_attribute_type_normal:
                        accessors[VertexElement::normal_index_] = acc;
                        element.index_ = VertexElement::normal_index_;
                        element.size_ = 3;
                        break;
                    default:
                        continue;
                }

                element.offset_ = vertex_layout.stride_;
                vertex_layout.stride_ += element.size_ * sizeof(float);
                vertex_layout.elements_.push_back(element);
            }

            if (accessors[VertexElement::position_index_] == nullptr) {
                continue;
            }

            const auto vertex_count = accessors[VertexElement::position_index_]->count;
            std::vector<float> vertices((vertex_layout.stride_ / sizeof(float)) * vertex_count);

            for (cgltf_size vi = 0; vi < vertex_count; ++vi) {
                for (auto& el : vertex_layout.elements_) {
                    if (accessors[el.index_] == nullptr) {
                        continue;
                    }
                    auto idx = (vi * vertex_layout.stride_ + el.offset_) / sizeof(float);
                    float* out = &vertices[idx];
                    read_floats(accessors[el.index_], vi, out, static_cast<int>(el.size_));

                    // glTF UV origin is top-left; our textures are loaded
                    // bottom-left (stbi flip on load). Flip V to reconcile.
                    if (el.index_ == VertexElement::uv_index_ && el.size_ >= 2) {
                        out[1] = 1.0F - out[1];
                    }
                }
            }

            std::shared_ptr<Mesh> mesh;
            if (primitive.indices != nullptr) {
                const auto index_count = primitive.indices->count;
                std::vector<uint32_t> indices(index_count);
                for (cgltf_size i = 0; i < index_count; ++i) {
                    indices[i] = read_index(primitive.indices, i);
                }
                mesh = std::make_shared<Mesh>(vertex_layout, vertices, indices);
            } else {
                mesh = std::make_shared<Mesh>(vertex_layout, vertices);
            }

            // --- Material -----------------------------------------------------
            auto material = std::make_shared<Material>();
            material->set_shader_program(get_default_shader_program());

            if (primitive.material != nullptr) {
                const cgltf_image* img = nullptr;
                if (primitive.material->has_pbr_metallic_roughness != 0) {
                    auto* tex =
                        primitive.material->pbr_metallic_roughness.base_color_texture.texture;
                    if (tex != nullptr)
                        img = tex->image;
                } else if (primitive.material->has_pbr_specular_glossiness != 0) {
                    auto* tex = primitive.material->pbr_specular_glossiness.diffuse_texture.texture;
                    if (tex != nullptr)
                        img = tex->image;
                }

                if (img != nullptr && img->uri != nullptr) {
                    auto rel = (folder / std::string(img->uri)).string();
                    auto texture =
                        Engine::get_instance().get_texture_manager().get_or_load_texture(rel);
                    if (texture) {
                        material->set_param("base_color_texture", texture);
                    }
                }
            }

            object->add_component(new MeshComponent(material, mesh));
        }
    }

    // --- Children ----------------------------------------------------------
    for (cgltf_size ci = 0; ci < node->children_count; ++ci) {
        parse_gltf_node(node->children[ci], object, folder);
    }
}

} // namespace

GameObject* GameObject::load_gltf(const std::string& path) {
    const auto contents = utils::read_asset_text(path);
    if (contents.empty()) {
        return nullptr;
    }

    cgltf_options options = {};
    cgltf_data* data = nullptr;

    cgltf_result res = cgltf_parse(&options, contents.data(), contents.size(), &data);
    if (res != cgltf_result_success) {
        return nullptr;
    }

    // Third arg is the *path of the .gltf file*. cgltf takes dirname() of it
    // to resolve relative URIs inside the JSON (e.g. "Suzanne.bin").
    res = cgltf_load_buffers(&options, data, utils::asset_path(path).string().c_str());
    if (res != cgltf_result_success) {
        cgltf_free(data);
        return nullptr;
    }

    // Folder *relative to assets/* — used to resolve textures referenced by URI
    // in the glTF. e.g. path="models/suzanne/Suzanne.gltf" -> "models/suzanne/".
    const auto relative_folder = std::filesystem::path(path).remove_filename();

    // Build the root holder GameObject under the current scene's root.
    auto* scene = Engine::get_instance().get_current_scene();
    auto* root = scene->create_object(path);

    if (data->scenes_count > 0) {
        const auto& gltf_scene = data->scenes[0];
        for (cgltf_size i = 0; i < gltf_scene.nodes_count; ++i) {
            parse_gltf_node(gltf_scene.nodes[i], root, relative_folder);
        }
    }

    // ----- Animations ---------------------------------------------------
    // Each glTF animation has channels; each channel binds a (sampler, node,
    // path) where path is one of translation / rotation / scale. Multiple
    // channels for the same node merge into a single TransformTrack.
    auto read_scalar = [](cgltf_accessor* acc, cgltf_size index) {
        float v = 0.0F;
        cgltf_accessor_read_float(acc, index, &v, 1);
        return v;
    };
    auto read_vec3 = [](cgltf_accessor* acc, cgltf_size index) {
        glm::vec3 v(0.0F);
        cgltf_accessor_read_float(acc, index, glm::value_ptr(v), 3);
        return v;
    };
    auto read_quat = [](cgltf_accessor* acc, cgltf_size index) {
        float v[4] = {0.0F, 0.0F, 0.0F, 1.0F};
        cgltf_accessor_read_float(acc, index, v, 4);
        // glTF stores quats as (x, y, z, w); glm::quat is (w, x, y, z).
        return glm::quat(v[3], v[0], v[1], v[2]);
    };
    auto read_times = [&](cgltf_accessor* acc, std::vector<float>& out) {
        out.resize(acc->count);
        for (cgltf_size i = 0; i < acc->count; ++i) {
            out[i] = read_scalar(acc, i);
        }
    };
    auto read_output_vec3 = [&](cgltf_accessor* acc, std::vector<glm::vec3>& out) {
        out.resize(acc->count);
        for (cgltf_size i = 0; i < acc->count; ++i) {
            out[i] = read_vec3(acc, i);
        }
    };
    auto read_output_quat = [&](cgltf_accessor* acc, std::vector<glm::quat>& out) {
        out.resize(acc->count);
        for (cgltf_size i = 0; i < acc->count; ++i) {
            out[i] = read_quat(acc, i);
        }
    };

    std::vector<std::shared_ptr<AnimationClip>> clips;
    for (cgltf_size ai = 0; ai < data->animations_count; ++ai) {
        auto& anim = data->animations[ai];
        auto clip = std::make_shared<AnimationClip>();
        clip->name_ = (anim.name != nullptr) ? anim.name : "noname";
        clip->duration_ = 0.0F;

        // Map node → index of its TransformTrack in clip->tracks_.
        std::unordered_map<cgltf_node*, size_t> track_index_of;
        auto get_or_create_track = [&](cgltf_node* node) -> TransformTrack& {
            if (auto it = track_index_of.find(node); it != track_index_of.end()) {
                return clip->tracks_[it->second];
            }
            TransformTrack track;
            track.target_name_ = (node->name != nullptr) ? node->name : "";
            clip->tracks_.push_back(std::move(track));
            const size_t idx = clip->tracks_.size() - 1;
            track_index_of[node] = idx;
            return clip->tracks_[idx];
        };

        for (cgltf_size ci = 0; ci < anim.channels_count; ++ci) {
            auto& channel = anim.channels[ci];
            auto* sampler = channel.sampler;
            if (channel.target_node == nullptr || sampler == nullptr || sampler->input == nullptr ||
                sampler->output == nullptr) {
                continue;
            }

            std::vector<float> times;
            read_times(sampler->input, times);

            auto& track = get_or_create_track(channel.target_node);

            switch (channel.target_path) {
                case cgltf_animation_path_type_translation: {
                    std::vector<glm::vec3> values;
                    read_output_vec3(sampler->output, values);
                    track.positions_.resize(times.size());
                    for (size_t i = 0; i < times.size(); ++i) {
                        track.positions_[i].time_ = times[i];
                        track.positions_[i].value_ = values[i];
                    }
                    break;
                }
                case cgltf_animation_path_type_rotation: {
                    std::vector<glm::quat> values;
                    read_output_quat(sampler->output, values);
                    track.rotations_.resize(times.size());
                    for (size_t i = 0; i < times.size(); ++i) {
                        track.rotations_[i].time_ = times[i];
                        track.rotations_[i].value_ = values[i];
                    }
                    break;
                }
                case cgltf_animation_path_type_scale: {
                    std::vector<glm::vec3> values;
                    read_output_vec3(sampler->output, values);
                    track.scales_.resize(times.size());
                    for (size_t i = 0; i < times.size(); ++i) {
                        track.scales_[i].time_ = times[i];
                        track.scales_[i].value_ = values[i];
                    }
                    break;
                }
                default:
                    break;
            }

            if (!times.empty()) {
                clip->duration_ = std::max(clip->duration_, times.back());
            }
        }

        clips.push_back(std::move(clip));
    }

    if (!clips.empty()) {
        auto* anim_comp = new AnimationComponent();
        root->add_component(anim_comp);
        for (auto& clip : clips) {
            anim_comp->register_clip(clip->name_, clip);
        }
    }

    cgltf_free(data);
    return root;
}

} // namespace engine
