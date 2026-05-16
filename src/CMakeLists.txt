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
    GIT_TAG        v1.15.3
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
    GIT_TAG        0.8.0
)

FetchContent_MakeAvailable(yaml-cpp)

# ---------------- libusb ----------------

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)

# ---------------- executable ----------------

add_executable(thermaltake-controller
    main.cpp
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
