#include <mb/systems/systems.h>

// FIXME
void combat_system(entt::registry &reg, float dt)
{
    auto soldiers = reg.view<comp::Soldier, Position>();

    static std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> damage_variation(-2, 2);

    for (auto [e, soldier, pos] : soldiers.each()) {
        // 找最近的敌人
        entt::entity closest_enemy = entt::null;
        float min_dist2 = std::numeric_limits<float>::max();
        for (auto [enemy_e, enemy_soldier, enemy_pos] : soldiers.each()) {
            if (enemy_soldier.camp != soldier.camp) {
                float d = glm::distance(pos.value, enemy_pos.value);
                if (d < min_dist2) {
                    min_dist2 = d;
                    closest_enemy = enemy_e;
                }
            }
        }

        if (closest_enemy != entt::null) {
            auto &enemy_pos = reg.get<Position>(closest_enemy);
            auto &enemy_soldier = reg.get<comp::Soldier>(closest_enemy);

            float dist = glm::distance(pos.value, enemy_pos.value);
            float move_speed = 2.0f; // 每帧移动速度

            if (dist > 5.0f) {
                // 移动靠近敌人
                glm::vec3 dir = glm::normalize(enemy_pos.value - pos.value);
                pos.value += dir * move_speed * dt;
                spdlog::info("Entity {} moves towards {}. New pos: "
                             "({:.1f},{:.1f},{:.1f})",
                             static_cast<int>(e),
                             static_cast<int>(closest_enemy), pos.value.x,
                             pos.value.y, pos.value.z);
            }
            else {
                // 攻击敌人
                int dmg = soldier.weapon_damage + damage_variation(rng);
                enemy_soldier.health -= dmg;
                spdlog::info("Entity {} attacks {} for {} damage. Enemy HP: {}",
                             static_cast<int>(e),
                             static_cast<int>(closest_enemy), dmg,
                             enemy_soldier.health);
                if (enemy_soldier.health <= 0) {
                    spdlog::info("Entity {} died!",
                                 static_cast<int>(closest_enemy));
                    reg.destroy(closest_enemy);
                }
            }
        }
    }
}
