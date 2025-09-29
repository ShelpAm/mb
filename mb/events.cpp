#include <mb/events.h>

#include <mb/components/components.h>
#include <mb/game.h>

#include "spdlog/spdlog.h"
#include <entt/entt.hpp>

inline comp::Dialog_option make_exit_option(entt::registry &reg,
                                            entt::entity dialog_e)
{
    auto exit = [&reg, dialog_e]() {
        reg.ctx().get<Game_state>() = Game_state::Normal;
        reg.remove<comp::Dialog>(dialog_e);
    };
    return comp::Dialog_option{.reply = "Exit", .action = exit};
}

// Start battle: Creates soldier entities within a new world
/// @return  New world registry for combat
entt::registry start_battle(entt::registry &reg, entt::entity army1,
                            entt::entity army2, Entity_factory const *facotry)
{
    reg.ctx().get<Game_state>() = Game_state::In_battle;

    entt::registry battle;
    auto make_soldiers = [&reg, &battle, facotry](entt::entity army_e,
                                                  glm::vec3 basepos) {
        auto const &army = reg.get<Army>(army_e);
        for (auto const &stack : army.stacks) {
            auto const &troop = facotry->get_troop(stack.type);
            spdlog::info("army.stacks.size={}", stack.size);
            for (auto i : std::views::iota(0UZ, stack.size)) {
                auto soldier = battle.create();
                comp::Soldier attrib{.camp{army_e},
                                     .armor{troop.armor},
                                     .health{troop.health},
                                     .weapon_damage{troop.weapon_damage}};
                battle.emplace<comp::Soldier>(soldier, attrib);
                battle.emplace<Position>(soldier, Position{.value{basepos}});
            }
        }
    };

    make_soldiers(army1, glm::vec3{10, 0, 10});
    make_soldiers(army2, glm::vec3{0, 0, 0});
    return battle;
}

void process_collision_event(Collision_event const &e)
{
    spdlog::info("Processing collision event");
    if (e.registry == nullptr) {
        spdlog::error("process_collision_event: e.registry is nullptr");
    }
    e.registry->ctx().get<Game_state>() = Game_state::In_dialog;

    auto dialog_e = e.registry->create();
    std::vector<comp::Dialog_option> options;

    // Collides with Army or Town?
    if (e.registry->all_of<Army>(e.other)) {
        // Creates army dialogs
        auto out_of_my_way = [e]() {
            auto newreg = start_battle(*e.registry, e.self, e.other, e.factory);
            comp::Battle battle{.world{std::move(newreg)}};
            e.registry->emplace<comp::Battle>(e.registry->create(),
                                              std::move(battle));
            spdlog::info("Go into battle");
        };
        comp::Dialog_option fuck_option{.reply = "Out of my way!",
                                        .action = out_of_my_way};
        options.push_back(fuck_option);
        options.push_back(make_exit_option(*e.registry, dialog_e));
        comp::Dialog dialog{
            .title{"You are faced with 强盗"},
            .scripts{"I'm here to block you way! Surrender now!"},
            .current_line = 0,
            .options{std::move(options)}};
        e.registry->emplace<comp::Dialog>(dialog_e, dialog);
    }
    else { // Town
        auto const &market = e.registry->get<comp::Market>(e.other);
        for (auto item_e : market.items) {
            auto item = e.registry->get<comp::Item>(item_e);
            auto buy = [registry{e.registry}, item, dialog_e]() {
                registry->ctx().get<Game_state>() = Game_state::Normal;
                spdlog::info("You bought {}", item.name);
                registry->remove<comp::Dialog>(dialog_e);
            };
            comp::Dialog_option opt{
                .reply{std::format("I want to buy {} with price {}.", item.name,
                                   item.price)},
                .action{buy}};
            options.push_back(opt);
        }
        options.push_back(make_exit_option(*e.registry, dialog_e));
        comp::Dialog dialog{.title{"Market"},
                            .scripts{"What do you want?"},
                            .current_line = 0,
                            .options{std::move(options)}};
        e.registry->emplace<comp::Dialog>(dialog_e, dialog);
    }
}
