#include <mb/app.h>
#include <mb/callbacks.h>
#include <mb/world.h>

#include <spdlog/spdlog.h>

void cursorpos_callback(GLFWwindow *window, double xpos, double ypos)
{
    auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
    spdlog::debug("window({}) cursor pos callback: x={} y={}",
                  static_cast<void *>(app), xpos, ypos);
    app->cursorpos_input(xpos, ypos);
}

void mousebutton_callback(GLFWwindow *window, int button, int action, int mods)
{
    auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
    spdlog::debug(
        "window({}) mousebutton button={} action={} mods={} cursor_pos={},{}",
        static_cast<void *>(app), button, action, mods, app->cursor_pos_.x,
        app->cursor_pos_.y);
    app->mousebutton_input(button, action, mods);
}

void framebuffersize_callback(GLFWwindow *window, int width, int height)
{
    auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
    spdlog::debug("window({}) framebuffersize width={} height={}",
                  static_cast<void *>(app), width, height);
    app->framebuffersize_input(width, height);
}
