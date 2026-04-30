#pragma once
#include "engine/src/scene/component.hpp"

#include <cstddef>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

struct KeyFrameVec3 {
    float time_ = 0.0F;
    glm::vec3 value_ = glm::vec3(0.0F);
};

struct KeyFrameQuat {
    float time_ = 0.0F;
    glm::quat value_ = glm::quat(1.0F, 0.0F, 0.0F, 0.0F);
};

struct TransformTrack {
    std::string target_name_;
    std::vector<KeyFrameVec3> positions_;
    std::vector<KeyFrameQuat> rotations_;
    std::vector<KeyFrameVec3> scales_;
};

struct AnimationClip {
    std::string name_;
    float duration_ = 0.0F;
    bool looping_ = true;
    std::vector<TransformTrack> tracks_;
};

struct ObjectBinding {
    GameObject* object_ = nullptr;
    std::vector<size_t> track_indices_;
};

class AnimationComponent : public Component {
    COMPONENT(AnimationComponent)

public:
    void update(float deltatime) override;
    void set_clip(AnimationClip* clip);
    void register_clip(const std::string& name, const std::shared_ptr<AnimationClip>& clip);
    void play(const std::string& name, bool loop = true);

private:
    AnimationClip* clip_ = nullptr;
    float time_ = 0.F;
    bool looping_ = true;
    bool is_playing_ = false;
    std::unordered_map<std::string, std::shared_ptr<AnimationClip>> clips_;
    std::unordered_map<GameObject*, std::unique_ptr<ObjectBinding>> bindings_;

    void build_bindings();
    glm::vec3 interpolate(const std::vector<KeyFrameVec3>& keys, float time);
    glm::quat interpolate(const std::vector<KeyFrameQuat>& keys, float time);
};

} // namespace engine
