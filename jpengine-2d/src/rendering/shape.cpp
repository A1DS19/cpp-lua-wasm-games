#include "rendering/shape.hpp"

#include "ecs/registry.hpp"
#include "rendering/shape-batch-renderer.hpp"

#include <array>
#include <cassert>
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include <sol/sol.hpp>
#include <vector>

namespace jpengine {

Rect::Rect(const glm::vec2& position, const glm::vec2& size, const Color& color, bool bwireframe)
    : position_{position}, size_{size}, color_{color}, bwireframe_{bwireframe} {}

void Rect::submit(ShapeRenderer& renderer) const {
    if (bwireframe_) {
        renderer.add_wire_rectangle(position_, size_, color_);
    } else {
        renderer.add_rectangle(position_, size_, color_);
    }
}

Triangle::Triangle(const glm::vec2& position, float base, float height, const Color& color,
                   bool bwireframe)
    : position_{position}, base_{base}, height_{height}, color_{color}, bwireframe_{bwireframe} {}

void Triangle::submit(ShapeRenderer& renderer) const {
    glm::vec2 p1 = {position_.x, position_.y};
    glm::vec2 p2 = {position_.x + base_, position_.y};
    glm::vec2 p3 = {position_.x + base_ * 0.5F, position_.y - height_};

    if (bwireframe_) {
        renderer.add_wire_triangle(p1, p2, p3, color_);
    } else {
        renderer.add_triangle(p1, p2, p3, color_);
    }
}

Circle::Circle(const glm::vec2& center, float radius, const Color& color, int segments,
               bool bwireframe)
    : center_{center}, radius_{radius}, color_{color}, segments_{segments},
      bwireframe_{bwireframe} {}

void Circle::submit(ShapeRenderer& renderer) const {
    if (bwireframe_) {
        renderer.add_wire_circle(center_, radius_, color_, segments_);
    } else {
        renderer.add_circle(center_, radius_, color_, segments_);
    }
}

Polygon::Polygon(std::vector<glm::vec2> points, const Color& color, bool bwireframe)
    : points_{std::move(points)}, color_{color}, bwireframe_{bwireframe} {}

void Polygon::submit(ShapeRenderer& renderer) const {
    if (bwireframe_) {
        renderer.add_wire_polygon(points_, color_);
    } else {
        renderer.add_polygon(points_, color_);
    }
}

Line::Line(std::array<glm::vec2, 2> points, const Color& color)
    : points_(std::move(points)), color_{color} {}

void Line::submit(ShapeRenderer& renderer) const {
    renderer.add_line(points_[0], points_[1], color_);
}

void ShapeBinder::create_lua_bind(sol::state& lua, Registry& registry) {
    lua.new_usertype<Rect>(
        "Rect", sol::call_constructor,
        sol::constructors<Rect(const glm::vec2&, const glm::vec2&, const Color&, bool)>(),
        "position", &Rect::position_, "size", &Rect::size_, "color", &Rect::color_, "bwireframe",
        &Rect::bwireframe_);

    lua.new_usertype<Circle>(
        "Circle", sol::call_constructor,
        sol::constructors<Circle(const glm::vec2&, float, const Color&, int, bool)>(), "center",
        &Circle::center_, "radius", &Circle::radius_, "segments", &Circle::segments_, "color",
        &Circle::color_, "bwireframe", &Circle::bwireframe_);

    lua.new_usertype<Triangle>(
        "Triangle", sol::call_constructor,
        sol::constructors<Triangle(const glm::vec2&, float, float, const Color&, bool)>(),
        "position", &Triangle::position_, "base", &Triangle::base_, "height", &Triangle::height_,
        "color", &Triangle::color_, "bwireframe", &Triangle::bwireframe_);

    lua.new_usertype<Polygon>(
        "Polygon", sol::call_constructor,
        sol::factories([](const sol::table& pts, const Color& color, bool bwireframe) {
            std::vector<glm::vec2> points{};
            for (const auto& [_, object] : pts) {
                points.push_back(object.as<glm::vec2>());
            }
            return Polygon{std::move(points), color, bwireframe};
        }),
        "points", &Polygon::points_, "color", &Polygon::color_, "bwireframe",
        &Polygon::bwireframe_);

    lua.new_usertype<Line>(
        "Line", sol::call_constructor,
        sol::factories([](const glm::vec2& p1, const glm::vec2& p2, const Color& color) {
            return Line{std::array<glm::vec2, 2>{p1, p2}, color};
        }),
        "points", &Line::points_, "color", &Line::color_);

    auto& pshape_container = registry.get_context<ShapeContainer>();
    assert(pshape_container && "Shape Container was not added to the registry context!");

    lua.set_function("draw_rect", [&pshape_container](const Rect& rect) {
        pshape_container->push_back(std::make_shared<Rect>(rect));
    });

    lua.set_function("draw_triangle", [&pshape_container](const Triangle& triangle) {
        pshape_container->push_back(std::make_shared<Triangle>(triangle));
    });

    lua.set_function("draw_circle", [&pshape_container](const Circle& circle) {
        pshape_container->push_back(std::make_shared<Circle>(circle));
    });

    lua.set_function("draw_polygon", [&pshape_container](const Polygon& polygon) {
        pshape_container->push_back(std::make_shared<Polygon>(polygon));
    });

    lua.set_function("draw_line", [&pshape_container](const Line& line) {
        pshape_container->push_back(std::make_shared<Line>(line));
    });
}

} // namespace jpengine
