#pragma once
#include <mb/entity-factory.h>

#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

struct Collision_event {
    entt::registry *registry;
    entt::entity self, other;
    Entity_factory const *factory;
};

entt::registry start_battle(entt::registry &reg, entt::entity army1,
                            entt::entity army2, Entity_factory const *facotry);

void process_collision_event(Collision_event const &e);
