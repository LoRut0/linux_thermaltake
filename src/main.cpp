#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "controllers.hpp"

int main() {
    spdlog::set_level(spdlog::level::info);

    std::string path = "config.yml";
    ThermaltakeController controller(Config::loadDevices(path).size(), path);
    controller.start();
    return 0;
}
