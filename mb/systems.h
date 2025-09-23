#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <random>

bool chance(float p);

// Feel environment
void ai_system(entt::registry &registry, float dt);

void movement_system(entt::registry &registry, float dt,
                     std::vector<std::vector<float>> const &mountain_height);

void collision_system(entt::registry &registry, entt::dispatcher &dispatcher,
                      float dt);

entt::entity get_active_camera(entt::registry &reg);

glm::mat4 get_active_view_mat(entt::registry &reg);

class Shader_program;
void uniform_lights(entt::registry &reg, Shader_program const &shader);
void render_system(entt::registry &registry, glm::mat4 const &proj);

void perception_system(entt::registry &registry);

void town_script(entt::registry &reg, float dt);

void collision_script(entt::registry &reg, entt::dispatcher &disp);
