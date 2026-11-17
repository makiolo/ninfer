// Selected only for Windows binary-protocol tests by HostTests.cmake.
// The CRT's default text streams translate CRLF and treat 0x1a as EOF.
#include <cstdio>
#include <fcntl.h>
#include <io.h>
#include <cstdlib>

namespace {
const bool binary_stdio = [] {
    if (_setmode(_fileno(stdin), _O_BINARY) < 0 ||
        _setmode(_fileno(stdout), _O_BINARY) < 0) {
        std::fputs("cannot select binary test protocol streams\n", stderr);
        std::abort();
    }
    return true;
}();
}
