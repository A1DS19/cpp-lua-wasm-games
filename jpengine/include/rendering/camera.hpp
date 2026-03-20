#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <sol/state.hpp>
namespace jpengine {
class Camera {
public:
    Camera();
    Camera(int width, int height);
    ~Camera();

    void update();
    void set_position(float pos_x, float pos_y) noexcept {
        position_ = glm::vec2{pos_x, pos_y};
        needs_update_ = true;
    }
    void set_position(const glm::vec2& pos) noexcept {
        position_ = pos;
        needs_update_ = true;
    }
    void set_scale(const float scale) noexcept {
        scale_ = scale;
        needs_update_ = true;
    }
    [[nodiscard]] glm::vec2 get_position() const noexcept { return position_; }
    [[nodiscard]] float get_scale() const noexcept { return scale_; }
    [[nodiscard]] glm::mat4 get_camera_matrix() const noexcept { return camera_matrix_; }
    [[nodiscard]] glm::mat4 get_projection_matrix() const noexcept { return projection_matrix_; }

    static void create_lua_bind(sol::state& lua, Camera& camera);

private:
    glm::vec2 position_;
    glm::mat4 camera_matrix_;
    glm::mat4 projection_matrix_;
    int width_;
    int height_;
    float scale_;
    bool needs_update_;

    void initialize();
};
} // namespace jpengine
