#pragma once
#include <cassert>
#include <entt/entt.hpp>
#include <mb/game.h>

entt::entity get_active_camera(entt::registry &reg)
{
    for (auto [entity, cam] : reg.view<Camera>().each()) {
        if (cam.is_active) {
            return entity;
        }
    }
    throw std::runtime_error("couldn't find active camera");
}
glm::mat4 get_active_view_mat(entt::registry &reg)
{
    for (auto [entity, cam, pos] : reg.view<Camera, Position>().each()) {
        if (cam.is_active) {
            return cam.calc_view_matrix(pos.value);
        }
    }
    throw std::runtime_error("couldn't find active camera");
}
