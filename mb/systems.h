#pragma once
#include <mb/game.h>

#include <entt/entt.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <random>

constexpr float view_dist{10};

bool chance(float p);

void ai_system(entt::registry &registry, float dt);

void movement_system(entt::registry &registry, float dt,
                     std::vector<std::vector<float>> const &mountain_height);

void collision_system(entt::registry &registry, entt::dispatcher &dispatcher,
                      float dt);

class Shader_program;
void render_system(entt::registry &registry, glm::mat4 const &proj);

// Feel environment
void perception_system(entt::registry &registry);

void town_script(entt::registry &reg, float dt);

void collision_script(entt::registry &reg, entt::dispatcher &disp);

void camera_script(entt::registry &reg, GLFWwindow *window,
                   View_mode current_view_mode);

void pathing_system(entt::registry &reg);
