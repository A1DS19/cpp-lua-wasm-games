#pragma once

#include "engine/common.hpp"
#include "glm/ext/matrix_float4x4.hpp"

#include <vector>
namespace engine {

class Mesh;
class Material;

struct RenderCommand {
    Mesh* mesh_ = nullptr;
    Material* material_ = nullptr;
    glm::mat4 model_matrix_;
};

class RenderQueue {

public:
    void submit(const RenderCommand& command);
    void draw(class GraphicsApi& graphics_api, const CameraData* camera_data,
              std::vector<LightData> lights);

private:
    std::vector<RenderCommand> commands_;
};
} // namespace engine
