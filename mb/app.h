#pragma once
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
    void main_loop();

    GLFWwindow *window_ = nullptr;
};
