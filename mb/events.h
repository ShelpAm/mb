#pragma once
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

class Game;

struct Collision_event {
    entt::registry *registry;
    entt::entity e1, e2;
};

void process_collision_event(Collision_event const &e);
