#include <mb/app.h>
#include <mb/callbacks.h>
#include <mb/world.h>

#include <spdlog/spdlog.h>

void cursorpos_callback(GLFWwindow *window, double xpos, double ypos)
{
    auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
    spdlog::trace("window({}) cursor pos callback: x={} y={}",
                  static_cast<void *>(app), xpos, ypos);
    app->cursor_pos_ = {xpos, ypos};
}

void mousebutton_callback(GLFWwindow *window, int button, int action, int mods)
{
    auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
    spdlog::debug(
        "window({}) mousebutton button={} action={} mods={} cursor_pos={},{}",
        static_cast<void *>(app), button, action, mods, app->cursor_pos_.x,
        app->cursor_pos_.y);
    app->input(button, action, mods);
}
