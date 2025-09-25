#pragma once
#include <cassert>
#include <entt/entt.hpp>
#include <mb/components.h>
#include <mb/game.h>

inline entt::entity get_active_camera(entt::registry &reg)
{
    for (auto [entity, cam] : reg.view<Camera>().each()) {
        if (cam.is_active) {
            return entity;
        }
    }
    throw std::runtime_error("couldn't find active camera");
}

inline glm::mat4 get_active_view_mat(entt::registry &reg)
{
    for (auto [entity, cam, pos] : reg.view<Camera, Position>().each()) {
        if (cam.is_active) {
            return cam.calc_view_matrix(pos.value);
        }
    }
    throw std::runtime_error("couldn't find active camera");
}

inline entt::entity get_first_local_player(entt::registry &reg)
{
    auto local_players = reg.view<Local_player_tag>();
    assert(!local_players.empty());
    return local_players.front();
}

void uniform_lights(entt::registry &reg, Shader_program const &shader);
inline void uniform_lights(entt::registry &reg, Shader_program const &shader)
{
    auto dlights = reg.view<Light, Directional_light>();
    for (auto [entity, light, dlight] : dlights.each()) {
        shader.uniform_vec3("dlight.light.ambient", light.ambient);
        shader.uniform_vec3("dlight.light.diffuse", light.diffuse);
        shader.uniform_vec3("dlight.light.specular", light.specular);
        shader.uniform_vec3("dlight.dir", dlight.dir);
    }
    auto plights = reg.view<Light, Point_light, Position>();
    for (auto [entity, light, plight, pos] : plights.each()) {
        constexpr float mul = 8;
        shader.uniform_vec3("plight.light.ambient", light.ambient * mul);
        shader.uniform_vec3("plight.light.diffuse", light.diffuse * mul);
        shader.uniform_vec3("plight.light.specular", light.specular * mul);
        shader.uniform_vec3("plight.position", pos.value);
        shader.uniform_1f("plight.constant", plight.constant);
        shader.uniform_1f("plight.linear", plight.linear);
        shader.uniform_1f("plight.quadratic", plight.quadratic);
    }
    auto slights = reg.view<Light, Spot_light, Position>();
    for (auto [entity, light, slight, pos] : slights.each()) {
        shader.uniform_vec3("slight.light.ambient", light.ambient);
        shader.uniform_vec3("slight.light.diffuse", light.diffuse);
        shader.uniform_vec3("slight.light.specular", light.specular);
        shader.uniform_1f("slight.constant", slight.constant);
        shader.uniform_1f("slight.linear", slight.linear);
        shader.uniform_1f("slight.quadratic", slight.quadratic);
        shader.uniform_vec3("slight.position", pos.value);
        shader.uniform_vec3("slight.dir", slight.dir);
        shader.uniform_1f("slight.cut_off", slight.cut_off);
        shader.uniform_1f("slight.outer_cut_off", slight.outer_cut_off);
    }
}
