#pragma once

#include <string_view>

struct DeviceSpec {
    std::string_view model;
    int num_leds;
    int index_per_led;
    bool has_rgb;
    bool has_fan;
};
inline constexpr DeviceSpec kDeviceSpecs[] = {
    {
        .model = "Riing Plus",
        .num_leds = 12,
        .index_per_led = 3,
        .has_rgb = true,
        .has_fan = true,
    },
    {
        .model = "Floe Riing RGB",
        .num_leds = 12,
        .index_per_led = 3,
        .has_rgb = true,
        .has_fan = false,
    },
    {
        .model = "Pacific PR22-D5 Plus",
        .num_leds = 12,
        .index_per_led = 3,
        .has_rgb = true,
        .has_fan = false,
    },
    {
        .model = "Pacific W4 Plus CPU Waterblock",
        .num_leds = 12,
        .index_per_led = 3,
        .has_rgb = true,
        .has_fan = false,
    },
    {
        .model = "Lumi Plus LED Strip",
        .num_leds = 12,
        .index_per_led = 3,
        .has_rgb = true,
        .has_fan = false,
    },
    {
        .model = "Pacific PR22-D5 Plus",
        .num_leds = 12,
        .index_per_led = 3,
        .has_rgb = true,
        .has_fan = false,
    },
    {
        .model = "Pacific W4 Plus CPU Waterblock",
        .num_leds = 12,
        .index_per_led = 3,
        .has_rgb = true,
        .has_fan = false,
    },
    {
        .model = "Pacific V-GTX 1080Ti Plus GPU Waterblock",
        .num_leds = 12,
        .index_per_led = 3,
        .has_rgb = true,
        .has_fan = false,
    },
    {
        .model = "Pacific Rad Plus LED Panel",
        .num_leds = 12,
        .index_per_led = 3,
        .has_rgb = true,
        .has_fan = false,
    },
    {
        .model = "Lumi Plus LED Strip",
        .num_leds = 12,
        .index_per_led = 3,
        .has_rgb = true,
        .has_fan = false,
    },
};
