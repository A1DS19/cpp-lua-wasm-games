#pragma once
#include "rendering/vertex.hpp"

#include <array>
#include <glm/ext/vector_float2.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <sol/state.hpp>
#include <vector>

namespace jpengine {

class ShapeRenderer;

struct IShape {
    virtual void submit(ShapeRenderer& renderer) const = 0;
    virtual ~IShape() = default;
};

struct Rect : IShape {
    glm::vec2 position_;
    glm::vec2 size_;
    Color color_;
    bool bwireframe_;

    Rect(const glm::vec2& position, const glm::vec2& size, const Color& color = Color{},
         bool bwireframe = false);
    void submit(ShapeRenderer& renderer) const override;
};

struct Triangle : IShape {
    glm::vec2 position_;
    float base_;
    float height_;
    Color color_;
    bool bwireframe_;

    Triangle(const glm::vec2& position, float base, float height, const Color& color = Color{},
             bool bwireframe = false);
    void submit(ShapeRenderer& renderer) const override;
};

struct Circle : IShape {
    glm::vec2 center_;
    float radius_;
    Color color_;
    int segments_;
    bool bwireframe_;

    Circle(const glm::vec2& center, float radius, const Color& color = Color{}, int segments = 32,
           bool bwireframe = false);
    void submit(ShapeRenderer& renderer) const override;
};

struct Polygon : IShape {
    std::vector<glm::vec2> points_;
    Color color_;
    bool bwireframe_;

    Polygon(std::vector<glm::vec2> points, const Color& color = Color{}, bool bwireframe = false);
    void submit(ShapeRenderer& renderer) const override;
};

struct Line : IShape {
    std::array<glm::vec2, 2> points_;
    Color color_;

    Line(std::array<glm::vec2, 2> points, const Color& color = Color{});
    void submit(ShapeRenderer& renderer) const override;
};

using ShapeContainer = std::shared_ptr<std::vector<std::shared_ptr<IShape>>>;
struct ShapeBinder {
    static void create_lua_bind(sol::state& lua, class Registry& registry);
};

} // namespace jpengine
