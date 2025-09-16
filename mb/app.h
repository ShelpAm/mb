#pragma once
#include <mb/shader-program.h>

#include <entt/entt.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

class App {
    friend void cursorpos_callback(GLFWwindow *window, double xpos,
                                   double ypos);
    friend void mousebutton_callback(GLFWwindow *window, int button, int action,
                                     int mods);

  public:
    App(App const &) = delete;
    App(App &&) = delete;
    App &operator=(App const &) = delete;
    App &operator=(App &&) = delete;
    App(int width, int height, std::string const &title);
    ~App();

    void run();

  private:
    void init_window(int width, int height, std::string const &title);
    static void init_opengl();
    void init_entities();
    void main_loop();

    GLFWwindow *window_ = nullptr;
    int width_;
    int height_;
    float fwidth_;
    float fheight_;
    float const znear;
    float const zfar;

    glm::vec2 cursor_pos_{};

    glm::mat4 view_;
    glm::mat4 proj_;
};
