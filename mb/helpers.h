#pragma once
#include <cassert>
#include <entt/entt.hpp>
#include <mb/game.h>

inline Game_state &get_game_state(entt::registry &reg)
{
    auto game_states = reg.view<Game_state>();
    assert(game_states->size() == 1);
    return reg.get<Game_state>(game_states.front());
}
