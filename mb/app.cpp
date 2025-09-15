#include <mb/app.h>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

App::App(int width, int height, std::string const &title)
{
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
}

void App::init_opengl()
{
    // Because we turned on on-demand option, opengl will be initialized
    // implicitly.
}

void App::main_loop()
{
    double last_frame = glfwGetTime();

    spdlog::info("Entering main loop...");
    while (glfwWindowShouldClose(window_) == 0) {
        glfwPollEvents();

        double now = glfwGetTime();
        double dt = now - last_frame;
        last_frame = now;
        ai_system(dt);

        glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window_);
    }
    spdlog::info("Exited from main loop");
}

void App::run()
{
    main_loop();
}

void App::init_entities()
{
    auto entity = registry_.create();
    registry_.emplace<Position>(entity, 1., 1., 2.);
}

void App::ai_system(double dt)
{
    auto view = registry_.view<Position>();
    for (auto [entity, pos] : view->each()) {
        pos.x += 1;
        spdlog::info("{}", pos.x);
    }
}
