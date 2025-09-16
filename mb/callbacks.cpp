#include <mb/app.h>
#include <mb/callbacks.h>
#include <mb/world.h>

#include <spdlog/spdlog.h>

glm::vec3 screen_to_world(float x_screen, float y_screen, float depth,
                          int width, int height, glm::mat4 const &view,
                          glm::mat4 const &proj)
{
    // 1. 屏幕坐标 → NDC
    float x_ndc = (2.0F * x_screen / width) - 1.0F;
    float y_ndc = 1.0F - (2.0F * y_screen / height);
    float z_ndc = (2.0F * depth) - 1.0F;

    glm::vec4 clip(x_ndc, y_ndc, z_ndc, 1.0f);

    // 2. NDC → 相机坐标
    glm::vec4 viewCoord = glm::inverse(proj) * clip;
    viewCoord /= viewCoord.w;

    // 3. 相机坐标 → 世界坐标
    glm::vec4 worldCoord = glm::inverse(view) * viewCoord;
    return {worldCoord};
}

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
    glm::vec3 ray_start =
        screen_to_world(app->cursor_pos_.x, app->cursor_pos_.y, 0.0F,
                        app->width_, app->height_, app->view_, app->proj_);
    glm::vec3 ray_end =
        screen_to_world(app->cursor_pos_.x, app->cursor_pos_.y, 1.0F,
                        app->width_, app->height_, app->view_, app->proj_);
    glm::vec3 ray_dir = glm::normalize(ray_end - ray_start);

    // Intersect with ground plane y=0
    float t = -ray_start.y / ray_dir.y;
    glm::vec3 hit = ray_start + t * ray_dir;
    World::registry().emplace_or_replace<Pathing>(World::me(), hit);
}
