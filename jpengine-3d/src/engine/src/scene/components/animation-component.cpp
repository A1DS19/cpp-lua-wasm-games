#include "engine/src/scene/components/animation-component.hpp"

#include "engine/src/scene/game-object.hpp"

#include <cmath>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <utility>

namespace engine {

void AnimationComponent::update(float deltatime) {
    if (clip_ == nullptr) {
        return;
    }
    if (!is_playing_) {
        return;
    }

    time_ += deltatime;

    if (time_ > clip_->duration_) {
        if (looping_) {
            time_ = std::fmod(time_, clip_->duration_);
        } else {
            time_ = 0.0F;
            is_playing_ = false;
            return;
        }
    }

    for (auto& [obj, binding] : bindings_) {
        for (auto i : binding->track_indices_) {
            auto& track = clip_->tracks_[i];

            if (!track.positions_.empty()) {
                obj->set_position(interpolate(track.positions_, time_));
            }
            if (!track.rotations_.empty()) {
                obj->set_rotation(interpolate(track.rotations_, time_));
            }
            if (!track.scales_.empty()) {
                obj->set_scale(interpolate(track.scales_, time_));
            }
        }
    }
}

void AnimationComponent::set_clip(AnimationClip* clip) {
    clip_ = clip;
    build_bindings();
}

void AnimationComponent::register_clip(const std::string& name,
                                       const std::shared_ptr<AnimationClip>& clip) {
    clips_[name] = clip;
}

void AnimationComponent::play(const std::string& name, bool loop) {
    if (clip_ != nullptr && clip_->name_ == name) {
        time_ = 0.0F;
        is_playing_ = true;
        looping_ = loop;
        return;
    }

    auto it = clips_.find(name);
    if (it != clips_.end()) {
        set_clip(it->second.get());
        time_ = 0.0F;
        is_playing_ = true;
        looping_ = loop;
    }
}

void AnimationComponent::build_bindings() {
    bindings_.clear();
    if (clip_ == nullptr) {
        return;
    }

    for (size_t i = 0; i < clip_->tracks_.size(); ++i) {
        auto& track = clip_->tracks_[i];
        auto* target = owner_->find_child_by_name(track.target_name_);
        if (target == nullptr) {
            continue;
        }

        auto it = bindings_.find(target);
        if (it != bindings_.end()) {
            it->second->track_indices_.push_back(i);
        } else {
            auto binding = std::make_unique<ObjectBinding>();
            binding->object_ = target;
            binding->track_indices_.push_back(i);
            bindings_.emplace(target, std::move(binding));
        }
    }
}

glm::vec3 AnimationComponent::interpolate(const std::vector<KeyFrameVec3>& keys, float time) {
    if (keys.empty()) {
        return glm::vec3(0.0F);
    }
    if (keys.size() == 1) {
        return keys[0].value_;
    }
    if (time <= keys.front().time_) {
        return keys.front().value_;
    }
    if (time >= keys.back().time_) {
        return keys.back().value_;
    }

    size_t i1 = 0;
    for (size_t i = 1; i < keys.size(); ++i) {
        if (time <= keys[i].time_) {
            i1 = i;
            break;
        }
    }
    size_t i0 = (i1 > 0) ? i1 - 1 : 0;

    const float dt = keys[i1].time_ - keys[i0].time_;
    const float k = (time - keys[i0].time_) / dt;
    return glm::mix(keys[i0].value_, keys[i1].value_, k);
}

glm::quat AnimationComponent::interpolate(const std::vector<KeyFrameQuat>& keys, float time) {
    // Identity quaternion as the safe fallback for rotations.
    if (keys.empty()) {
        return glm::quat(1.0F, 0.0F, 0.0F, 0.0F);
    }
    if (keys.size() == 1) {
        return keys[0].value_;
    }
    if (time <= keys.front().time_) {
        return keys.front().value_;
    }
    if (time >= keys.back().time_) {
        return keys.back().value_;
    }

    size_t i1 = 0;
    for (size_t i = 1; i < keys.size(); ++i) {
        if (time <= keys[i].time_) {
            i1 = i;
            break;
        }
    }
    size_t i0 = (i1 > 0) ? i1 - 1 : 0;

    const float dt = keys[i1].time_ - keys[i0].time_;
    const float k = (time - keys[i0].time_) / dt;
    // slerp for quaternions — preserves unit length and gives constant
    // angular velocity, unlike a linear interpolation.
    return glm::slerp(keys[i0].value_, keys[i1].value_, k);
}

} // namespace engine
