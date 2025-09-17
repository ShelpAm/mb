#pragma once
#include <vector>

struct Troop {
    int armor;
    int weapon_damage;
};

struct Troop_stack {
    std::size_t size;
    std::size_t troop_id;
};

struct Army {
    std::vector<Troop_stack> stacks; // 例如 [100步兵, 50骑兵]
};
