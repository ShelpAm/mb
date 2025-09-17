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
    glm::vec3 value;
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

inline void ai_system(entt::registry &registry)
{
    auto troops = registry.view<Ai_tag, Army, Position>();
    for (auto [entity, army, pos] : troops.each()) {
        std::random_device rd;
        std::mt19937 gen(rd()); // Mersenne Twister RNG
        std::uniform_real_distribution<float> dist_x(0.0f, 90.0f); // x bounds
        std::uniform_real_distribution<float> dist_y(0.0f,
                                                     0.0f); // if y is height, 0
        std::uniform_real_distribution<float> dist_z(0.0f, 60.0f); // z bounds

        if (!registry.any_of<Pathing>(entity)) {
            glm::vec3 random_pos{dist_x(gen), dist_y(gen), dist_z(gen)};
            registry.emplace<Pathing>(entity, random_pos);
            spdlog::debug("pathing：{} -> ({}, {}, {})",
                          static_cast<std::size_t>(entity), random_pos.x,
                          random_pos.y, random_pos.z);
        }
    }
}

inline void movement_system(entt::registry &registry, float dt)
{
    // Grants velocity to those who have will to pathing to somewhere.
    constexpr double eps{0.5};
    constexpr float speed_scale{30.};
    auto pathings = registry.view<Pathing, Position>();
    for (auto [entity, pathing, pos] : pathings.each()) {
        if (glm::length(pathing.destination - pos.value) > eps) {
            registry.emplace_or_replace<Velocity>(
                entity,
                glm::normalize(pathing.destination - pos.value) * speed_scale);
        }
        else {
            registry.emplace_or_replace<Velocity>(entity, glm::vec3{});
            registry.remove<Pathing>(entity);
        }
    }

    // Moves those have velocity to their direction.
    auto moveables = registry.view<Position, Velocity>().each();
    for (auto [entity, pos, vel] : moveables) {
        pos.value += vel.value * dt;
        auto p = pos.value;
        spdlog::debug("entity {} pos={},{},{}",
                      static_cast<std::size_t>(entity), p.x, p.y, p.z);
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
