#include <spdlog/spdlog.h>

#include <string_view>

#include "devices.hpp"

class LightingEffect {
  private:
    enum class Type_ {
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

    Type_ str_to_type_(std::string_view effect_name) {
        if (effect_name == "Alternating")
            return Type_::Alternating;
        else if (effect_name == "Temperature")
            return Type_::Temperature;
        else if (effect_name == "Full")
            return Type_::Full;
        else if (effect_name == "PerLED")
            return Type_::PerLED;
        else if (effect_name == "Flow")
            return Type_::Flow;
        else if (effect_name == "Spectrum")
            return Type_::Spectrum;
        else if (effect_name == "Pulse")
            return Type_::Pulse;
        else if (effect_name == "Wave")
            return Type_::Wave;
        else if (effect_name == "Off")
            return Type_::Off;
        spdlog::error("Incorrect effect name, switching to default (\"Off\")");
        return Type_::Off;
    }

    const Type_ type_;

    ThermaltakeDevice* device_;

  public:
    LightingEffect(std::string_view effect_name, ThermaltakeDevice* dev)
        : type_(str_to_type_(effect_name)), device_(dev) {}

    void bind_device(ThermaltakeDevice* dev) {
        device_ = dev;
        return;
    }

    void start() const;

    void stop() const;
};
