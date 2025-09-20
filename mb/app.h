#pragma once
#include <mb/game.h>

#include <GLFW/glfw3.h>

class App {
  public:
    static void initialize();
    static void deinitialize();

    App(App const &) = delete;
    App(App &&) = delete;
    App &operator=(App const &) = delete;
    App &operator=(App &&) = delete;
    App(int width, int height, std::string const &title);
    ~App();

    void run();

    void cursorpos_input(double xpos, double ypos);
    void mousebutton_input(int button, int action, int mods);
    void framebuffersize_input(int width, int height);
    void scroll_input(double xoffset, double yoffset);
    void key_input(int key, int scancode, int action, int mods);

  private:
    GLFWwindow *make_window(int width, int height, std::string const &title);
    void center_glfwwindow() const;

    GLFWwindow *window_;
    Game game_;
};
