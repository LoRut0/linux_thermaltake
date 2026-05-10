#pragma once

#include <spdlog/spdlog.h>

#include <optional>

#include "globals.hpp"
#include "spec.hpp"

struct FanSpeed {
    std::uint8_t set_speed{};
    std::uint16_t rpm{};
};

class ThermaltakeDevice {
  public:
    ThermaltakeDevice(std::string_view& model) : spec_(find_spec_(model)) {
        spdlog::debug("successfully created device {}", model);
    }

    std::string_view model_() const { return spec_.model; }

    std::uint8_t num_leds_() const { return spec_.num_leds; }

    std::uint8_t index_per_led_() const { return spec_.index_per_led; }

    bool has_rgb_() const { return spec_.has_rgb; }

    bool has_fan_() const { return spec_.has_fan; }

  private:
    static const DeviceSpec find_spec_(const std::string_view model) {
        auto equals_ignore_case = [](std::string_view a, std::string_view b) {
            return a.size() == b.size() &&
                   std::equal(
                       a.begin(), a.end(), b.begin(), [](char x, char y) {
                           return std::tolower(static_cast<unsigned char>(x)) ==
                                  std::tolower(static_cast<unsigned char>(y));
                       });
        };

        for (const auto& spec : kDeviceSpecs) {
            if (equals_ignore_case(spec.model, model)) {
                return spec;
            }
        }
        spdlog::error("Didnt find requested model, creating default device");
        return kDeviceSpecs[0];
    }

    const DeviceSpec spec_;
};
