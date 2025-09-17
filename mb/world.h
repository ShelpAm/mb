#pragma once
#include <mb/app.h>
#include <mb/mesh.h>
#include <mb/troop.h>

#include <entt/entt.hpp>
#include <random>
#include <spdlog/spdlog.h>

struct Pathing {
    glm::vec3 destination;
};

struct Position {
    glm::vec3 value;
};

struct Velocity {
    glm::vec3 dir;

    // speed shouldn't be tweaked except that an entity's moving speed has
    // changed. If you want to let some entity stop, you probably want to set
    // dir to {0,0,0}.
    float speed;
};

struct Renderable { // Use shared_ptr here? TODO
    std::shared_ptr<Mesh> mesh;
    Shader_program const *shader;
};

struct Ai_tag {};

struct Ai_cooldown {
    float timer = 0.0f; // seconds until next decision
};

struct Renderable { // Use shared_ptr here? TODO
    std::shared_ptr<Mesh> mesh;
    Shader_program const *shader;
};

struct Ai_tag {};

class World {
  public:
    static entt::registry &registry()
    {
        static entt::registry reg;
        return reg;
    }
};

inline bool chance(float p)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(0.0F, 1.0F);
    return dist(gen) < p;
}

inline void ai_system(entt::registry &registry, float dt)
{
    auto troops = registry.view<Ai_tag, Army, Position>();
    for (auto [entity, army, pos] : troops.each()) {
        // Add cooldown component if missing
        if (!registry.any_of<Ai_cooldown>(entity)) {
            registry.emplace<Ai_cooldown>(entity, 0.0F);
        }

        auto &cd = registry.get<Ai_cooldown>(entity);
        cd.timer -= dt; // decrease by frame delta time
        if (cd.timer > 0.0F) {
            continue; // not ready yet
        }

        cd.timer = 1.0F; // reset 1 second cooldown

        // Decide action
        if (!registry.any_of<Pathing>(entity)) {
            if (chance(0.3F)) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> dist_x(0.0F, 90.0F);
                std::uniform_real_distribution<float> dist_y(0.0F, 0.0F);
                std::uniform_real_distribution<float> dist_z(0.0F, 60.0F);
                glm::vec3 random_pos{dist_x(gen), dist_y(gen), dist_z(gen)};
                registry.emplace<Pathing>(entity, random_pos);
                spdlog::debug("pathing: {} -> ({}, {}, {})",
                              static_cast<std::size_t>(entity), random_pos.x,
                              random_pos.y, random_pos.z);
            }
        }
    }
}

inline void movement_system(entt::registry &registry, float dt)
{
    // Grants velocity to those who have will to pathing to somewhere.
    constexpr double eps{0.5};
    auto pathings = registry.view<Pathing, Position, Velocity>();
    for (auto [entity, pathing, pos, vel] : pathings.each()) {
        if (glm::length(pathing.destination - pos.value) > eps) {
            vel.dir = glm::normalize(pathing.destination - pos.value);
        }
        else {
            vel.dir = {};
            registry.remove<Pathing>(entity);
        }
    }

    // Moves those have velocity to their direction.
    auto moveables = registry.view<Position, Velocity>().each();
    for (auto [entity, pos, vel] : moveables) {
        pos.value += vel.dir * vel.speed * dt;
        auto p = pos.value;
        spdlog::debug("entity {} pos={},{},{}",
                      static_cast<std::size_t>(entity), p.x, p.y, p.z);
    }
}

inline void collision_system(entt::registry &registry, float now)
{
    // Pairs in this map won't collide until `value`.
    static std::map<std::pair<entt::entity, entt::entity>, double>
        collision_free;

    // Removes pairs that can perform collision between them.
    for (auto it = collision_free.begin(); it != collision_free.end();) {
        if (now >= it->second) {
            spdlog::debug("Removing collision_free pair <{} {}>",
                          static_cast<int>(it->first.first),
                          static_cast<int>(it->first.second));
            it = collision_free.erase(it);
        }
        else {
            ++it;
        }
    }

    auto armies = registry.view<Army, Position>();
    for (auto [entt1, arm1, pos1] : armies.each()) {
        for (auto [entt2, arm2, pos2] : armies.each()) {
            auto enttpair = std::make_pair(entt1, entt2);
            if (entt1 >= entt2 || collision_free.contains(enttpair)) {
                continue;
            }
            if (glm::length(pos1.value - pos2.value) <
                1.F) { // TODO(shelpam): may use another value here?
                spdlog::info("Collision detected: {} with {}",
                             static_cast<int>(entt1), static_cast<int>(entt2));
                collision_free.insert({enttpair, now + 1});
            }
        }
    }
}

inline void render_system(entt::registry &registry, float now,
                          glm::mat4 const &view, glm::mat4 const &proj)
{

    auto renderables = registry.view<Renderable, Position>();
    for (auto [entity, renderable, pos] : renderables.each()) {
        auto const *shader = renderable.shader;
        glm::mat4 model(1.);
        model = glm::translate(model, pos.value);
        auto mvp = proj * view * model;
        shader->uniform_mat4("uMVP", mvp);
        shader->uniform_1f("time", now);
        renderable.mesh->render(*shader);
    }
}
