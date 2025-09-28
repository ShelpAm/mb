#include <mb/systems.h>

void combat_system(entt::registry &reg, float dt)
{
    auto soldiers = reg.view<comp::Soldier>();
    for (auto [e, soldier] : soldiers.each()) {
        spdlog::info("Camp {} Soldier {}", static_cast<int>(e),
                     static_cast<int>(soldier.camp));
    }
}
