#include "product/logging/process_signals.h"
#include "product/logging/logging.h"
#include "product/logging/startup_log.h"

#include <spdlog/logger.h>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <thread>

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
#    include <windows.h>
#else
#    include <csignal>
#endif

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

void send(bool ctrl_break) {
#ifdef _WIN32
    require(GenerateConsoleCtrlEvent(ctrl_break ? CTRL_BREAK_EVENT : CTRL_C_EVENT, 0),
            "generate console control event");
#else
    require(std::raise(ctrl_break ? SIGTERM : SIGINT) == 0, "raise stop signal");
#endif
}
} // namespace

int main(int argc, char** argv) {
    try {
#ifdef _WIN32
        // Build runners may inherit the process-group attribute that ignores Ctrl+C.
        // This isolated console models an interactive launch with Ctrl+C enabled.
        require(SetConsoleCtrlHandler(nullptr, FALSE), "enable console Ctrl+C delivery");
#endif
        if (argc == 2 && std::string_view(argv[1]) == "--redirected-log") {
            ninfer::product::LoggingRuntime logging({.logger_name = "redirected"});
            require(!logging.terminal_progress()->enabled(), "redirected stderr detected as TTY");
            logging.terminal_progress()->update("transient progress must not reach file");
            ninfer::product::StartupLogRenderer startup(logging);
            startup.observer().callback({.phase = ninfer::StartupPhase::EngineStartup,
                                         .status = ninfer::StartupStatus::Begin});
            logging.logger()->warn("redirected record");
            logging.flush();
            return 0;
        }
        const bool manual = argc == 2 && std::string_view(argv[1]) == "--manual";
        const bool ctrl_break = argc == 2 && std::string_view(argv[1]) == "--break";
        const bool close = argc == 2 && std::string_view(argv[1]) == "--close";
        for (unsigned cycle = 0; cycle < (manual || close ? 1U : 2U); ++cycle) {
            ninfer::product::ProcessSignals signals;
            struct Resource {
                ~Resource() { std::cout << "resources released" << std::endl; }
            } resource; // Must be destroyed before the close handler can return.
            require(!signals.requested(), "uninstalled registration requested stop");
            signals.install();
            signals.install(); // Idempotent for the owning registration.
            require(!signals.requested(), "registration inherited a previous stop");
            bool rejected = false;
            try {
                ninfer::product::ProcessSignals other;
                other.install();
            } catch (const std::logic_error&) { rejected = true; }
            require(rejected, "second owner installed process handlers");
            std::cout << "ready" << std::endl;
#ifdef _WIN32
            if (close)
                require(PostMessageW(GetConsoleWindow(), WM_CLOSE, 0, 0), "close test console");
#endif
            if (!manual && !close) send(ctrl_break);
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
            while (!signals.requested() && std::chrono::steady_clock::now() < deadline)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            require(signals.requested(), "stop event did not reach the process loop");
            // A second request during cleanup must neither terminate nor reset the request.
            if (!manual && !close) send(ctrl_break);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            require(signals.requested(), "repeated event cleared the stop request");
            std::cout << "cleanup complete" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
