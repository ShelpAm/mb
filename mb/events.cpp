#include <mb/events.h>

#include <mb/game.h>

void process_collision_event(Collision_event const &e)
{
    e.game->set_state(Game_state::In_dialog);
}
