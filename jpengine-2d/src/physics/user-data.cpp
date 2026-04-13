#include "physics/user-data.hpp"

#include <algorithm>
#include <ios>
#include <sstream>

namespace jpengine {

ObjectData::ObjectData(const std::string& tag, const std::string& group, bool collider,
                       bool is_friendly, bool trigger, const std::uint32_t entity_id)
    : tag_{tag}, group_{group}, collider_{collider}, trigger_{trigger}, is_friendly_{is_friendly},
      entity_id_{entity_id} {}

bool operator==(const ObjectData& a, const ObjectData& b) {
    return a.collider_ == b.collider_ && a.trigger_ == b.trigger_ &&
           a.is_friendly_ == b.is_friendly_ && a.tag_ == b.tag_ && a.group_ == b.group_ &&
           a.entity_id_ == b.entity_id_;
}

std::string ObjectData::to_string() const {
    std::stringstream ss;
    ss << "===== Object Data ======\n"
       << std::boolalpha << "Tag:        " << tag_ << "\n"
       << "Group:      " << group_ << "\n"
       << "Collider:   " << collider_ << "\n"
       << "Trigger:    " << trigger_ << "\n"
       << "Friendly:   " << is_friendly_ << "\n"
       << "Entity ID:  " << entity_id_ << "\n";
    return ss.str();
}

bool ObjectData::add_contact(const ObjectData* object_data) {
    if (tag_.empty() && group_.empty()) {
        return false;
    }
    if (object_data->tag_.empty() && object_data->group_.empty()) {
        return false;
    }
    if (object_data->tag_ == tag_ && object_data->group_ == group_) {
        return false;
    }
    if (is_friendly_ && object_data->is_friendly_ && trigger_ && object_data->trigger_) {
        return false;
    }

    auto it = std::ranges::find_if(contact_entities_, [&](const ObjectData* contact_info) {
        return *contact_info == *object_data;
    });
    if (it != contact_entities_.end()) {
        return false;
    }

    contact_entities_.push_back(object_data);
    return true;
}

bool ObjectData::remove_contact(const ObjectData* object_data) {
    if (object_data->tag_.empty() && object_data->group_.empty()) {
        return false;
    }

    auto tail = std::ranges::remove_if(contact_entities_, [&](const ObjectData* contact_info) {
        return *contact_info == *object_data;
    });

    if (tail.empty()) {
        return false;
    }

    contact_entities_.erase(tail.begin(), tail.end());
    return true;
}

} // namespace jpengine
