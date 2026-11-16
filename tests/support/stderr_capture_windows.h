#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <array>
#include <cerrno>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <system_error>

// Capture both CRT stderr (progress) and the Win32 standard handle (spdlog).
class StderrCapture {
public:
    StderrCapture() {
        saved_handle_ = GetStdHandle(STD_ERROR_HANDLE);
        try {
            if (_pipe(pipe_, 65536, _O_BINARY) != 0)
                throw std::system_error(errno, std::generic_category(), "create capture pipe");
            saved_ = _dup(_fileno(stderr));
            if (saved_ < 0 || _dup2(pipe_[1], _fileno(stderr)) < 0)
                throw std::system_error(errno, std::generic_category(), "redirect CRT stderr");
            if (!SetStdHandle(STD_ERROR_HANDLE,
                              reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(stderr)))))
                throw std::system_error(GetLastError(), std::system_category(), "redirect Win32 stderr");
            _close(pipe_[1]);
            pipe_[1] = -1;
        } catch (...) {
            cleanup();
            throw;
        }
    }

    ~StderrCapture() { cleanup(); }

    std::string finish() {
        std::fflush(stderr);
        if (!restore()) throw std::runtime_error("restore captured stderr");
        std::string output;
        std::array<char, 4096> buffer{};
        for (;;) {
            const auto count = _read(pipe_[0], buffer.data(), static_cast<unsigned>(buffer.size()));
            if (count == 0) break;
            if (count < 0) throw std::system_error(errno, std::generic_category(), "read stderr");
            output.append(buffer.data(), static_cast<std::size_t>(count));
        }
        _close(pipe_[0]);
        pipe_[0] = -1;
        return output;
    }

private:
    bool restore() noexcept {
        if (saved_ < 0) return true;
        const bool crt_ok = _dup2(saved_, _fileno(stderr)) == 0;
        const bool win_ok = SetStdHandle(STD_ERROR_HANDLE, saved_handle_) != 0;
        _close(saved_);
        saved_ = -1;
        return crt_ok && win_ok;
    }

    void cleanup() noexcept {
        restore();
        for (auto& fd : pipe_) {
            if (fd >= 0) _close(fd);
            fd = -1;
        }
    }

    HANDLE saved_handle_ = INVALID_HANDLE_VALUE;
    int pipe_[2]{-1, -1};
    int saved_ = -1;
};
