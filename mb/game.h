#pragma once
#include <mb/font.h>
#include <mb/shader-program.h>

#include <entt/entt.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum class View_mode { God, First_player };

enum class Game_state { Normal, In_dialog, Should_exit };

class Game {
    friend class Ui;

  public:
    Game(int width, int height);

    void init_world();
    void main_loop(GLFWwindow *window);

    void cursorpos_input(double xpos, double ypos);
    void mousebutton_input(int button, int action, int mods);
    void windowresize_input(int width, int height);
    void scroll_input(double xoffset, double yoffset);
    void key_input(int key, int scancode, int action, int mods);

    // void set_state(Game_state state)
    // {
    //     state_ = state;
    // }

  private:
    void process_events();

    void normal(GLFWwindow *window, float dt);
    void in_dialog(GLFWwindow *window);

    int width_;
    int height_;
    glm::vec2 cursor_pos_{-1, -1};
    glm::vec2 cursor_pos_delta_{};
    glm::mat4 proj_;
    Shader_program shader_;
    Shader_program light_cube_shader_;
    Shader_program font_shader_;
    entt::registry registry_;
    entt::dispatcher dispatcher_;

    std::vector<std::vector<float>> height_map_;

    Font font_;
    Ui ui_;

    View_mode view_mode_{View_mode::God};
};
