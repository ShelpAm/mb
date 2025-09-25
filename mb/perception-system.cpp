#include <mb/components.h>
#include <mb/systems.h>

void perception_system(entt::registry &registry)
{
    auto armies = registry.view<Army, Position>();
    for (auto [e, army1, pos1] : armies.each()) {
        army1.perception.viewable_entity.clear();

        for (auto [e2, army2, pos2] : armies.each()) {
            auto dist = glm::distance(glm::vec2{pos1.value.x, pos1.value.z},
                                      glm::vec2{pos2.value.x, pos2.value.z});
            if (dist < view_dist) {
                army1.perception.viewable_entity.push_back(e2);
            }
        }
    }
}
