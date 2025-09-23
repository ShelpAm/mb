#include "spdlog/spdlog.h"
#include <mb/events.h>

#include <mb/game.h>

void process_collision_event(Collision_event const &e)
{
    if (e.registry == nullptr) {
        spdlog::error("process_collision_event: e.registry is nullptr");
    }
    auto game_states = e.registry->view<Game_state>();
    for (auto [entity, game_state] : game_states.each()) {
        game_state = Game_state::In_dialog;
    }
    // e.game->set_state(Game_state::In_dialog);
}
