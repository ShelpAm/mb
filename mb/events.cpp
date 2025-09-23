#include "spdlog/spdlog.h"
#include <mb/events.h>

#include <mb/game.h>

void process_collision_event(Collision_event const &e)
{
    if (e.registry == nullptr) {
        spdlog::error("process_collision_event: e.registry is nullptr");
    }
    e.registry->ctx().get<Game_state>() = Game_state::In_dialog;
}
