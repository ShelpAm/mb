#include <mb/app.h>
#include <mb/callbacks.h>
#include <mb/world.h>

#include <spdlog/spdlog.h>

void cursorpos_callback(GLFWwindow *window, double xpos, double ypos)
{
    auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
    app->cursorpos_input(xpos, ypos);
    spdlog::debug("window({}) cursor pos callback: x={} y={}",
                  static_cast<void *>(app), xpos, ypos);
}

void mousebutton_callback(GLFWwindow *window, int button, int action, int mods)
{
    auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
    app->mousebutton_input(button, action, mods);
    spdlog::debug(
        "window({}) mousebutton callback: button={} action={} mods={}",
        static_cast<void *>(app), button, action, mods);
}

void framebuffersize_callback(GLFWwindow *window, int width, int height)
{
    auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
    app->framebuffersize_input(width, height);
    spdlog::debug("window({}) framebuffersize callback: width={} height={}",
                  static_cast<void *>(app), width, height);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
    app->scroll_input(xoffset, yoffset);
    spdlog::debug("window({}) scroll callback: xoffset={} yoffset={}",
                  static_cast<void *>(app), xoffset, yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods)
{
    auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
    app->key_input(key, scancode, action, mods);
    spdlog::debug(
        "window({}) key callback: key={} scancode={} action={} mods={}",
        static_cast<void *>(app), key, scancode, action, mods);
}
