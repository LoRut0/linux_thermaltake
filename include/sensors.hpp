// sensors.hpp
// Sensors class: detects CPU/GPU temperature sources once in constructor,
// then reads them efficiently on demand.
//
// Build: g++ -std=c++17 sensors.cpp -o sensors -lnvidia-ml
// (NVML lib comes with the proprietary NVIDIA driver)

#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "third_vendors/nvml.h"

namespace fs = std::filesystem;

class Sensors {
  public:
    Sensors();
    ~Sensors();

    // Non-copyable: owns NVML state
    Sensors(const Sensors&) = delete;
    Sensors& operator=(const Sensors&) = delete;

    // Return temperature in degrees Celsius, or -1 on failure / no sensor.
    int get_cpu_temperature() const;
    int get_gpu_temperature() const;

    // Diagnostics
    const std::string& cpu_source() const { return cpu_source_desc_; }
    const std::string& gpu_source() const { return gpu_source_desc_; }

  private:
    enum class GpuBackend { None, Hwmon, Nvml };

    // Helpers used only during construction
    static std::string read_hwmon_name(const fs::path& dir);
    static fs::path find_hwmon_by_names(const std::vector<std::string>& names);

    // CPU: cached full path to tempN_input
    std::string cpu_temp_path_;
    std::string cpu_source_desc_ = "none";

    // GPU: either a sysfs path OR an NVML device handle
    GpuBackend gpu_backend_ = GpuBackend::None;
    std::string gpu_temp_path_;           // used when backend == Hwmon
    nvmlDevice_t nvml_device_ = nullptr;  // used when backend == Nvml
    bool nvml_initialized_ = false;
    std::string gpu_source_desc_ = "none";
};
