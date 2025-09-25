#include "spdlog/spdlog.h"
#include <mb/events.h>

#include <entt/entt.hpp>
#include <mb/game.h>

comp::Dialog_option make_exit_option(entt::registry &reg, entt::entity dialog_e)
{
    auto exit = [&reg, dialog_e]() {
        reg.ctx().get<Game_state>() = Game_state::Normal;
        reg.remove<comp::Dialog>(dialog_e);
    };
    return comp::Dialog_option{.reply = "Exit", .action = exit};
}

void process_collision_event(Collision_event const &e)
{
    if (e.registry == nullptr) {
        spdlog::error("process_collision_event: e.registry is nullptr");
    }
    e.registry->ctx().get<Game_state>() = Game_state::In_dialog;

    auto dialog_e = e.registry->create();
    std::vector<comp::Dialog_option> options;

    // Collides with Army or Town?
    if (e.registry->all_of<Army>(e.other)) {
        // Creates army dialogs
        auto out_of_my_way = []() { spdlog::info("ljf sb"); };
        comp::Dialog_option fuck_option{.reply = "Out of my way!",
                                        .action = out_of_my_way};
        options.push_back(fuck_option);
        options.push_back(make_exit_option(*e.registry, dialog_e));
        comp::Dialog dialog{
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
        comp::Dialog dialog{.scripts{"What do you want?"},
                            .current_line = 0,
                            .options{std::move(options)}};
        e.registry->emplace<comp::Dialog>(dialog_e, dialog);
    }
}
