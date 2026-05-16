#pragma once

#include <spdlog/spdlog.h>

#include <memory>
#include <string_view>

#include "devices.hpp"
#include "driver.hpp"
#include "globals.hpp"
#include "lighting_manager.hpp"

class ThermaltakeController {
  private:
    std::vector<std::unique_ptr<ThermaltakeDevice>> devices_;
    std::vector<std::unique_ptr<LightingEffect>> effects_;
    ThermaltakeControllerDriver driver_;
    uint8_t ports_;
    uint8_t unit_;
    std::string_view model_;

    void set_lighting_(const std::vector<std::uint8_t>& values,
                       const std::uint8_t port, const std::uint8_t mode = 0x18,
                       const std::uint8_t speed = 0x00) {
        std::vector<std::uint8_t> data{kPROTOCOL_SET, kPROTOCOL_LIGHT, port,
                                       static_cast<std::uint8_t>(mode + speed)};

        data.insert(data.end(), values.begin(), values.end());

        spdlog::debug(
            "fan on port: {} set lightning with hex {0:x} {0:x} {0:x}", port,
            values[0], values[1], values[2]);

        driver_.write_out(data);
    }

    void set_fan_speed_(const std::uint8_t port, const std::uint8_t speed) {
        std::vector<std::uint8_t> data{0x00, kPROTOCOL_SET, kPROTOCOL_FAN,
                                       port, 0x01,          speed};
        spdlog::debug("fan on port: {} set speed with hex {0:x}", port, speed);
        driver_.write_out(data);
    }

    FanSpeed get_fan_speed_(const std::uint8_t port) {
        std::vector<std::uint8_t> data{kPROTOCOL_GET, kPROTOCOL_FAN, port};

        driver_.write_out(data);

        auto response = driver_.read_in();

        if (response.size() < 7) {
            throw std::runtime_error("invalid fan speed response");
        }

        std::uint8_t speed = response[4];
        std::uint8_t rpm_l = response[5];
        std::uint8_t rpm_h = response[6];

        std::uint16_t rpm = static_cast<std::uint16_t>((rpm_h << 8) | rpm_l);

        return FanSpeed{.set_speed = speed, .rpm = rpm};
    }

  public:
    ThermaltakeController(const uint8_t num_of_ports, const uint8_t unit = 0,
                          std::string_view model = "")
        : devices_(num_of_ports),
          effects_(num_of_ports),
          ports_(num_of_ports),
          unit_(unit),
          model_(model) {}

    void attach_device(const uint8_t port, std::string_view model) {
        devices_[port] = std::make_unique<ThermaltakeDevice>(model);
        return;
    }

    void attach_effect(const uint8_t port, std::string_view effect) {
        if (!devices_[port].get())
            throw std::logic_error(
                "Device was not set before attaching effect");
        Type_ effect_type = str_to_type(effect);
        switch (effect_type) {
            case Type_::Full: {
                uint8_t r, g, b;
                std::unique_ptr<LightingEffect> effect_ptr =
                    std::make_unique<Full>(devices_[port].get(), r, g, b);
                effects_[port] = std::move(effect_ptr);
            }
        }
        return;
    }

    bool start() {}
};
