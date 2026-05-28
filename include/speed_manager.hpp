#pragma once

#include "algorithm"
#include "globals.hpp"
#include "sensors.hpp"

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
    }
};

class Temperature : public SpeedManager {
  private:
    enum class Source { Cpu, Gpu, Both };
    std::vector<uint8_t> data;
    const std::vector<std::pair<int, int>> curve;
    const Source source_;
    Sensors sensors_;

    // Map a temperature to a PWM value using linear interpolation between
    // the curve's control points. The curve must be sorted by temperature.
    uint8_t interpolate_(int temp) {
        if (curve.empty()) return 0;

        // Below the first point: clamp to the first pwm value
        if (temp <= curve.front().first) return curve.front().second;

        // Above the last point: clamp to the last pwm value
        if (temp >= curve.back().first) return curve.back().second;

        // Find the segment [p0, p1] that contains temp, then interpolate
        for (size_t i = 1; i < curve.size(); ++i) {
            const auto& [t0, pwm0] = curve[i - 1];
            const auto& [t1, pwm1] = curve[i];

            if (temp <= t1) {
                // Linear interpolation: how far temp sits between t0 and t1
                double ratio = static_cast<double>(temp - t0) / (t1 - t0);
                return pwm0 + static_cast<int>(ratio * (pwm1 - pwm0));
            }
        }

        return curve.back()
            .second;  // unreachable, but keeps the compiler happy
    }

    uint8_t compute_speed_() {
        int temp;
        switch (source_) {
            case Source::Cpu: {
                temp = sensors_.get_cpu_temperature();
                break;
            }
            case Source::Gpu: {
                temp = sensors_.get_gpu_temperature();
                break;
            }
            case Source::Both: {
                temp = std::max(sensors_.get_cpu_temperature(),
                                sensors_.get_gpu_temperature());
                break;
            }
        }
        return interpolate_(temp);
    }

    // updates fans speed in data
    void update_speed_() {
        data[4] = compute_speed_();
        return;
    }

  public:
    /*
     * @param (vector) receives pairs <temperature, speed>
     * @param (cpu) if fan speed should depend on cpu temperature
     * @param (gpu) if fan speed should depend on gpu temperature
     */
    Temperature(const std::vector<std::pair<int, int>>& curve, bool cpu,
                bool gpu)
        : curve(curve),
          source_(cpu && gpu ? Source::Both
                  : gpu      ? Source::Gpu
                             : Source::Cpu) {
        data.resize(PKT_SIZE, 0x00);
        data[0] = 0x32;
        data[1] = 0x51;
        // channel data[2]
        data[3] = 0x01;
        data[4] = curve.back().second;
    }

    uint8_t* get_speed(const uint8_t& port) {
        if (port == 0) update_speed_();
        data[2] = port + 1;
        return data.data();
    }
};
