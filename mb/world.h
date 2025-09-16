#pragma once
#include <mb/app.h>

#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

struct Pathing {
    glm::vec3 destination;
};

struct Position {
    glm::vec3 value;
};

struct Velocity {
    glm::vec3 value;
};

class World {
  public:
    static entt::registry &registry()
    {
        static entt::registry reg;
        return reg;
    }

    static entt::entity me()
    {
        static auto me = []() {
            auto me = World::registry().create();
            registry().emplace<Position>(me, glm::vec3{0., 0., 0.});
            registry().emplace<Velocity>(me, glm::vec3{0., 0., 0.});
            return me;
        }();
        return me;
    }

    static void ai_system(double dt)
    {
        auto pathing_entities = registry().view<Pathing, Position, Velocity>();
        for (auto [entity, pathing, pos, vel] : pathing_entities.each()) {
            if (glm::length(pathing.destination - pos.value) > 0.5F) {
                vel.value =
                    glm::normalize(pathing.destination - pos.value) * 30.F;
            }
            else {
                vel.value = {};
                registry().remove<Pathing>(entity);
            }
        }
    }

    static void movement(double dt)
    {
        for (auto [entity, pos, vel] :
             registry().view<Position, Velocity>().each()) {
            pos.value += vel.value * static_cast<float>(dt);
            auto p = pos.value;
            spdlog::debug("my pos={},{},{}", p.x, p.y, p.z);
        }
    }
};
