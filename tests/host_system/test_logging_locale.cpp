#include "product/logging/logging.h"
#include "support/stderr_capture_windows.h"
#include <spdlog/logger.h>
#include <chrono>
#include <clocale>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <locale>
#include <stdexcept>

namespace {
void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

void console_utf8() {
    struct Console {
        HANDLE previous = GetStdHandle(STD_ERROR_HANDLE);
        UINT codepage = GetConsoleOutputCP();
        HANDLE handle = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        ~Console() {
            SetStdHandle(STD_ERROR_HANDLE, previous);
            if (codepage) SetConsoleOutputCP(codepage);
            if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
        }
    } console;
    require(console.handle != INVALID_HANDLE_VALUE, "private console unavailable");
    require(SetConsoleOutputCP(CP_UTF8) && SetStdHandle(STD_ERROR_HANDLE, console.handle),
            "cannot select UTF-8 console output");
    require(SetConsoleCursorPosition(console.handle, {0, 0}), "cannot position test console");
    {
        ninfer::product::LoggingRuntime logging(
            {.logger_name = "utf8-console", .color = ninfer::product::LogColorMode::Never,
             .presentation = ninfer::product::LogPresentation::Tool});
        logging.logger()->info("Niño café");
        logging.flush();
    }
    wchar_t text[9]{};
    DWORD read = 0;
    require(ReadConsoleOutputCharacterW(console.handle, text, 9, {0, 0}, &read) && read == 9 &&
                std::wstring(text, read) == L"Niño café",
            "UTF-8 bytes did not reach the console as Unicode characters");
}

void run(const char* zone, const char* locale, const char* prefix) {
    require(_putenv_s("TZ", zone) == 0, "cannot set test timezone");
    _tzset();
    require(std::setlocale(LC_ALL, locale) != nullptr, "test locale unavailable");
    std::locale::global(std::locale(locale));
    StderrCapture capture;
    {
        ninfer::product::LoggingRuntime logging(
            {.logger_name = "host-locale", .color = ninfer::product::LogColorMode::Never});
        logging.logger()->log(
            std::chrono::system_clock::time_point(std::chrono::seconds(1767225600)),
            spdlog::source_loc{}, spdlog::level::info, "Niño 日本語 | 1.5");
        logging.flush();
    }
    require(capture.finish() == std::string(prefix) + "  INFO  Niño 日本語 | 1.5\n",
            "timezone, locale or UTF-8 bytes changed the log contract");
}
}

int main() {
    try {
        console_utf8();
        run("UTC0", "C", "2026-01-01 00:00:00.000");
        run("PST8PDT", "Spanish_Spain.utf8", "2025-12-31 16:00:00.000");
        require(std::use_facet<std::numpunct<char>>(std::locale()).decimal_point() == ',',
                "Spanish numeric locale was not applied");
        std::cout << "UTC/Pacific, C/Spanish locale and exact UTF-8 log bytes passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
