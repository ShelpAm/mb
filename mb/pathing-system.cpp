#include <mb/systems.h>

#include <ranges>

/// @brief Grants velocity to those who have will to pathing to somewhere, but
/// remove pathing for arrived and losing target views.
///
/// @note Depends on perception_system
void pathing_system(entt::registry &reg)
{
    constexpr double pathing_eps{0.5};
    auto pathings = reg.view<Army, Pathing, Position, Velocity>();
    for (auto [e, army, pathing, pos, vel] : pathings.each()) {
        // Pathing to x,z
        glm::vec3 dest;
        if (pathing.target_is_entity) {
            // Lost view of target
            if (!std::ranges::contains(army.perception.viewable_entity,
                                       pathing.dest_e)) {
                spdlog::info("{} lost view of {}, stop pathing",
                             static_cast<int>(e),
                             static_cast<int>(pathing.dest_e));
                reg.remove<Pathing>(e);
                continue;
            }
            dest = reg.get<Position>(pathing.dest_e).value;
        }
        else { // glm::vec3
            dest = pathing.dest_pos;
        }
        if (glm::distance(glm::vec2{dest.x, dest.z},
                          glm::vec2{pos.value.x, pos.value.z}) > pathing_eps) {
            spdlog::debug("pathing: {} -> ({}, {}, {})", static_cast<int>(e),
                          dest.x, dest.y, dest.z);
            vel.dir = glm::normalize(dest - pos.value);
        }
        else {
            spdlog::debug("pathing: {} arrived ({}, {}, {})",
                          static_cast<int>(e), pos.value.x, pos.value.y,
                          pos.value.z);
            vel.dir = {};
            reg.remove<Pathing>(e);
        }
    }
}
