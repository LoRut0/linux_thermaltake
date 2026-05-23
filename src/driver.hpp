#pragma once

#include <libusb-1.0/libusb.h>

#include <system_error>
#include <thread>
#include <vector>

#include "globals.hpp"

class ThermaltakeControllerDriver {
  private:
    const u_int16_t kVENDOR_ID = 0x264a;
    const u_int16_t kPRODUCT_ID = 0x2260;
    const u_int8_t kENDPOINT_OUT = 0x01;
    const u_int8_t kENDPOINT_IN = 0x81;

    libusb_device* device_;
    libusb_device_handle* handle_;

    void find_device_() {
        handle_ =
            libusb_open_device_with_vid_pid(nullptr, kVENDOR_ID, kPRODUCT_ID);
        if (!handle_) {
            SPDLOG_ERROR("Failed to find device");
            throw std::runtime_error("Device not found");
        }
        SPDLOG_INFO("Device was found");
        return;
    }
    void claim_interface_() {
        int res = libusb_claim_interface(handle_, 0);
        if (res) {
            SPDLOG_ERROR("Failed to claim interface");
            throw std::runtime_error(libusb_error_name(res));
        }
        SPDLOG_INFO("Interface claimed");
        return;
    }
    void set_configuration_() {
        int res = libusb_set_configuration(handle_, 1);
        if (res) {
            SPDLOG_ERROR("Failed to set configuration");
            throw std::runtime_error(libusb_error_name(res));
        }
        SPDLOG_INFO("Configuration set");
        return;
    }
    void reset_device_() {
        int res = libusb_reset_device(handle_);
        if (res) {
            SPDLOG_ERROR("Failed to reset device");
            throw std::runtime_error(libusb_error_name(res));
        }
        SPDLOG_INFO("Device reseted");
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        return;
    }
    void detach_kernel_driver_() {
        int res = libusb_detach_kernel_driver(handle_, 0);
        if (res) {
            SPDLOG_WARN("Failed to detach kernel driver with Libusb Error: {}",
                        libusb_error_name(res));
            return;
        }
        SPDLOG_INFO("Driver detached");
        return;
    }

  public:
    ThermaltakeControllerDriver() {
        int res = libusb_init(nullptr);
        if (res) {
            SPDLOG_ERROR("Failed to init libusb");
            throw std::runtime_error(libusb_error_name(res));
        }
        SPDLOG_INFO("Libusb driver inited");

        find_device_();
        reset_device_();
        detach_kernel_driver_();
        set_configuration_();
        claim_interface_();
    }

    void init_controller() {
        std::vector<u_int8_t> data(PKT_SIZE, 0x00);
        data[0] = 0xFE;
        data[1] = 0x33;
        write_out(data);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        read_in(64);
        SPDLOG_INFO("Controller successfully inited");
        return;
    }

    void write_out(std::vector<u_int8_t>& data) {
        int res = libusb_interrupt_transfer(handle_, kENDPOINT_OUT, data.data(),
                                            data.size(), nullptr, 1000);
        if (res) {
            SPDLOG_ERROR("Failed to write_out Libusb_Error: {}",
                         libusb_error_name(res));
        }
        return;
    }

    void write_out(u_int8_t* data) {
        int res = libusb_interrupt_transfer(handle_, kENDPOINT_OUT, data,
                                            PKT_SIZE, nullptr, 1000);
        if (res) {
            SPDLOG_ERROR("Failed to write_out Libusb_Error: {}",
                         libusb_error_name(res));
        }
        return;
    }

    // std::vector<u_int8_t> read_out(std::vector<u_int8_t>& data) {
    //     throw std::errc::operation_not_permitted;
    // }
    //
    // void write_in(std::vector<u_int8_t>& data) {
    //     throw std::errc::operation_not_permitted;
    // }

    std::vector<u_int8_t> read_in(int len = 64) {
        std::vector<u_int8_t> data(len, 0);
        int res = libusb_interrupt_transfer(handle_, kENDPOINT_IN, data.data(),
                                            data.size(), nullptr, 1000);
        if (res) {
            SPDLOG_ERROR("Failed to read in Libusb_Error: {}",
                         libusb_error_name(res));
        }
        return data;
    }

    void save_profile() {
        u_int8_t data[] = {0x32, 0x53};
        int res = libusb_interrupt_transfer(handle_, kENDPOINT_OUT, data, 2,
                                            nullptr, 1000);
        if (res) {
            SPDLOG_ERROR("Failed to save profile");
            throw std::runtime_error(libusb_error_name(res));
        }
        return;
    }
};
