#pragma once

#include <yaml-cpp/yaml.h>

struct LightingConfig {
    std::string model;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct FanConfig {
    std::string model;
    uint8_t speed = 50;
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

    static FanConfig loadFanConfig(const std::string& path) {
        YAML::Node root = YAML::LoadFile(path);

        YAML::Node fm = root["fan_manager"];
        if (!fm) {
            throw std::runtime_error("No 'fan_manager' section in YAML-config");
        }

        FanConfig cfg;

        if (!fm["model"]) {
            throw std::runtime_error("No field 'model' in 'fan_manager'");
        }

        if (fm["speed"]) {
            int value = fm["speed"].as<int>();
            cfg.speed = static_cast<uint8_t>(value);
        }

        return cfg;
    }
};
