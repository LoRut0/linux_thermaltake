#pragma once

#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string_view>

#include "config.hpp"
#include "devices.hpp"
#include "driver.hpp"
#include "globals.hpp"
#include "lighting_manager.hpp"
#include "speed_manager.hpp"

class ThermaltakeController {
  private:
    std::atomic_bool running = true;
    std::vector<std::unique_ptr<ThermaltakeDevice>> devices_;
    std::vector<std::unique_ptr<LightingEffect>> effects_;
    std::vector<std::unique_ptr<SpeedManager>> speeds_;
    ThermaltakeControllerDriver driver_;
    uint8_t ports_;
    uint8_t unit_;
    std::string_view model_;  // FULL or SEPARATE
    std::string_view path_to_cfg_;

    void set_lighting_(const std::vector<std::uint8_t>& values,
                       const std::uint8_t port, const std::uint8_t mode = 0x18,
                       const std::uint8_t speed = 0x00) {
        std::vector<std::uint8_t> data{kPROTOCOL_SET, kPROTOCOL_LIGHT, port,
                                       static_cast<std::uint8_t>(mode + speed)};

        data.insert(data.end(), values.begin(), values.end());

        SPDLOG_DEBUG("fan on port: {} set lightning with hex {:x} {:x} {:x}",
                     port, values[0], values[1], values[2]);

        driver_.write_out(data);
    }

    void set_fan_speed_(const std::uint8_t port, const std::uint8_t speed) {
        std::vector<std::uint8_t> data{0x00, kPROTOCOL_SET, kPROTOCOL_FAN,
                                       port, 0x01,          speed};
        SPDLOG_DEBUG("fan on port: {} set speed with hex {}", port, speed);
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

    void attach_effect_() {
        LightingConfig cfg = Config::loadLightingConfig(path_to_cfg_.data());
        EffectType effect_type = str_to_effect_type(cfg.model);
        switch (effect_type) {
            case EffectType::Full: {
                std::unique_ptr<LightingEffect> effect_ptr =
                    std::make_unique<Full>(cfg.r, cfg.g, cfg.b);
                effects_[0] = std::move(effect_ptr);
            }
        }
        return;
    }

    void attach_speed_() {
        SpeedConfig cfg = Config::loadSpeedConfig(path_to_cfg_.data());
        SpeedType speed_type = str_to_speed_type(cfg.model);
        switch (speed_type) {
            case SpeedType::Locked: {
                std::unique_ptr<Locked> speed_ptr =
                    std::make_unique<Locked>(cfg.speed);
                speeds_[0] = std::move(speed_ptr);
                break;
            }
            case SpeedType::Temperature: {
                std::unique_ptr<Temperature> speed_ptr =
                    std::make_unique<Temperature>(cfg.curve, cfg.source_cpu,
                                                  cfg.source_gpu);
                speeds_[0] = std::move(speed_ptr);
                break;
            }
        }
    };

    void attach_device_() {
        std::vector<std::string> devices =
            Config::loadDevices(path_to_cfg_.data());
        for (size_t i = 0; i < devices.size(); ++i) {
            auto new_dev = std::make_unique<ThermaltakeDevice>(devices[i]);
            devices_[i] = std::move(new_dev);
        }
        return;
    }

    void full_loop_() {
        if (!speeds_[0]) throw std::runtime_error("No speed manager pointer");
        if (!effects_[0])
            throw std::runtime_error("No lightning manager pointer");
        int cycle = 0;
        while (running) {
            for (uint8_t i = 0; i < devices_.size(); ++i) {
                SPDLOG_DEBUG("write_out effect for fan on port {}", i);
                uint8_t* data = effects_[0]->next_frame(i);
                SPDLOG_TRACE("effect data: {:#x}",
                             fmt::join(data, data + 13, ", "));
                driver_.write_out(data);
                driver_.read_in(64);
            }
            if (cycle % 10 == 0) {
                for (uint8_t i = 0; i < devices_.size(); ++i) {
                    SPDLOG_DEBUG("write_out speed for fan on port {}", i);
                    uint8_t* data = speeds_[0]->get_speed(i);
                    SPDLOG_TRACE("effect data: {:#x}",
                                 fmt::join(data, data + 13, ", "));
                    driver_.write_out(data);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    driver_.read_in(64);
                }
                cycle = 0;
            }
            cycle++;
            std::this_thread::sleep_for(
                std::chrono::milliseconds(KEEPALIVE_INTERVAL));
        }
    }

    void separate_loop_() { throw std::runtime_error("Not implemented yet"); }

  public:
    ThermaltakeController(const uint8_t num_of_ports,
                          std::string_view path_to_cfg, const uint8_t unit = 0,
                          std::string_view model = "")
        : devices_(num_of_ports),
          effects_(num_of_ports),
          speeds_(num_of_ports),
          ports_(num_of_ports),
          unit_(unit),
          model_(model),
          path_to_cfg_(path_to_cfg) {
        driver_.init_controller();
        attach_device_();
        SPDLOG_DEBUG("devices attached");
        attach_effect_();
        SPDLOG_DEBUG("effects attached");
        attach_speed_();
        SPDLOG_DEBUG("speed attached");
    }

    void start() {
        auto isequals = [](std::string_view a, std::string_view b) {
            if (a.size() != b.size()) return false;

            for (size_t i = 0; i < a.size(); ++i) {
                if (std::tolower(static_cast<unsigned char>(a[i])) !=
                    std::tolower(static_cast<unsigned char>(b[i]))) {
                    return false;
                }
            }
            return true;
        };
        if (isequals(model_, "separate")) {
            SPDLOG_DEBUG("starting separate loop");
            separate_loop_();
        } else {
            SPDLOG_DEBUG("starting full loop");
            full_loop_();
        }
        return;
    }
};
