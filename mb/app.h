#pragma once
#include <entt/entt.hpp>
#include <GLFW/glfw3.h>
#include <string>

class App {
  public:
    App(int width, int height, std::string const &title);
    ~App();

    void run();

  private:
    void init_window(int width, int height, std::string const &title);
    void init_opengl();
    void init_entities();
    void main_loop();

    void ai_system(double dt);

    GLFWwindow *window_ = nullptr;
    entt::registry registry_;
};

struct Position {
    double x, y, z;
};
