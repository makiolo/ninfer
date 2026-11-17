#include "text/jinja.h"

#include <iostream>
#include <stdexcept>

int main() {
    try {
        const ninfer::text::JinjaTemplate source(
            "{{ strftime_now('%Y-%m-%d %H:%M:%S') }}|{{ strftime_now('') }}|"
            "{{ strftime_now(prefix ~ '%Y') }}", "host-time");
        ninfer::text::TemplateRenderOptions options;
        options.timestamp = 1704164645; // 2024-01-02 03:04:05 UTC; CTest sets TZ.
        const std::string prefix(200, 'x'); // Exercise the formatting buffer growth.
        const auto output = source.render({{"prefix", prefix}}, options);
        if (output.text != "2024-01-02 03:04:05||" + prefix + "2024")
            throw std::runtime_error("strftime_now changed the supplied timestamp or format");
        options.timestamp += 86400;
        if (source.render({{"prefix", ""}}, options).text != "2024-01-03 03:04:05||2024")
            throw std::runtime_error("strftime_now reused a previous render's timestamp");
        std::cout << "strftime_now frontend contract passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
