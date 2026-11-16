#pragma once

// Implementation details shared only by the Windows file and queue owners.
#ifndef NOMINMAX
#    define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <io.h>

#include <cerrno>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace ninfer::io::windows {

class Handle {
public:
    explicit Handle(HANDLE value = INVALID_HANDLE_VALUE) noexcept : value_(value) {}

    ~Handle() {
        if (valid()) ::CloseHandle(value_);
    }

    Handle(const Handle&)            = delete;
    Handle& operator=(const Handle&) = delete;

    Handle(Handle&& other) noexcept : value_(other.release()) {}

    Handle& operator=(Handle&& other) noexcept {
        Handle previous(release());
        value_ = other.release();
        return *this;
    }

    [[nodiscard]] bool valid() const noexcept {
        return value_ != nullptr && value_ != INVALID_HANDLE_VALUE;
    }

    [[nodiscard]] HANDLE get() const noexcept { return value_; }

    HANDLE release() noexcept { return std::exchange(value_, INVALID_HANDLE_VALUE); }

private:
    HANDLE value_;
};

inline int error_number(DWORD error) noexcept {
    switch (error) {
    case ERROR_SUCCESS:
        return 0;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_INVALID_DRIVE:
        return ENOENT;
    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
        return EACCES;
    case ERROR_INVALID_HANDLE:
        return EBADF;
    case ERROR_INVALID_PARAMETER:
    case ERROR_NEGATIVE_SEEK:
        return EINVAL;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
    case ERROR_NO_SYSTEM_RESOURCES:
    case ERROR_NOT_ENOUGH_QUOTA:
        return ENOMEM;
    case ERROR_TOO_MANY_OPEN_FILES:
        return EMFILE;
    case ERROR_OPERATION_ABORTED:
        return ECANCELED;
    case ERROR_NOT_SUPPORTED:
    case ERROR_INVALID_FUNCTION:
        return EOPNOTSUPP;
    case ERROR_INVALID_USER_BUFFER:
        return EFAULT;
    case ERROR_DISK_FULL:
        return ENOSPC;
    case ERROR_HANDLE_EOF:
        return ENODATA;
    default:
        return EIO;
    }
}

[[noreturn]] inline void fail(std::string_view context, const char* operation,
                              DWORD error = ::GetLastError()) {
    throw std::system_error(
        error_number(error), std::generic_category(),
        std::string(context) + (operation ? std::string(": ") + operation : "") + " (Windows " +
            std::to_string(error) + ": " + std::system_category().message(error) + ")");
}

[[noreturn]] inline void fail(std::string_view operation, DWORD error = ::GetLastError()) {
    fail(operation, nullptr, error);
}

inline HANDLE native_handle(int descriptor) {
    return reinterpret_cast<HANDLE>(::_get_osfhandle(descriptor));
}

inline void validate_offset(std::uint64_t value) {
    if (value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
        throw std::invalid_argument("file offset exceeds signed 64-bit range");
}

inline OVERLAPPED overlapped(std::uint64_t offset) {
    validate_offset(offset);
    OVERLAPPED result{};
    result.Offset     = static_cast<DWORD>(offset);
    result.OffsetHigh = static_cast<DWORD>(offset >> 32);
    return result;
}

} // namespace ninfer::io::windows
