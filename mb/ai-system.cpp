#include <mb/components.h>
#include <mb/systems.h>
#include <spdlog/spdlog.h>

bool chance(float p)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(0.0F, 1.0F);
    return dist(gen) < p;
}

void ai_system(entt::registry &reg, float dt)
{
    // Collision handling
    // for ()

    auto armies = reg.view<Ai_tag, Army, Position>();
    for (auto [e, army, pos] : armies.each()) {
        if (army.perception.viewable_entity.size() > 1) {
            auto src = e;
            auto dest = army.perception.viewable_entity[1];
            spdlog::debug("{} is tracing {} because target is in its view",
                          static_cast<int>(src), static_cast<int>(dest));
            reg.emplace_or_replace<Pathing>(
                e, Pathing{.target_is_entity = true, .dest_e = dest});
        }
        else {
            // Add cooldown component if missing
            if (!reg.all_of<Ai_cooldown>(e)) {
                reg.emplace<Ai_cooldown>(e,
                                         Ai_cooldown{.timer = 0, .total = 1});
            }

            auto &cd = reg.get<Ai_cooldown>(e);
            cd.timer -= dt; // decrease by frame delta time

            if (cd.timer > 0.0F) {
                continue; // not ready yet
            }
            cd.timer = cd.total; // reset cooldown

            // Decide action
            if (!reg.all_of<Pathing>(e)) {
                if (chance(0.3F)) {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_real_distribution<float> dist_x(0.0F, 100.0F);
                    std::uniform_real_distribution<float> dist_y(0, 0);
                    std::uniform_real_distribution<float> dist_z(0.0F, 100.0F);
                    glm::vec3 random_pos{dist_x(gen), dist_y(gen), dist_z(gen)};
                    reg.emplace<Pathing>(e, Pathing{.target_is_entity = false,
                                                    .dest_pos = random_pos});
                    spdlog::info("randomly wandering: {}", static_cast<int>(e));
                }
            }
        }
    }
}
