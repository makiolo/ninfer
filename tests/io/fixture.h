#pragma once

#include <chrono>
#include <filesystem>
#include <stdexcept>

#ifndef NINFER_IO_TEST_ROOT
#    define NINFER_IO_TEST_ROOT ".agent-tmp/R97-05"
#endif

namespace ninfer::test::io {

inline std::filesystem::path make_directory() {
    const std::filesystem::path root(NINFER_IO_TEST_ROOT);
    std::filesystem::create_directories(root);
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    for (unsigned i = 0; i < 100; ++i) {
        const auto path = root / ("fixture-" + std::to_string(stamp) + "-" + std::to_string(i));
        if (std::filesystem::create_directory(path)) return path;
    }
    throw std::runtime_error("cannot create I/O fixture directory");
}

struct Directory {
    std::filesystem::path path = make_directory();

    ~Directory() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};

} // namespace ninfer::test::io
