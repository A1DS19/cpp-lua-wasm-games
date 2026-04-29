#pragma once
#include "engine/src/scene/component.hpp"

#include <cstddef>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/vector_float3.hpp>
#include <string>
#include <vector>

namespace engine {

class AnimationComponent : public Component {

    struct KeyFrameVec3 {
        float time_ = 0.0f;
        glm::vec3 value_ = glm::vec3(0.0f);
    };

    struct KeyFrameQuat {
        float time_ = 0.0f;
        glm::quat value_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    };

    struct TransformTrack {
        std::string target_name_;
        std::vector<KeyFrameVec3> positions_;
        std::vector<KeyFrameQuat> rotations_;
        std::vector<KeyFrameVec3> scales_;
    };

    struct AnimationClip {
        std::string name_;
        float duration_ = 0.0f;
        bool looping_ = true;
        std::vector<TransformTrack> tracks_;
    };

    struct ObjectBinding {
        GameObject* object_ = nullptr;
        std::vector<size_t> track_indices_;
    };

public:
    void update(float deltatime) override;
};

} // namespace engine
