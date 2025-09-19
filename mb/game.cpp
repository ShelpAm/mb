#include <mb/game.h>

#include <mb/components.h>
#include <mb/generate-mesh.h>
#include <mb/lights.h>
#include <mb/systems.h>
#include <mb/texture.h>
#include <mb/troop.h>

Game::Game(int width, int height)
    : width_{width}, height_{height},
      proj_{glm::perspective(
          glm::radians(45.F),
          static_cast<float>(width) / static_cast<float>(height), .1F, 300.F)},
      shader_("./shader/main.vert", "./shader/main.frag"),
      light_cube_shader_("./shader/main.vert", "./shader/light.frag")
{
    windowresize_input(width, height);
}

void Game::init_world()
{
    auto &reg = registry_;

    auto cube = generate_cube_mesh();
    auto [terrain_mesh, height] = generate_terrain_mesh(100, 100, 0.05F);
    height_map_ = height;

    // Init camere
    {
        auto cam_entity = reg.create();
        reg.emplace<Camera>(cam_entity, Camera{.yaw = std::numbers::pi / 2,
                                               .pitch = -std::numbers::pi / 3,
                                               .is_active = false});
        reg.emplace<Position>(cam_entity, Position{.value = {45, 80, 100}});
        reg.emplace<Velocity>(cam_entity, Velocity{.dir = {}, .speed = 30});
        reg.emplace<View_mode>(cam_entity, View_mode::God);
    }
    entt::entity fpscam;
    {
        auto cam_entity = reg.create();
        fpscam = cam_entity;
        reg.emplace<Camera>(
            cam_entity,
            Camera{.yaw = std::numbers::pi / 2, .pitch = 0, .is_active = true});
        reg.emplace<Position>(cam_entity, Position{.value = {29, 18, 50}});
        reg.emplace<Velocity>(cam_entity, Velocity{.dir = {}, .speed = 5});
        reg.emplace<View_mode>(cam_entity, View_mode::First_player);
    }

    Texture diffuse("/home/shelpam/Pictures/wjz.jpg");
    Texture specular("/home/shelpam/Pictures/wjz.jpg");

    // Init light cube
    {
        auto light_cube = reg.create();
        reg.emplace<Position>(light_cube, glm::vec3{30, 20, 40});
        reg.emplace<Renderable>(light_cube,
                                Renderable{.mesh = cube,
                                           .shader = &light_cube_shader_,
                                           .diffuse_map = diffuse,
                                           .specular_map = specular});
    }
    { // Init directional light
        auto light = reg.create();
        reg.emplace<Directional_light>(light,
                                       Directional_light{.dir = {-1, -3, 2}});
        reg.emplace<Light>(light, Light{.ambient = glm::vec3{0.1},
                                        .diffuse = glm::vec3{0.5},
                                        .specular = glm::vec3{0.5}});
    }
    { // Init point light
        auto light = reg.create();
        reg.emplace<Point_light>(light, Point_light{.constant = 1.0F,
                                                    .linear = 0.09F,
                                                    .quadratic = 0.032F});
        reg.emplace<Position>(light, glm::vec3{30, 20, 40});
        reg.emplace<Light>(light, Light{.ambient = glm::vec3{0.1},
                                        .diffuse = glm::vec3{0.5},
                                        .specular = glm::vec3{1}});
    }
    { // Init spot light
        auto light = reg.create();
        reg.emplace<Spot_light>(
            light, Spot_light{.constant = 1,
                              .linear = 0.045,
                              .quadratic = 0.0075,
                              .dir = reg.get<Camera>(fpscam).front(),
                              .cut_off = glm::cos(glm::radians(25.F)),
                              .outer_cut_off = glm::cos(glm::radians(30.F))});
        reg.emplace<Position>(light, reg.get<Position>(fpscam));
        reg.emplace<Light>(light, Light{.ambient = glm::vec3{0.1},
                                        .diffuse = glm::vec3{0.8},
                                        .specular = glm::vec3{1}});
    }

    // Init `me`
    auto me = reg.create();
    reg.emplace<Local_player_tag>(me);
    reg.emplace<Position>(me, glm::vec3{28, 17, 47});
    reg.emplace<Velocity>(me, Velocity{.dir = {0., 0., 0.}, .speed = 25});
    std::vector<Troop_stack> myarmy;
    myarmy.push_back(Troop_stack{.size = 1, .troop_id = -1UZ});
    reg.emplace<Army>(me, Army{.stacks = myarmy});
    reg.emplace<Renderable>(me, Renderable{.mesh = cube,
                                           .shader = &shader_,
                                           .diffuse_map = diffuse,
                                           .specular_map = specular});

    // Init armies
    for (int i{}; i != 5; ++i) {
        auto entity = reg.create();
        reg.emplace<Ai_tag>(entity);
        std::vector<Troop_stack> army;
        army.push_back(Troop_stack{.size = 3, .troop_id = -1UZ});
        reg.emplace<Army>(entity, Army{.stacks = army});
        reg.emplace<Position>(entity, glm::vec3{10, 0, 15});
        reg.emplace<Velocity>(entity, Velocity{.dir = {}, .speed = 20});
        reg.emplace<Renderable>(entity, Renderable{.mesh = cube,
                                                   .shader = &shader_,
                                                   .diffuse_map = diffuse,
                                                   .specular_map = specular});
    }

    auto terrain_entity = reg.create();
    reg.emplace<Renderable>(terrain_entity,
                            Renderable{.mesh = terrain_mesh,
                                       .shader = &shader_,
                                       .diffuse_map = diffuse,
                                       .specular_map = specular});
    reg.emplace<Position>(terrain_entity, glm::vec3{0.0F, 0.0F, 0.0F});
}

void Game::main_loop(GLFWwindow *window)
{
    double last_frame = glfwGetTime();

    spdlog::info("Entering main loop...");
    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();

        double now = glfwGetTime();
        double dt = now - last_frame;
        last_frame = now;

        { // Show FPS
            static double accumu{};
            accumu += dt;
            if (accumu >= 1) {
                auto fps = 1. / dt;
                spdlog::warn("fps={}", fps);
                accumu = 0;
            }
        }
        { // MANAGE VIEW MODE: Set active camera
            switch (view_mode_) {
            case View_mode::God:
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
                break;
            case View_mode::First_player:
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                break;
            }
            auto cameras = registry_.view<Camera, View_mode>();
            for (auto [entity, cam, view_mode] : cameras.each()) {
                cam.is_active = view_mode == view_mode_;
            }
        }
        ai_system(registry_, static_cast<float>(dt));
        movement_system(registry_, static_cast<float>(dt), height_map_);
        collision_system(registry_, static_cast<float>(now));

        glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_system(registry_, static_cast<float>(now), proj_);
        glfwSwapBuffers(window);
    }
    spdlog::info("Exited from main loop");
}

void Game::cursorpos_input(double xpos, double ypos)
{
    // Initialize first cursor_pos_ to its correct value. Otherwise, pos_delta
    // could be very large in the first cursor moving frame.
    if (cursor_pos_ == glm::vec2{-1, -1}) {
        cursor_pos_ = {xpos, ypos};
    }

    cursor_pos_delta_ = glm::vec2{xpos, ypos} - cursor_pos_;
    spdlog::debug("dx={} dy={}", cursor_pos_delta_.x, cursor_pos_delta_.y);
    cursor_pos_ = {xpos, ypos};

    auto cam_entity = get_active_camera(registry_);
    switch (view_mode_) {
    case View_mode::God: {
        constexpr int edge_width{20};
        auto &vel{registry_.get<Velocity>(cam_entity)};
        vel.dir = {};
        if (xpos < edge_width) {
            vel.dir += glm::vec3{-1, 0, 0};
        }
        else if (xpos >= width_ - edge_width) {
            vel.dir += glm::vec3{1, 0, 0};
        }
        if (ypos < edge_width) {
            vel.dir += glm::vec3{0, 0, -1};
        }
        else if (ypos >= height_ - edge_width) {
            vel.dir += glm::vec3{0, 0, 1};
        }
        break;
    }
    case View_mode::First_player: {
        constexpr float sensitivity{0.03 * std::numbers::pi / 180}; // 1 degree
        auto &cam{registry_.get<Camera>(cam_entity)};
        cam.yaw -= sensitivity * cursor_pos_delta_.x;
        cam.pitch -= sensitivity * cursor_pos_delta_.y;
        cam.pitch = glm::clamp<float>(cam.pitch, (-std::numbers::pi / 2) + 1e-5,
                                      (std::numbers::pi / 2) - 1e-5);
        break;
    }
    }
}

inline glm::vec3 screen_to_world(float x_screen, float y_screen, float depth,
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

void Game::mousebutton_input(int button, int action, int mods)
{
    auto view = get_active_view_mat(registry_);

    switch (view_mode_) {
    case View_mode::God: {
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            glm::vec3 ray_start =
                screen_to_world(cursor_pos_.x, cursor_pos_.y, 0.0F, width_,
                                height_, view, proj_);
            glm::vec3 ray_end =
                screen_to_world(cursor_pos_.x, cursor_pos_.y, 1.0F, width_,
                                height_, view, proj_);
            glm::vec3 ray_dir = glm::normalize(ray_end - ray_start);

            // Intersect with ground plane y=0
            float t = -ray_start.y / ray_dir.y;
            glm::vec3 hit = ray_start + t * ray_dir;
            for (auto [me] : registry_.view<Local_player_tag>().each()) {
                registry_.emplace_or_replace<Pathing>(
                    me,
                    glm::vec3{hit.x,
                              height_map_[int(hit.z) % height_map_.size()]
                                         [int(hit.x) % height_map_[0].size()] +
                                  2,
                              hit.z});
            }
        }
        break;
    }
    case View_mode::First_player:
        break;
    }

    // TODO(shelpam): left to pick
}

void Game::windowresize_input(int width, int height)
{
    width_ = width;
    height_ = height;

    glViewport(0, 0, width, height);
    proj_ = glm::perspective(
        glm::radians(45.F),
        static_cast<float>(width) / static_cast<float>(height), .1F, 300.F);
}

void Game::scroll_input(double xoffset, double yoffset)
{
    switch (view_mode_) {
    case View_mode::God: {
        auto cam_entity = get_active_camera(registry_);
        auto const &cam = registry_.get<Camera>(cam_entity);
        registry_.get<Position>(cam_entity).value +=
            5.F * static_cast<float>(yoffset) * cam.front();
        break;
    }
    case View_mode::First_player:
        break;
    }
}

void Game::key_input(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        view_mode_ = static_cast<View_mode>(static_cast<int>(view_mode_) ^ 1);
    }

    static std::deque<bool> key_pressed(GLFW_KEY_LAST + 1);
    key_pressed[key] = action == GLFW_PRESS || action == GLFW_REPEAT;

    auto cam_entity = get_active_camera(registry_);
    if (!registry_.valid(cam_entity)) {
        spdlog::error("[key_input] No active camera");
        return;
    }

    switch (view_mode_) {
    case View_mode::God:
        break;
    case View_mode::First_player: {
        auto &cam = registry_.get<Camera>(cam_entity);
        auto &vel = registry_.get<Velocity>(cam_entity);
        vel.dir = glm::vec3{};

        glm::vec3 facing{glm::cos(cam.yaw), 0, -glm::sin(cam.yaw)};
        glm::vec3 right{glm::normalize(glm::cross(facing, {0, 1, 0}))};
        if (key_pressed[GLFW_KEY_W] && !key_pressed[GLFW_KEY_S]) {
            vel.dir += facing; // Move forward
        }
        else if (key_pressed[GLFW_KEY_S] && !key_pressed[GLFW_KEY_W]) {
            vel.dir += -facing; // Move backward
        }

        if (key_pressed[GLFW_KEY_A] && !key_pressed[GLFW_KEY_D]) {
            vel.dir += -right; // Move left
        }
        else if (key_pressed[GLFW_KEY_D] && !key_pressed[GLFW_KEY_A]) {
            vel.dir += right; // Move right
        }

        if (key_pressed[GLFW_KEY_LEFT_CONTROL] &&
            !key_pressed[GLFW_KEY_SPACE]) {
            vel.dir.y = -1.0F; // Move down
        }
        else if (key_pressed[GLFW_KEY_SPACE] &&
                 !key_pressed[GLFW_KEY_LEFT_CONTROL]) {
            vel.dir.y = 1.0F; // Move up
        }
        break;
    }
    }
}
