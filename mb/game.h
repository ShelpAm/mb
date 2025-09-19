#pragma once
#include <mb/shader-program.h>

#include <entt/entt.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum class View_mode { God, First_player };

class Game {
  public:
    Game(int width, int height);

    void init_world();
    void main_loop(GLFWwindow *window);

    void cursorpos_input(double xpos, double ypos);
    void mousebutton_input(int button, int action, int mods);
    void windowresize_input(int width, int height);
    void scroll_input(double xoffset, double yoffset);
    void key_input(int key, int scancode, int action, int mods);

    [[nodiscard]] entt::registry &registry();

  private:
    void process_events();

    int width_;
    int height_;
    glm::vec2 cursor_pos_{-1, -1};
    glm::vec2 cursor_pos_delta_{};
    glm::mat4 proj_;
    Shader_program shader_;
    Shader_program light_cube_shader_;
    entt::registry registry_;
    std::vector<std::vector<float>> height_map_;

    View_mode view_mode_{View_mode::God};
};
