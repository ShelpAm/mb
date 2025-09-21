#include <mb/app.h>
#include <spdlog/spdlog.h>

int main()
{
    // try {
    App::initialize();
    App app(1920, 1080, "Mount & Blade Clone");
    app.run();
    App::deinitialize();
    // }
    // catch (std::exception const &e) {
    //     spdlog::critical("{}", e.what());
    //     return EXIT_FAILURE;
    // }
    return EXIT_SUCCESS;
}
