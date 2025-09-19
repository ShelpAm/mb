#include <mb/app.h>

#include <mb/callbacks.h>
#include <mb/game.h>

#include <glad/gl.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

void App::initialize()
{
    spdlog::set_level(spdlog::level::info);

    spdlog::info("Initializing app...");
    if (glfwInit() == 0) {
        throw std::runtime_error("Failed to init GLFW");
    }

    constexpr auto opengl_version_major = 4;
    constexpr auto opengl_version_minor = 6;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_version_minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    spdlog::info("Initialized app");
}

void App::deinitialize()
{
    glfwTerminate();
}

App::App(int width, int height, std::string const &title)
    : window_{make_window(width, height, title)}, game_(width, height)
{
}

App::~App()
{
    glfwDestroyWindow(window_);
}

GLFWwindow *App::make_window(int width, int height, std::string const &title)
{
    spdlog::info("Initializing window...");
    GLFWwindow *window =
        glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("Failed to create window");
    }
    glfwMakeContextCurrent(window); // OpenGL context

    // GLAD
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        spdlog::error("Failed to initialize OpenGL context (glad)\n");
    }
    // Successfully loaded OpenGL
    spdlog::info("Loaded OpenGL {}.{}", GLAD_VERSION_MAJOR(version),
                 GLAD_VERSION_MINOR(version));

    glfwSwapInterval(1); // V-sync
    // 4x MSAA
    glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_DEPTH_TEST);

    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, cursorpos_callback);
    glfwSetMouseButtonCallback(window, mousebutton_callback);
    glfwSetFramebufferSizeCallback(window, framebuffersize_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    spdlog::info("Initialized window");
    return window;
}

void App::run()
{
    game_.init_world();
    game_.main_loop(window_);
}

void App::cursorpos_input(double xpos, double ypos)
{
    game_.cursorpos_input(xpos, ypos);
}

void App::mousebutton_input(int button, int action, int mods)
{
    game_.mousebutton_input(button, action, mods);
}

void App::framebuffersize_input(int width, int height)
{
    game_.windowresize_input(width, height);
}

void App::scroll_input(double xoffset, double yoffset)
{
    game_.scroll_input(xoffset, yoffset);
}

void App::key_input(int key, int scancode, int action, int mods)
{
    game_.key_input(key, scancode, action, mods);
}
