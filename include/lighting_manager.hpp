#pragma once

#include <spdlog/spdlog.h>

#include <vector>

#include "devices.hpp"

class LightingEffect {
  protected:
    ThermaltakeDevice* device_;

  public:
    LightingEffect(ThermaltakeDevice* dev = nullptr) : device_(dev) {}

    void bind_device(ThermaltakeDevice* dev) {
        device_ = dev;
        return;
    }

    virtual uint8_t* next_frame(const uint8_t& port) = 0;
};

class Full : public LightingEffect {
  private:
    std::vector<uint8_t> data;

  public:
    Full(const uint8_t& red, const uint8_t& green, const uint8_t& blue)
        : LightingEffect(nullptr) {
        data.resize(PKT_SIZE, 0x00);
        data[0] = 0x32;
        data[1] = 0x52;
        // channel data[2]
        data[3] = 0x24;
        for (int i = 4; i + 2 < PKT_SIZE; i += 3) {
            data[i] = green;
            data[i + 1] = red;
            data[i + 2] = blue;
        }
    }

    uint8_t* next_frame(const uint8_t& port) {
        data[2] = port + 1;
        return data.data();
    }
};
