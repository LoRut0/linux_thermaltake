#pragma once

#include <spdlog/spdlog.h>

#include <cstdint>
#include <string_view>

#define PKT_SIZE 192

#define KEEPALIVE_INTERVAL 2000

const uint8_t kPROTOCOL_GET = 0x33;
const uint8_t kPROTOCOL_SET = 0x32;

const uint8_t kPROTOCOL_FAN = 0x51;
const uint8_t kPROTOCOL_LIGHT = 0x52;

// credit: https://github.com/devcompl/riingplusapi
class RGB {
    class Mode {
        const uint8_t kLOW = 0x00;
        const uint8_t kPECTRUM = 0x04;
        const uint8_t kIPPLE = 0x08;
        const uint8_t kLINK = 0x0c;
        const uint8_t kULSE = 0x10;
        const uint8_t kAVE = 0x14;
        const uint8_t kY_LED = 0x18;
        const uint8_t kULL = 0x19;
    };

    class Speed {
        const uint8_t kLOW = 0x03;
        const uint8_t kORMAL = 0x02;
        const uint8_t kAST = 0x01;
        const uint8_t kXTREME = 0x00;
    };
};

enum class Model { Full, Separate };

enum class EffectType {
    Alternating,
    Temperature,
    Full,
    PerLED,
    Flow,
    Spectrum,
    Pulse,
    Wave,
    Off
};

enum class SpeedType { Locked, Temperature };

EffectType str_to_effect_type(std::string_view effect_name) {
    if (effect_name == "Alternating")
        return EffectType::Alternating;
    else if (effect_name == "Temperature")
        return EffectType::Temperature;
    else if (effect_name == "Full")
        return EffectType::Full;
    else if (effect_name == "PerLED")
        return EffectType::PerLED;
    else if (effect_name == "Flow")
        return EffectType::Flow;
    else if (effect_name == "Spectrum")
        return EffectType::Spectrum;
    else if (effect_name == "Pulse")
        return EffectType::Pulse;
    else if (effect_name == "Wave")
        return EffectType::Wave;
    else if (effect_name == "Off")
        return EffectType::Off;
    spdlog::error("Incorrect effect name, switching to default (\"Off\")");
    return EffectType::Off;
}

SpeedType str_to_speed_type(std::string_view speed_name) {
    if (speed_name == "Locked")
        return SpeedType::Locked;
    else if (speed_name == "Temperature")
        return SpeedType::Temperature;
    spdlog::error(
        "Incorrect speed model name, switching to default (\"Locked\")");
    return SpeedType::Locked;
}
