#include <mb/game.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <mb/components.h>
#include <mb/dialog.h>
#include <mb/events.h>
#include <mb/font.h>
#include <mb/generate-mesh.h>
#include <mb/get-terrain-height.h>
#include <mb/helpers.h>
#include <mb/lights.h>
#include <mb/model.h>
#include <mb/systems.h>
#include <mb/texture.h>
#include <mb/town.h>
#include <mb/troop.h>

Game::Game(int width, int height)
    : width_{width}, height_{height},
      proj_{glm::perspective(
          glm::radians(45.F),
          static_cast<float>(width) / static_cast<float>(height), .1F, 300.F)},
      shader_("./shader/main.vert", "./shader/main.frag"),
      light_cube_shader_("./shader/main.vert", "./shader/light.frag"),
      font_shader_("./shader/font.vert", "./shader/font.frag"),
      font_("./resources/MonaspaceNeon-Regular.otf"),
      ui_(width, height, &font_, &font_shader_)
{
    windowresize_input(width, height);
}

void Game::init_world()
{
    auto &reg = registry_;

    registry_.ctx().emplace<Game_state>(Game_state::Normal);

    auto cube = generate_cube_model();
    auto [terrain_model, height_map] = generate_terrain_model(100, 100, 0.05F);
    height_map_ = height_map;
    auto vex = std::make_shared<Model>("./resources/vex.glb");
    auto yen = std::make_shared<Model>("./resources/yen.glb");

    // Init camere
    {
        auto e = reg.create();
        reg.emplace<Camera>(e, Camera{.yaw = std::numbers::pi / 2,
                                      .pitch = -std::numbers::pi / 3,
                                      .is_active = false});
        reg.emplace<Position>(e, Position{.value = {45, 80, 100}});
        reg.emplace<Velocity>(e, Velocity{.dir = {}, .speed = 30});
        reg.emplace<View_mode>(e, View_mode::God);
    }
    entt::entity fpscam;
    {
        auto e = reg.create();
        fpscam = e;
        reg.emplace<Fps_camemra_tag>(e);
        reg.emplace<Camera>(
            e,
            Camera{.yaw = std::numbers::pi / 2, .pitch = 0, .is_active = true});
        reg.emplace<Position>(e, Position{.value = {29, 18, 50}});
        reg.emplace<Velocity>(e, Velocity{.dir = {}, .speed = 5});
        reg.emplace<View_mode>(e, View_mode::First_player);
    }

    // Init light cube
    {
        auto e = reg.create();
        reg.emplace<Position>(e, glm::vec3{30, 20, 40});
        reg.emplace<Renderable>(
            e, Renderable{.model = cube, .shader = &light_cube_shader_});
    }
    { // Init directional light
        auto e = reg.create();
        reg.emplace<Directional_light>(e,
                                       Directional_light{.dir = {-1, -3, 2}});
        reg.emplace<Light>(e, Light{.ambient = glm::vec3{0.1},
                                    .diffuse = glm::vec3{0.5},
                                    .specular = glm::vec3{0.5}});
    }
    { // Init point light
        auto e = reg.create();
        reg.emplace<Point_light>(e, Point_light{.constant = 1.0F,
                                                .linear = 0.09F,
                                                .quadratic = 0.032F});
        reg.emplace<Position>(e, glm::vec3{30, 20, 40});
        reg.emplace<Light>(e, Light{.ambient = glm::vec3{0.1},
                                    .diffuse = glm::vec3{0.5},
                                    .specular = glm::vec3{1}});
    }
    { // Init spot light
        auto e = reg.create();
        reg.emplace<Spot_light>(
            e, Spot_light{.constant = 1,
                          .linear = 0.045,
                          .quadratic = 0.0075,
                          .dir = reg.get<Camera>(fpscam).front(),
                          .cut_off = glm::cos(glm::radians(12.F)),
                          .outer_cut_off = glm::cos(glm::radians(20.F))});
        reg.emplace<Position>(e, reg.get<Position>(fpscam));
        reg.emplace<Light>(e, Light{.ambient = glm::vec3{0.1},
                                    .diffuse = glm::vec3{0.8},
                                    .specular = glm::vec3{1}});
    }

    { // Init `me`
        auto e = reg.create();
        reg.emplace<Local_player_tag>(e);
        reg.emplace<Position>(
            e, glm::vec3{28, get_terrain_height(height_map_, 28, 47), 47});
        reg.emplace<Velocity>(e, Velocity{.dir = {0., 0., 0.}, .speed = 25});
        std::vector<Troop_stack> myarmy;
        myarmy.push_back(Troop_stack{.size = 1, .troop_id = -1UZ});
        reg.emplace<Army>(e, Army{.stacks = myarmy});
        reg.emplace<Renderable>(e,
                                Renderable{.model = vex, .shader = &shader_});
        reg.emplace<Transform>(e, Transform{.scale = glm::vec3(0.03)});
    }

    // Init armies
    {
        std::random_device rd;
        std::mt19937 gen(rd());

        // 随机位置范围
        std::uniform_real_distribution<float> pos_x(0, 100);
        std::uniform_real_distribution<float> pos_z(0, 100);

        // 随机队伍规模
        std::uniform_int_distribution<int> troop_size(1, 5);

        for (int i{}; i != 1; ++i) {
            auto e = reg.create();
            reg.emplace<Ai_tag>(e);

            std::vector<Troop_stack> army;
            std::size_t size = troop_size(gen);
            army.push_back(Troop_stack{.size = size, .troop_id = -1UZ});
            reg.emplace<Army>(e, Army{.stacks = army});

            glm::vec3 pos{pos_x(gen), 0, pos_z(gen)};
            pos.y = get_terrain_height(height_map_, pos.x, pos.z);
            reg.emplace<Position>(e, pos);

            reg.emplace<Velocity>(e, Velocity{.dir = {}, .speed = 20.0f});

            reg.emplace<Renderable>(
                e, Renderable{.model = yen, .shader = &shader_});

            reg.emplace<Transform>(e, Transform{.scale = glm::vec3(0.03)});
        }
    }
    { // Init towns
        auto e = reg.create();
        reg.emplace<Town>(e, Town{.money = 0});
        reg.emplace<Position>(
            e, Position{
                   .value = {30, get_terrain_height(height_map_, 30, 40), 40}});
        reg.emplace<Renderable>(e,
                                Renderable{.model = cube, .shader = &shader_});
        reg.emplace<Transform>(e, Transform{.scale = glm::vec3(8)});
    }

    { // Init terrain
        auto e = reg.create();
        reg.emplace<Renderable>(
            e, Renderable{.model = terrain_model, .shader = &shader_});
        reg.emplace<Position>(e, glm::vec3{0.0F, 0.0F, 0.0F});
    }

    { // Init dialog
        auto e = reg.create();

        auto fuck = [this]() { spdlog::info("ljf sb"); };
        comp::Dialog_option fuck_option{.reply = "Fuck", .action = fuck};
        auto exit = [this]() {
            registry_.ctx().get<Game_state>() = Game_state::Normal;
        };
        comp::Dialog_option exit_option{.reply = "Exit", .action = exit};
        reg.emplace<comp::Dialog>(
            e, comp::Dialog{.scripts = {"FWW shi shaluan ma?", "YES, he is!"},
                            .current_line = 0,
                            .is_active = true,
                            .options = {fuck_option, exit_option}});
    }
}

void Game::main_loop(GLFWwindow *window)
{
    double last_frame = glfwGetTime();

    dispatcher_.sink<Collision_event>().connect<process_collision_event>();

    spdlog::info("Entering main loop...");
    // When send close command to window, glfwWindowShouldClose will return
    // true NOLINTNEXTLINE(readability-implicit-bool-conversion)
    while (registry_.ctx().get<Game_state>() != Game_state::Should_exit &&
           glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();
        glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // double now = glfwGetTime();
        // double dt = now - last_frame;
        auto now = static_cast<float>(glfwGetTime());
        auto dt = static_cast<float>(now - last_frame);
        last_frame = now;

        switch (registry_.ctx().get<Game_state>()) {
        case Game_state::Normal:
            normal(window, dt);
            break;
        case Game_state::In_dialog:
            in_dialog(window);
            break;
        case Game_state::Should_exit:
            break;
        }

        render_system(registry_, proj_);
        { // Show FPS
            static double accumu{};
            accumu += dt;
            static double fps = 0;
            if (accumu >= 1) {
                fps = 1. / dt;
                spdlog::trace("fps={}", fps);
                accumu = 0;
            }
            ui_.render_text(std::format("fps={:.0f}", fps), {0, 0}, 1,
                            {1, 1, 1});
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    spdlog::info("Exited from main loop");
}

void Game::cursorpos_input(double xpos, double ypos)
{
    // Initialize first cursor_pos_ to its correct value. Otherwise,
    // pos_delta could be very large in the first cursor moving frame.
    if (cursor_pos_ == glm::vec2{-1, -1}) {
        cursor_pos_ = {xpos, ypos};
    }

    cursor_pos_delta_ = glm::vec2{xpos, ypos} - cursor_pos_;
    spdlog::debug("cursor dx={} dy={}", cursor_pos_delta_.x,
                  cursor_pos_delta_.y);
    cursor_pos_ = {xpos, ypos};

    auto state = registry_.ctx().get<Game_state>();
    switch (state) {
    case Game_state::Normal: {
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
            constexpr float sensitivity{0.03 * std::numbers::pi /
                                        180}; // 1 degree
            auto &cam{registry_.get<Camera>(cam_entity)};
            cam.yaw -= sensitivity * cursor_pos_delta_.x;
            cam.pitch -= sensitivity * cursor_pos_delta_.y;
            cam.pitch =
                glm::clamp<float>(cam.pitch, (-std::numbers::pi / 2) + 1e-5,
                                  (std::numbers::pi / 2) - 1e-5);
            break;
        }
        }
        break;
    }
    case Game_state::In_dialog:
        break;
    case Game_state::Should_exit:
        break;
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
    auto &state = registry_.ctx().get<Game_state>();
    switch (state) {
    case Game_state::Normal: {
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
                        glm::vec3{
                            hit.x,
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
        break;
    }
    case Game_state::In_dialog:
    case Game_state::Should_exit:
        break;
    }
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
    if (key == GLFW_KEY_ESCAPE) {
        registry_.ctx().get<Game_state>() = Game_state::Should_exit;
        return;
    }
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

void Game::normal(GLFWwindow *window, float dt)
{
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
    town_script(registry_, dt);
    movement_system(registry_, dt, height_map_);
    collision_system(registry_, dispatcher_, dt);
    collision_script(registry_, dispatcher_);
    perception_system(registry_);
    ai_system(registry_, dt);
}

void Game::in_dialog(GLFWwindow *window)
{
    auto dialogs = registry_.view<comp::Dialog>();
    for (auto [e, dialog] : dialogs.each()) {
        if (dialog.is_active) {
            ImGui::Begin("dialog : ");
            ImGui::Text("%s", dialog.scripts[0].c_str());
            for (auto const &option : dialog.options) {
                if (ImGui::Button(option.reply.c_str())) {
                    option.action();
                }
            }
            ImGui::End();
        }
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
}
