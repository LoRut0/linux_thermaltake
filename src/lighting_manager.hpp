#include <spdlog/spdlog.h>

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
    const uint8_t& red;
    const uint8_t& green;
    const uint8_t& blue;

    std::vector<uint8_t> data;

  public:
    Full(ThermaltakeDevice* dev, const uint8_t& red, const uint8_t& green,
         const uint8_t& blue)
        : LightingEffect(dev), red(red), green(green), blue(blue) {
        data.resize(PKT_SIZE, 0x00);
        data[0] = 0x00;
        data[1] = 0x32;
        data[2] = 0x52;
        // channel data[3]
        data[4] = 0x24;
        for (int i = 5; i < PKT_SIZE; i += 3) {
            data[i] = green;
            data[i + 1] = red;
            data[i + 2] = blue;
        }
    }

    uint8_t* next_frame(const uint8_t& port) {
        data[3] = port;
        return data.data();
    }
};
