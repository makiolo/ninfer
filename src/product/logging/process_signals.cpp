#include "product/logging/process_signals.h"

#include <atomic>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
#    include <windows.h>
#else
#    include <cerrno>
#    include <csignal>
#endif

namespace ninfer::product {
namespace {

// Static storage keeps callbacks independent of the registration object's lifetime.
static_assert(std::atomic<bool>::is_always_lock_free);
std::atomic<bool> stop_requested{false};
std::atomic_flag owned = ATOMIC_FLAG_INIT;

#ifdef _WIN32
static_assert(std::atomic<unsigned>::is_always_lock_free);
std::atomic<unsigned> completed_shutdown{0};

BOOL WINAPI handle_console(DWORD event) {
    switch (event) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        stop_requested.store(true, std::memory_order_relaxed);
        return TRUE;
    case CTRL_CLOSE_EVENT: {
        const auto generation = completed_shutdown.load(std::memory_order_acquire);
        stop_requested.store(true, std::memory_order_relaxed);
        // Returning from a close handler lets Windows terminate the process. Give the normal
        // shutdown path time to release its resources, within the console's five-second limit.
        const auto deadline = GetTickCount64() + 4500;
        while (completed_shutdown.load(std::memory_order_acquire) == generation &&
               GetTickCount64() < deadline)
            Sleep(10);
        return TRUE;
    }
    default:
        return FALSE;
    }
}
#else
using SignalHandler = void (*)(int);
SignalHandler previous_int = SIG_DFL, previous_term = SIG_DFL;

void handle_signal(int) { stop_requested.store(true, std::memory_order_relaxed); }
#endif

} // namespace

void ProcessSignals::install() {
    if (installed_) return;
    if (owned.test_and_set(std::memory_order_acquire))
        throw std::logic_error("process signals already installed");
    stop_requested.store(false, std::memory_order_relaxed);
#ifdef _WIN32
    if (!SetConsoleCtrlHandler(handle_console, TRUE)) {
        const auto error = GetLastError();
        owned.clear(std::memory_order_release);
        throw std::system_error(error, std::system_category(), "install console control handler");
    }
#else
    previous_int = std::signal(SIGINT, handle_signal);
    if (previous_int == SIG_ERR) {
        const auto error = errno;
        owned.clear(std::memory_order_release);
        throw std::system_error(error, std::generic_category(), "install SIGINT handler");
    }
    previous_term = std::signal(SIGTERM, handle_signal);
    if (previous_term == SIG_ERR) {
        const auto error = errno;
        std::signal(SIGINT, previous_int);
        owned.clear(std::memory_order_release);
        throw std::system_error(error, std::generic_category(), "install SIGTERM handler");
    }
#endif
    installed_ = true;
}

ProcessSignals::~ProcessSignals() {
    if (!installed_) return;
#ifdef _WIN32
    completed_shutdown.fetch_add(1, std::memory_order_release);
    SetConsoleCtrlHandler(handle_console, FALSE);
#else
    std::signal(SIGINT, previous_int);
    std::signal(SIGTERM, previous_term);
#endif
    owned.clear(std::memory_order_release);
}

bool ProcessSignals::requested() const noexcept {
    return installed_ && stop_requested.load(std::memory_order_relaxed);
}

} // namespace ninfer::product
