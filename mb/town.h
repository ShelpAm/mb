#pragma once
#include <entt/entt.hpp>
#include <vector>

namespace component {

// Basically, all items in the game are sellable
struct Item {
    std::string name;
    float price;
};

struct Market {
    std::vector<entt::entity> items; // vector of Items
};

// Town_tag should own Market component.
struct Town_tag {};

} // namespace component
