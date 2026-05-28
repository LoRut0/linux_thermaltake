#pragma once

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <limits>

struct LightingConfig {
    std::string model;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct SpeedConfig {
    std::string model;
    uint8_t speed = 50;
    std::vector<std::pair<int, int>> curve;
    bool source_cpu;
    bool source_gpu;
};

class Config {
  private:
    static uint8_t readColorChannel_(const YAML::Node& node,
                                     const std::string& name) {
        if (!node[name]) {
            return 0;  // канал не задан — считаем 0
        }
        int value = node[name].as<int>();
        if (value < 0 || value > 255) {
            throw std::runtime_error("value '" + name +
                                     "' вне диапазона 0..255");
        }
        return static_cast<uint8_t>(value);
    }

  public:
    static LightingConfig loadLightingConfig(const std::string& path) {
        YAML::Node root = YAML::LoadFile(path);

        YAML::Node lm = root["lighting_manager"];
        if (!lm) {
            throw std::runtime_error(
                "No 'lighting_manager' section in YAML-config");
        }

        LightingConfig cfg;

        if (!lm["model"]) {
            throw std::runtime_error("No field 'model' in 'lighting_manager'");
        }
        cfg.model = lm["model"].as<std::string>();

        cfg.r = readColorChannel_(lm, "r");
        cfg.g = readColorChannel_(lm, "g");
        cfg.b = readColorChannel_(lm, "b");

        return cfg;
    }

    static SpeedConfig loadSpeedConfig(const std::string& path) {
        constexpr int kMaxPwm = 100;  // curve PWM is expressed in percent
        YAML::Node root = YAML::LoadFile(path);
        YAML::Node fm = root["fan_manager"];
        if (!fm) {
            spdlog::error("No 'fan_manager' section in YAML-config");
            throw std::runtime_error("No 'fan_manager' section in YAML-config");
        }

        SpeedConfig cfg;

        if (!fm["model"]) {
            spdlog::error("No field 'model' in 'fan_manager'");
            throw std::runtime_error("No field 'model' in 'fan_manager'");
        }
        cfg.model = fm["model"].as<std::string>();

        if (fm["speed"]) {
            int pwm = fm["speed"].as<int>();
            if (pwm < 0 or pwm > kMaxPwm) {
                spdlog::error("Curve pwm {} out of range [0, {}]", pwm,
                              kMaxPwm);
                throw std::runtime_error("Curve pwm out of range");
            }
            cfg.speed = static_cast<uint8_t>(pwm);
        }

        // sources and curve are only meaningful for the Temperature model
        if (cfg.model == "Temperature") {
            // ---- sources ----
            if (!fm["sources"]) {
                spdlog::error("Model 'Temperature' requires a 'sources' field");
                throw std::runtime_error(
                    "Missing 'sources' for Temperature model");
            }
            if (!fm["sources"].IsSequence()) {
                spdlog::error(
                    "'sources' must be a list, e.g. [\"Cpu\", \"Gpu\"]");
                throw std::runtime_error("'sources' is not a sequence");
            }

            for (const auto& node : fm["sources"]) {
                std::string s = node.as<std::string>();
                if (s == "Cpu") {
                    cfg.source_cpu = true;
                } else if (s == "Gpu") {
                    cfg.source_gpu = true;
                } else {
                    spdlog::error(
                        "Unknown source '{}' (expected 'Cpu' or 'Gpu')", s);
                    throw std::runtime_error(
                        "Invalid source value in 'sources'");
                }
            }

            if (!cfg.source_cpu && !cfg.source_gpu) {
                spdlog::error(
                    "'sources' must contain at least one of 'Cpu' or 'Gpu'");
                throw std::runtime_error("'sources' is empty");
            }

            // ---- curve ----
            if (!fm["curve"]) {
                spdlog::error("Model 'Temperature' requires a 'curve' field");
                throw std::runtime_error(
                    "Missing 'curve' for Temperature model");
            }
            if (!fm["curve"].IsSequence()) {
                spdlog::error("'curve' must be a list of [temp, pwm] points");
                throw std::runtime_error("'curve' is not a sequence");
            }
            if (fm["curve"].size() < 2) {
                spdlog::error("'curve' must have at least 2 points, got {}",
                              fm["curve"].size());
                throw std::runtime_error("'curve' has too few points");
            }

            int prev_temp = std::numeric_limits<int>::min();
            for (const auto& point : fm["curve"]) {
                // Each point must be a pair [temperature, pwm]
                if (!point.IsSequence() || point.size() != 2) {
                    spdlog::error(
                        "Each curve point must be a [temp, pwm] pair");
                    throw std::runtime_error("Malformed curve point");
                }

                int temp = point[0].as<int>();
                int pwm = point[1].as<int>();

                // Temperature must be strictly increasing for interpolation to
                // work
                if (temp <= prev_temp) {
                    spdlog::error(
                        "Curve temperatures must strictly increase "
                        "(got {} after {})",
                        temp, prev_temp);
                    throw std::runtime_error("Curve temperatures not sorted");
                }
                prev_temp = temp;

                // PWM must be within the valid range
                if (pwm < 0 || pwm > kMaxPwm) {
                    spdlog::error("Curve pwm {} out of range [0, {}]", pwm,
                                  kMaxPwm);
                    throw std::runtime_error("Curve pwm out of range");
                }

                cfg.curve.emplace_back(temp, pwm);
            }
        }

        return cfg;
    }

    static std::vector<std::string> loadDevices(const std::string& path) {
        YAML::Node root = YAML::LoadFile(path);

        YAML::Node devicesNode = root["devices"];
        if (!devicesNode) {
            throw std::runtime_error("Missing 'devices' section in YAML file");
        }
        if (!devicesNode.IsMap()) {
            throw std::runtime_error("'devices' section must be a map");
        }

        // First collect (key, value) pairs, then sort by key so that the order
        // does not depend on how the library iterated over the map.
        std::vector<std::pair<int, std::string>> pairs;
        pairs.reserve(devicesNode.size());
        for (const auto& entry : devicesNode) {
            int key = entry.first.as<int>();
            std::string value = entry.second.as<std::string>();
            pairs.emplace_back(key, std::move(value));
        }

        std::sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

        std::vector<std::string> devices;
        devices.reserve(pairs.size());
        for (auto& p : pairs) {
            devices.push_back(std::move(p.second));
        }
        return devices;
    }
};
