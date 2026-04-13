#pragma once

#include <any>
#include <cstdint>
#include <entt.hpp>
#include <string>
#include <vector>

namespace jpengine {

struct UserData {
    std::any user_data_;
    std::uint32_t type_id_{0};
};

struct ObjectData {
    std::string tag_;
    std::string group_;
    bool collider_{false};
    bool trigger_{false};
    bool is_friendly_{false};
    std::uint32_t entity_id_{entt::null};

    ObjectData() = default;
    ObjectData(const std::string& tag, const std::string& group, bool collider, bool is_friendly,
               bool trigger, const std::uint32_t entity_id = entt::null);
    friend bool operator==(const ObjectData& a, const ObjectData& b);
    [[nodiscard]] std::string to_string() const;

private:
    bool add_contact(const ObjectData* object_data);
    bool remove_contact(const ObjectData* object_data);
    inline bool clear_contacts() { contact_entities_.clear(); };
    friend class ContactListener;

    std::vector<const ObjectData*> contact_entities_;
};

} // namespace jpengine
