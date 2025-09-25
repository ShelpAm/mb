#pragma once
#include <entt/entt.hpp>
#include <vector>

// =======TROOP=========

struct Troop {
    int armor;
    int weapon_damage;
};

struct Troop_stack {
    std::size_t size;
    std::size_t troop_id;
};

namespace internal {

struct Perception {
    std::vector<entt::entity> viewable_entity;
};

} // namespace internal

struct Army {
    std::vector<Troop_stack> stacks; // 例如 [100步兵, 50骑兵]
    internal::Perception perception;
    float money;
};
