#include "loopback_server.h"
#include "product/media_acquire/acquire.h"

#include <iostream>

namespace {
using namespace ninfer::product::media_acquire;
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

void test_url_rejections() {
    for (const auto* url : {"file:///C:/media.bin", "ftp://127.0.0.1/media",
                            "http://user@127.0.0.1/media"}) {
        bool rejected = false;
        try { (void)acquire_bytes({.kind = SourceKind::Url, .value = url}); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "media accepted a non-HTTP(S) or credential-bearing URL");
    }
}

void test_media_root() {
    const std::filesystem::path path(__FILE__);
    const Source source{.kind = SourceKind::Path, .value = path.string()};
    Policy policy{.media_root = path.parent_path()};
    require(!acquire_bytes(source, policy).empty(), "media root rejected its own file");
    policy.media_root /= "another-root";
    bool rejected = false;
    try { (void)acquire_bytes(source, policy); }
    catch (const std::invalid_argument&) { rejected = true; }
    require(rejected, "media path outside configured root was accepted");
}

void test_loopback(const char* host, const char* url_host) {
    LoopbackServer listener(host);
    listener.server.Get("/media", [](const auto&, auto& response) {
        response.set_content("media-fixture", "application/octet-stream");
    });
    listener.start();
    const Source source{.kind = SourceKind::Url,
                        .value = std::string("http://") + url_host + ':' +
                                 std::to_string(listener.port) + "/media"};
    bool rejected = false;
    try { (void)acquire_bytes(source); }
    catch (const std::invalid_argument&) { rejected = true; }
    require(rejected, "private media address was accepted without permission");
    Policy policy{.allow_private_network = true};
    const auto bytes = acquire_bytes(source, policy);
    require(std::string(bytes.begin(), bytes.end()) == "media-fixture",
            "media acquisition changed loopback response bytes");
}
} // namespace

int main() {
    try {
        test_url_rejections();
        test_media_root();
        test_loopback("127.0.0.1", "127.0.0.1");
        test_loopback("::1", "[::1]");
        std::cout << "media protocol rejection, IPv4 and IPv6 acquisition/filtering: passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
