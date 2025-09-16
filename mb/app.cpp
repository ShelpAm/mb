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
      view_{glm::lookAt(glm::vec3{30, 60, 30}, {30, 0, 30},
                        // glm::vec3{0, 50, 0}, {0, 0, 0},
                        {0, 0, -1})},
      proj_{
          glm::perspective(glm::radians(45.F), fwidth_ / fheight_, znear, zfar)}
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Initializing app...");
    init_window(width, height, title);
    init_opengl();
    init_entities();
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
    // Because we turned on on-demand option, opengl will be initialized
    // implicitly.
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        spdlog::error("Failed to initialize OpenGL context\n");
    }

    // Successfully loaded OpenGL
    spdlog::info("Loaded OpenGL {}.{}\n", GLAD_VERSION_MAJOR(version),
                 GLAD_VERSION_MINOR(version));
}

void App::main_loop()
{
    std::vector<float> vertices{-1, 0, -1, 1, 0, 1, 0, 0, 1};
    std::vector<std::uint32_t> indices{0, 1, 2};
    Mesh mesh(vertices, indices);
    Shader_program shader("./shader/main.vert", "./shader/main.frag");
    double last_frame = glfwGetTime();

    auto &registry = World::registry();
    auto me = World::me();

    spdlog::info("Entering main loop...");
    while (glfwWindowShouldClose(window_) == 0) {
        glfwPollEvents();

        double now = glfwGetTime();
        double dt = now - last_frame;
        last_frame = now;
        World::ai_system(dt);
        World::movement(dt);

        glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 model(1.);
        model = glm::scale(model, glm::vec3{1, 2, 1});
        model = glm::translate(model, registry.get<Position>(me).value);
        auto mvp = proj_ * view_ * model;
        shader.uniform_mat4("uMVP", mvp);
        shader.uniform_1f("time", static_cast<float>(now));
        mesh.render(shader);

        glfwSwapBuffers(window_);
    }
    spdlog::info("Exited from main loop");
}

void App::run()
{
    main_loop();
}

void App::init_entities() {}
