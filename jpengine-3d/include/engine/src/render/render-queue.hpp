#pragma once

#include <vector>
namespace engine {

class Mesh;
class Material;

struct RenderCommand {
    Mesh* mesh_ = nullptr;
    Material* material_ = nullptr;
};

class RenderQueue {

public:
    void submit(const RenderCommand& command);
    void draw(class GraphicsApi& graphics_api);

private:
    std::vector<RenderCommand> commands_;
};
} // namespace engine
