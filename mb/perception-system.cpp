#include <mb/components.h>
#include <mb/systems.h>

void perception_system(entt::registry &registry)
{
    auto ai_armies = registry.view<Ai_tag, Army, Position>();
    auto armies = registry.view<Army, Position>();
    for (auto [e, army1, pos1] : ai_armies.each()) {
        Perception perc{.viewable_enemies = {}};

        for (auto [e2, army2, pos2] : armies.each()) {
            if (e == e2) {
                continue;
            }

            constexpr float view_dist{10};
            auto dist = glm::distance(glm::vec2{pos1.value.x, pos1.value.z},
                                      glm::vec2{pos2.value.x, pos2.value.z});
            if (dist < view_dist) {
                perc.viewable_enemies.push_back(e2);
            }
        }

        registry.emplace_or_replace<Perception>(e, perc);
    }
}
