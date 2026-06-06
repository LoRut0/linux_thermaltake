#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include "controllers.hpp"

int main() {
    spdlog::set_level(spdlog::level::info);

    std::string path = "/usr/local/etc/thermaltake-controller/config.yml";
    ThermaltakeController controller(Config::loadDevices(path).size(), path);
    controller.start();
    return 0;
}
