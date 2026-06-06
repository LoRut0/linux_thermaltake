cmake_minimum_required(VERSION 3.20)

project(
    thermaltake_fan_controller
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


include(FetchContent)

# ---------------- spdlog ----------------

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(spdlog)

# ---------------- yaml-cpp ----------------

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        yaml-cpp-0.9.0
)
FetchContent_MakeAvailable(yaml-cpp)

# ---------------- libusb ----------------

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)

# ---------------- NVML (libnvidia-ml) ----------------
# Header is vendored in include/third_vendors/nvml.h.
# The shared library ships with the proprietary NVIDIA driver (nvidia-utils).

find_library(NVML_LIB nvidia-ml)
if(NOT NVML_LIB)
    message(FATAL_ERROR "libnvidia-ml not found (install nvidia-utils)")
endif()

# ---------------- executable ----------------

add_executable(thermaltake-controller
    src/main.cpp
    src/sensors.cpp
)

target_include_directories(thermaltake-controller
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(thermaltake-controller
    PRIVATE
        spdlog::spdlog
        yaml-cpp::yaml-cpp
        PkgConfig::LIBUSB
        ${NVML_LIB}
)

target_compile_options(thermaltake-controller
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_compile_definitions(thermaltake-controller
    PRIVATE
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
)


# ---------------- installation ----------------
include(GNUInstallDirs)

# Main binary -> /usr/bin
install(TARGETS thermaltake-controller
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Default config -> /etc/thermaltake-controller/
# Installed only if it does NOT already exist,
# so user edits are never overwritten.
install(CODE "
    set(cfg_dst \"\$ENV{DESTDIR}${CMAKE_INSTALL_FULL_SYSCONFDIR}/thermaltake-controller/config.yml\")
    if(EXISTS \"\${cfg_dst}\")
        message(STATUS \"Config already exists, keeping it: \${cfg_dst}\")
    else()
        file(INSTALL \"${CMAKE_CURRENT_SOURCE_DIR}/config.yml\"
             DESTINATION \"\$ENV{DESTDIR}${CMAKE_INSTALL_FULL_SYSCONFDIR}/thermaltake-controller\")
    endif()
")

# systemd unit -> /usr/lib/systemd/system
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/packaging/thermaltake-controller.service
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system
)

# udev rule -> /usr/lib/udev/rules.d
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/packaging/99-thermaltake-controller.rules
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/udev/rules.d
)
