#include <mb/game.h>

#include <mb/components.h>
#include <mb/generate-terrain-mesh.h>
#include <mb/systems.h>
#include <mb/troop.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

inline std::shared_ptr<Mesh> generate_character_mesh()
{
    std::vector<float> vertices;
    std::vector<std::uint32_t> indices;

    // Humanoid dimensions (in model-space, scaled for terrain)
    float headSize = 0.3f; // Head cube size
    float torsoHeight = 0.8f, torsoWidth = 0.4f, torsoDepth = 0.2f; // Torso
    float armLength = 0.7f, armWidth = 0.15f;                       // Arms
    float legLength = 0.9f, legWidth = 0.15f;                       // Legs
    float totalHeight = headSize + torsoHeight + legLength; // ~2.0 units

    // Base position (centered at origin, feet at y=0)
    float yBase = 0.0f;

    // Head (cube centered at y = torsoHeight + legLength + headSize/2)
    float headY = yBase + legLength + torsoHeight + headSize / 2.0f;
    std::vector<glm::vec3> headPos = {
        {-headSize / 2, headY - headSize / 2,
         -headSize / 2}, // 0: Bottom-left-front
        {headSize / 2, headY - headSize / 2,
         -headSize / 2}, // 1: Bottom-right-front
        {headSize / 2, headY + headSize / 2,
         -headSize / 2}, // 2: Top-right-front
        {-headSize / 2, headY + headSize / 2,
         -headSize / 2}, // 3: Top-left-front
        {-headSize / 2, headY - headSize / 2,
         headSize / 2}, // 4: Bottom-left-back
        {headSize / 2, headY - headSize / 2,
         headSize / 2}, // 5: Bottom-right-back
        {headSize / 2, headY + headSize / 2, headSize / 2}, // 6: Top-right-back
        {-headSize / 2, headY + headSize / 2, headSize / 2} // 7: Top-left-back
    };

    // Torso (rectangular prism)
    float torsoY = yBase + legLength + torsoHeight / 2.0f;
    std::vector<glm::vec3> torsoPos = {
        {-torsoWidth / 2, torsoY - torsoHeight / 2, -torsoDepth / 2}, // 8
        {torsoWidth / 2, torsoY - torsoHeight / 2, -torsoDepth / 2},  // 9
        {torsoWidth / 2, torsoY + torsoHeight / 2, -torsoDepth / 2},  // 10
        {-torsoWidth / 2, torsoY + torsoHeight / 2, -torsoDepth / 2}, // 11
        {-torsoWidth / 2, torsoY - torsoHeight / 2, torsoDepth / 2},  // 12
        {torsoWidth / 2, torsoY - torsoHeight / 2, torsoDepth / 2},   // 13
        {torsoWidth / 2, torsoY + torsoHeight / 2, torsoDepth / 2},   // 14
        {-torsoWidth / 2, torsoY + torsoHeight / 2, torsoDepth / 2}   // 15
    };

    // Left Arm (right from character’s perspective)
    float armLY = yBase + legLength + torsoHeight - armLength / 2.0f;
    std::vector<glm::vec3> leftArmPos = {
        {torsoWidth / 2, armLY - armLength / 2, -armWidth / 2},            // 16
        {torsoWidth / 2 + armWidth, armLY - armLength / 2, -armWidth / 2}, // 17
        {torsoWidth / 2 + armWidth, armLY + armLength / 2, -armWidth / 2}, // 18
        {torsoWidth / 2, armLY + armLength / 2, -armWidth / 2},            // 19
        {torsoWidth / 2, armLY - armLength / 2, armWidth / 2},             // 20
        {torsoWidth / 2 + armWidth, armLY - armLength / 2, armWidth / 2},  // 21
        {torsoWidth / 2 + armWidth, armLY + armLength / 2, armWidth / 2},  // 22
        {torsoWidth / 2, armLY + armLength / 2, armWidth / 2}              // 23
    };

    // Right Arm (left from character’s perspective)
    float armRY = armLY;
    std::vector<glm::vec3> rightArmPos = {
        {-torsoWidth / 2 - armWidth, armRY - armLength / 2,
         -armWidth / 2},                                         // 24
        {-torsoWidth / 2, armRY - armLength / 2, -armWidth / 2}, // 25
        {-torsoWidth / 2, armRY + armLength / 2, -armWidth / 2}, // 26
        {-torsoWidth / 2 - armWidth, armRY + armLength / 2,
         -armWidth / 2},                                                   // 27
        {-torsoWidth / 2 - armWidth, armRY - armLength / 2, armWidth / 2}, // 28
        {-torsoWidth / 2, armRY - armLength / 2, armWidth / 2},            // 29
        {-torsoWidth / 2, armRY + armLength / 2, armWidth / 2},            // 30
        {-torsoWidth / 2 - armWidth, armRY + armLength / 2, armWidth / 2}  // 31
    };

    // Left Leg
    float legLY = yBase + legLength / 2.0f;
    std::vector<glm::vec3> leftLegPos = {
        {torsoWidth / 4 - legWidth / 2, legLY - legLength / 2,
         -legWidth / 2}, // 32
        {torsoWidth / 4 + legWidth / 2, legLY - legLength / 2,
         -legWidth / 2}, // 33
        {torsoWidth / 4 + legWidth / 2, legLY + legLength / 2,
         -legWidth / 2}, // 34
        {torsoWidth / 4 - legWidth / 2, legLY + legLength / 2,
         -legWidth / 2}, // 35
        {torsoWidth / 4 - legWidth / 2, legLY - legLength / 2,
         legWidth / 2}, // 36
        {torsoWidth / 4 + legWidth / 2, legLY - legLength / 2,
         legWidth / 2}, // 37
        {torsoWidth / 4 + legWidth / 2, legLY + legLength / 2,
         legWidth / 2}, // 38
        {torsoWidth / 4 - legWidth / 2, legLY + legLength / 2, legWidth / 2}
        // 39
    };

    // Right Leg
    float legRY = legLY;
    std::vector<glm::vec3> rightLegPos = {
        {-torsoWidth / 4 - legWidth / 2, legRY - legLength / 2,
         -legWidth / 2}, // 40
        {-torsoWidth / 4 + legWidth / 2, legRY - legLength / 2,
         -legWidth / 2}, // 41
        {-torsoWidth / 4 + legWidth / 2, legRY + legLength / 2,
         -legWidth / 2}, // 42
        {-torsoWidth / 4 - legWidth / 2, legRY + legLength / 2,
         -legWidth / 2}, // 43
        {-torsoWidth / 4 - legWidth / 2, legRY - legLength / 2,
         legWidth / 2}, // 44
        {-torsoWidth / 4 + legWidth / 2, legRY - legLength / 2,
         legWidth / 2}, // 45
        {-torsoWidth / 4 + legWidth / 2, legRY + legLength / 2,
         legWidth / 2}, // 46
        {-torsoWidth / 4 - legWidth / 2, legRY + legLength / 2, legWidth / 2}
        // 47
    };

    // Combine all positions
    std::vector<glm::vec3> allPos;
    allPos.insert(allPos.end(), headPos.begin(), headPos.end());
    allPos.insert(allPos.end(), torsoPos.begin(), torsoPos.end());
    allPos.insert(allPos.end(), leftArmPos.begin(), leftArmPos.end());
    allPos.insert(allPos.end(), rightArmPos.begin(), rightArmPos.end());
    allPos.insert(allPos.end(), leftLegPos.begin(), leftLegPos.end());
    allPos.insert(allPos.end(), rightLegPos.begin(), rightLegPos.end());

    // Generate vertices (x, y, z only)
    for (auto const &pos : allPos) {
        vertices.push_back(pos.x); // X
        vertices.push_back(pos.y); // Y
        vertices.push_back(pos.z); // Z
    }

    // Generate indices (6 faces per cube, 2 triangles per face)
    auto addCubeIndices = [&](uint32_t base) {
        // Front (0, 1, 2, 3)
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
        // Back (4, 5, 6, 7)
        indices.push_back(base + 4);
        indices.push_back(base + 6);
        indices.push_back(base + 5);
        indices.push_back(base + 4);
        indices.push_back(base + 7);
        indices.push_back(base + 6);
        // Left (0, 3, 7, 4)
        indices.push_back(base + 0);
        indices.push_back(base + 3);
        indices.push_back(base + 7);
        indices.push_back(base + 0);
        indices.push_back(base + 7);
        indices.push_back(base + 4);
        // Right (1, 5, 6, 2)
        indices.push_back(base + 1);
        indices.push_back(base + 6);
        indices.push_back(base + 5);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 6);
        // Top (3, 2, 6, 7)
        indices.push_back(base + 3);
        indices.push_back(base + 2);
        indices.push_back(base + 6);
        indices.push_back(base + 3);
        indices.push_back(base + 6);
        indices.push_back(base + 7);
        // Bottom (0, 4, 5, 1)
        indices.push_back(base + 0);
        indices.push_back(base + 5);
        indices.push_back(base + 4);
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 5);
    };

    // Add indices for each body part
    addCubeIndices(0);  // Head
    addCubeIndices(8);  // Torso
    addCubeIndices(16); // Left Arm
    addCubeIndices(24); // Right Arm
    addCubeIndices(32); // Left Leg
    addCubeIndices(40); // Right Leg

    return std::make_shared<Mesh>(vertices, indices);
}

void load_texture(char const *path, GLuint &texture)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    unsigned char *data = stbi_load(path, &width, &height, &channels, 0);
    if (data) {
        GLenum format = channels == 3 ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else {
        spdlog::error("Can't find {}", path);
    }
}

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

    std::vector<float> vertices{
        -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f, -0.5f,
        0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
        0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, 0.5f,  -0.5f,
        0.0f,  0.0f,  -1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

        -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,
        0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  -0.5f, 0.5f,  0.5f,
        0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f,
        -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, 0.5f,
        -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
        1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
        0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,
        1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, -0.5f,
        0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
        0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, 0.5f,
        0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,

        -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
        0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,
        0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f};
    std::vector<std::uint32_t> indices(vertices.size() / 6);
    std::ranges::iota(indices, 0);
    auto cube = std::make_shared<Mesh>(vertices, indices);

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
    {
        auto cam_entity = reg.create();
        reg.emplace<Camera>(
            cam_entity,
            Camera{.yaw = std::numbers::pi / 2, .pitch = 0, .is_active = true});
        reg.emplace<Position>(cam_entity, Position{.value = {-5, 0, 10}});
        reg.emplace<Velocity>(cam_entity, Velocity{.dir = {}, .speed = 5});
        reg.emplace<View_mode>(cam_entity, View_mode::First_player);
    }

    // Init light cube
    // {
    //     auto light_cube = reg.create();
    //     reg.emplace<Position>(light_cube, glm::vec3{0, 10., 0.});
    //     reg.emplace<Renderable>(
    //         light_cube,
    //         Renderable{.mesh = cube, .shader = &light_cube_shader_, .});
    // }
    // {
    //     auto light_cube = reg.create();
    //     reg.emplace<Position>(light_cube, glm::vec3{-3, 2, -5.});
    //     reg.emplace<Renderable>(
    //         light_cube,
    //         Renderable{.mesh = cube, .shader = &light_cube_shader_});
    // }

    // Init `me`
    // auto me = reg.create();
    // reg.emplace<Local_player_tag>(me);
    // reg.emplace<Position>(me, glm::vec3{-5, 0., 0.});
    // reg.emplace<Velocity>(me, Velocity{.dir = {0., 0., 0.}, .speed = 25});
    // std::vector<Troop_stack> myarmy;
    // myarmy.push_back(Troop_stack{.size = 1, .troop_id = -1UZ});
    // reg.emplace<Army>(me, Army{.stacks = myarmy});
    // reg.emplace<Renderable>(
    //     me, Renderable{.mesh = cube, .shader = &light_cube_shader_});

    // Init armies
    // auto entity = reg.create();
    // reg.emplace<Ai_tag>(entity);
    // std::vector<Troop_stack> army;
    // army.push_back(Troop_stack{.size = 3, .troop_id = -1UZ});
    // reg.emplace<Army>(entity, Army{.stacks = army});
    // reg.emplace<Position>(entity, glm::vec3{10, 0, 15});
    // reg.emplace<Velocity>(entity, Velocity{.dir = {0., 0., 0.}, .speed =
    // 20}); reg.emplace<Renderable>(entity,
    //                         Renderable{.mesh = cube, .shader = &shader_});

    // Init terrain FIXME: Normal vectors needed
    auto [terrain_mesh, height] = generate_terrain_mesh(100, 100, 0.05F);
    height_map_ = height;
    auto terrain_entity = reg.create();
    GLuint terrain_texture;
    load_texture("/home/shelpam/Pictures/wjz.jpg", terrain_texture);
    reg.emplace<Renderable>(terrain_entity,
                            Renderable{.mesh = terrain_mesh,
                                       .shader = &shader_,
                                       .texture = terrain_texture});
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
        constexpr float sensitivity{0.1 * std::numbers::pi / 180}; // 1 degree
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

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        glm::vec3 ray_start = screen_to_world(
            cursor_pos_.x, cursor_pos_.y, 0.0F, width_, height_, view, proj_);
        glm::vec3 ray_end = screen_to_world(cursor_pos_.x, cursor_pos_.y, 1.0F,
                                            width_, height_, view, proj_);
        glm::vec3 ray_dir = glm::normalize(ray_end - ray_start);

        // Intersect with ground plane y=0
        float t = -ray_start.y / ray_dir.y;
        glm::vec3 hit = ray_start + t * ray_dir;
        for (auto [me] : registry_.view<Local_player_tag>().each()) {
            registry_.emplace_or_replace<Pathing>(
                me, glm::vec3{hit.x,
                              height_map_[int(hit.z) % height_map_.size()]
                                         [int(hit.x) % height_map_[0].size()] +
                                  2,
                              hit.z});
        }
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
        auto cam = get_active_camera(registry_);
        registry_.get<Position>(cam).value += glm::vec3{0, -3 * yoffset, 0};
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

entt::registry &Game::registry()
{
    return registry_;
}
