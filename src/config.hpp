#pragma once

#include <yaml-cpp/yaml.h>

struct LightingConfig {
    std::string model;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct SpeedConfig {
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

    static SpeedConfig loadSpeedConfig(const std::string& path) {
        YAML::Node root = YAML::LoadFile(path);

        YAML::Node fm = root["fan_manager"];
        if (!fm) {
            throw std::runtime_error("No 'fan_manager' section in YAML-config");
        }

        SpeedConfig cfg;

        if (!fm["model"]) {
            throw std::runtime_error("No field 'model' in 'fan_manager'");
        }

        if (fm["speed"]) {
            int value = fm["speed"].as<int>();
            cfg.speed = static_cast<uint8_t>(value);
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
