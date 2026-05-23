#pragma once

#include "globals.hpp"

class SpeedManager {
  public:
    ~SpeedManager() = default;
    virtual uint8_t* get_speed(const uint8_t& port) = 0;
};

class Locked : public SpeedManager {
  private:
    std::vector<uint8_t> data;

  public:
    Locked(const uint8_t& speed) {
        data.resize(PKT_SIZE, 0x00);
        data[0] = 0x32;
        data[1] = 0x51;
        // channel data[2]
        data[3] = 0x01;
        data[4] = speed;
    }

    uint8_t* get_speed(const uint8_t& port) {
        data[2] = port + 1;
        return data.data();
    };
};
