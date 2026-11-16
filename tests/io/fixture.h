#pragma once

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

namespace ninfer::test::io {

inline std::filesystem::path make_directory() {
#ifdef _WIN32
    const auto root = std::filesystem::temp_directory_path();
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    for (unsigned i = 0; i < 100; ++i) {
        const auto path = root / ("ninfer-io-" + std::to_string(stamp) + "-" + std::to_string(i));
        if (std::filesystem::create_directory(path)) return path;
    }
    throw std::runtime_error("cannot create I/O fixture directory");
#else
    std::string name = (std::filesystem::temp_directory_path() / "ninfer-io-XXXXXX").string();
    auto* path       = ::mkdtemp(name.data());
    if (!path) throw std::runtime_error("mkdtemp failed");
    return path;
#endif
}

struct Directory {
    std::filesystem::path path = make_directory();

    ~Directory() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};

} // namespace ninfer::test::io
