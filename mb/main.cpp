#include <mb/app.h>
#include <spdlog/spdlog.h>

int main()
{
    try {
        App app(1280, 720, "Mount & Blade Clone");
        app.run();
    }
    catch (std::exception const &e) {
        spdlog::error("Fatal error: {}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
