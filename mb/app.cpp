#include <mb/app.h>

#include <mb/callbacks.h>
#include <mb/mesh.h>
#include <mb/world.h>

#include <glad/gl.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

App::App(int width, int height, std::string const &title)
    : width_(width), height_(height), fwidth_{static_cast<float>(width)},
      fheight_{static_cast<float>(height)}, znear{0.1F}, zfar{100.F},
      view_{glm::lookAt(glm::vec3{45, 80, 30}, {45, 0, 30},
                        // glm::vec3{0, 50, 0}, {0, 0, 0},
                        {0, 0, -1})},
      proj_{glm::perspective(fovy_, fwidth_ / fheight_, znear, zfar)},
      me_{World::registry().create()}
{
    spdlog::set_level(spdlog::level::info);
    spdlog::info("Initializing app...");
    init_window(width, height, title);
    init_opengl();
    shader_ = std::make_unique<Shader_program>("./shader/main.vert",
                                               "./shader/main.frag");
    init_entities(World::registry());
    spdlog::info("Initialized app");
}

App::~App()
{
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void App::init_window(int width, int height, std::string const &title)
{
    if (glfwInit() == 0) {
        throw std::runtime_error("Failed to init GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (window_ == nullptr) {
        throw std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(window_); // OpenGL context
    glfwSwapInterval(1);             // V-sync

    glfwSetWindowUserPointer(window_, this);
    glfwSetCursorPosCallback(window_, cursorpos_callback);
    glfwSetMouseButtonCallback(window_, mousebutton_callback);
}

void App::init_opengl()
{
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        spdlog::error("Failed to initialize OpenGL context\n");
    }
    // Successfully loaded OpenGL
    spdlog::info("Loaded OpenGL {}.{}", GLAD_VERSION_MAJOR(version),
                 GLAD_VERSION_MINOR(version));
}

void App::main_loop()
{
    double last_frame = glfwGetTime();
    auto &registry = World::registry();

    spdlog::info("Entering main loop...");
    while (glfwWindowShouldClose(window_) == 0) {
        glfwPollEvents();

        double now = glfwGetTime();
        double dt = now - last_frame;
        last_frame = now;
        auto fps = 1. / dt;
        spdlog::debug("fps={}", fps);
        ai_system(registry, static_cast<float>(dt));
        movement_system(registry, static_cast<float>(dt));
        collision_system(registry, static_cast<float>(now));

        glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_system(registry, static_cast<float>(now), view_, proj_);
        glfwSwapBuffers(window_);
    }
    spdlog::info("Exited from main loop");
}

void App::run()
{
    main_loop();
}
void App::init_entities(entt::registry &reg)
{
    std::vector<float> vertices{-1, 0, -1, 1, 0, 1, 0, 0, 1};
    std::vector<std::uint32_t> indices{0, 1, 2};
    auto triangle = std::make_shared<Mesh>(vertices, indices);

    // Init me
    reg.emplace<Position>(me_, glm::vec3{0., 0., 0.});
    reg.emplace<Velocity>(me_, Velocity{.dir = {0., 0., 0.}, .speed = 25});
    std::vector<Troop_stack> myarmy;
    myarmy.push_back(Troop_stack{.size = 1, .troop_id = -1UZ});
    reg.emplace<Army>(me_, Army{.stacks = myarmy});
    reg.emplace<Renderable>(
        me_, Renderable{.mesh = triangle, .shader = shader_.get()});

    // Init armies
    auto entity = reg.create();
    reg.emplace<Ai_tag>(entity);
    std::vector<Troop_stack> army;
    army.push_back(Troop_stack{.size = 3, .troop_id = -1UZ});
    reg.emplace<Army>(entity, Army{.stacks = army});
    reg.emplace<Position>(entity, glm::vec3{10, 0, 15});
    reg.emplace<Velocity>(entity, Velocity{.dir = {0., 0., 0.}, .speed = 20});
    reg.emplace<Renderable>(
        entity, Renderable{.mesh = triangle, .shader = shader_.get()});
}

static glm::vec3 screen_to_world(float x_screen, float y_screen, float depth,
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

void App::input(int button, int action, int mods)
{
    glm::vec3 ray_start = screen_to_world(cursor_pos_.x, cursor_pos_.y, 0.0F,
                                          width_, height_, view_, proj_);
    glm::vec3 ray_end = screen_to_world(cursor_pos_.x, cursor_pos_.y, 1.0F,
                                        width_, height_, view_, proj_);
    glm::vec3 ray_dir = glm::normalize(ray_end - ray_start);

    // Intersect with ground plane y=0
    float t = -ray_start.y / ray_dir.y;
    glm::vec3 hit = ray_start + t * ray_dir;
    World::registry().emplace_or_replace<Pathing>(me_, hit);
}
