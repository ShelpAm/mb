#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <random>

bool chance(float p);

void ai_system(entt::registry &registry, float dt);

void movement_system(entt::registry &registry, float dt,
                     std::vector<std::vector<float>> const &mountain_height);

void collision_system(entt::registry &registry, float now);

entt::entity get_active_camera(entt::registry &reg);

glm::mat4 get_active_view_mat(entt::registry &reg);

void render_system(entt::registry &registry, float now, glm::mat4 const &proj);
