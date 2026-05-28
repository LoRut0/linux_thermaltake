#include "sensors.hpp"

std::string Sensors::read_hwmon_name(const fs::path& dir) {
    std::ifstream f(dir / "name");
    std::string name;
    if (f >> name) return name;
    return {};
}

fs::path Sensors::find_hwmon_by_names(const std::vector<std::string>& names) {
    const fs::path root = "/sys/class/hwmon";
    if (!fs::exists(root)) return {};
    for (const auto& entry : fs::directory_iterator(root)) {
        std::string n = read_hwmon_name(entry.path());
        for (const auto& candidate : names) {
            if (n == candidate) return entry.path();
        }
    }
    return {};
}

// ---------- constructor: detect sources once ----------

Sensors::Sensors() {
    // ---- CPU: look for known CPU temperature drivers ----
    fs::path cpu_hwmon =
        find_hwmon_by_names({"coretemp", "k10temp", "zenpower"});
    if (!cpu_hwmon.empty()) {
        cpu_temp_path_ = (cpu_hwmon / "temp1_input").string();
        cpu_source_desc_ = "hwmon:" + read_hwmon_name(cpu_hwmon);
    }

    // ---- GPU: try open-source hwmon drivers first ----
    fs::path gpu_hwmon = find_hwmon_by_names({"amdgpu", "nouveau", "i915"});
    if (!gpu_hwmon.empty()) {
        gpu_backend_ = GpuBackend::Hwmon;
        gpu_temp_path_ = (gpu_hwmon / "temp1_input").string();
        gpu_source_desc_ = "hwmon:" + read_hwmon_name(gpu_hwmon);
        return;
    }

    // ---- GPU: fall back to NVML (proprietary NVIDIA driver) ----
    nvmlReturn_t r = nvmlInit_v2();
    if (r != NVML_SUCCESS) {
        std::cerr << "nvmlInit failed: " << nvmlErrorString(r) << "\n";
        return;
    }
    nvml_initialized_ = true;

    unsigned int count = 0;
    r = nvmlDeviceGetCount_v2(&count);
    if (r != NVML_SUCCESS || count == 0) {
        std::cerr << "no NVML devices found\n";
        return;
    }

    // Use first GPU; extend here if multi-GPU support is needed
    r = nvmlDeviceGetHandleByIndex_v2(0, &nvml_device_);
    if (r != NVML_SUCCESS) {
        std::cerr << "nvmlDeviceGetHandleByIndex failed: " << nvmlErrorString(r)
                  << "\n";
        nvml_device_ = nullptr;
        return;
    }

    char name[NVML_DEVICE_NAME_BUFFER_SIZE] = {0};
    nvmlDeviceGetName(nvml_device_, name, sizeof(name));

    gpu_backend_ = GpuBackend::Nvml;
    gpu_source_desc_ = std::string("nvml:") + name;
}

Sensors::~Sensors() {
    if (nvml_initialized_) {
        nvmlShutdown();
    }
}

// ---------- runtime reads: cheap, no path resolution ----------

int Sensors::get_cpu_temperature() const {
    if (cpu_temp_path_.empty()) return -1;
    std::ifstream f(cpu_temp_path_);
    int millideg = 0;
    if (!(f >> millideg)) return -1;
    return millideg / 1000;
}

int Sensors::get_gpu_temperature() const {
    switch (gpu_backend_) {
        case GpuBackend::Hwmon: {
            std::ifstream f(gpu_temp_path_);
            int millideg = 0;
            if (!(f >> millideg)) return -1;
            return millideg / 1000;
        }
        case GpuBackend::Nvml: {
            unsigned int temp = 0;
            nvmlReturn_t r = nvmlDeviceGetTemperature(
                nvml_device_, NVML_TEMPERATURE_GPU, &temp);
            if (r != NVML_SUCCESS) return -1;
            return static_cast<int>(temp);
        }
        case GpuBackend::None:
        default:
            return -1;
    }
}
