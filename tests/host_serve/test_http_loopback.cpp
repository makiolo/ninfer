#include "loopback_server.h"

#include <future>
#include <iostream>
#include <string>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

#if defined(_WIN32)
void test_windows_socket_options() {
    struct Socket {
        SOCKET value = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        ~Socket() { if (value != INVALID_SOCKET) ::closesocket(value); }
    } socket;
    require(socket.value != INVALID_SOCKET, "cannot create Winsock option probe");
    ninfer::serve::configure_http_server_socket(socket.value);
    const auto equals = [&](int level, int option, int expected) {
        int actual = 0;
        int size = sizeof(actual);
        const int result = ::getsockopt(socket.value, level, option,
                                       reinterpret_cast<char*>(&actual), &size);
        // Winsock may report SO_KEEPALIVE as one byte, unlike its integer TCP options.
        const bool valid_size = size == sizeof(actual) ||
            (level == SOL_SOCKET && option == SO_KEEPALIVE && size == sizeof(bool));
        const bool matches = result == 0 && valid_size && actual == expected;
        if (!matches) std::cerr << "socket option " << option << ": result=" << result
                                << ", value=" << actual << ", expected=" << expected
                                << ", size=" << size
                                << ", WSAError=" << ::WSAGetLastError() << '\n';
        return matches;
    };
    require(equals(SOL_SOCKET, SO_EXCLUSIVEADDRUSE, 1) &&
                equals(SOL_SOCKET, SO_REUSEADDR, 0),
            "Windows listener did not reserve its address exclusively");
    require(equals(SOL_SOCKET, SO_KEEPALIVE, 1) &&
                equals(IPPROTO_TCP, TCP_KEEPIDLE, 10) &&
                equals(IPPROTO_TCP, TCP_KEEPINTVL, 3) &&
                equals(IPPROTO_TCP, TCP_KEEPCNT, 3),
            "Windows listener lost its keepalive policy");
}
#endif
} // namespace

int main() {
    using namespace std::chrono_literals;
    using namespace ninfer::serve;
    try {
#if defined(_WIN32)
        test_windows_socket_options();
#endif
        std::atomic<bool> cancelled{false};
        std::promise<void> released;
        auto release = released.get_future();
        LoopbackServer listener;
        // A protocol fixture stands in for the unavailable Engine's model description.
        listener.server.Get("/v1/models", [](const auto&, auto& response) {
            set_owned_json_content(response,
                R"({"object":"list","data":[{"id":"host-fixture","object":"model","owned_by":"ninfer"}]})",
                {});
        });
        listener.server.Get("/stream", [&](const auto&, auto& response) {
            prepare_sse_response(response);
            response.set_chunked_content_provider("text/event-stream",
                [&](std::size_t, httplib::DataSink& sink) {
                    SseTransport transport(sink, cancelled, 10ms);
                    const auto deadline = SseTransport::Clock::now() + 5s;
                    try {
                        while (SseTransport::Clock::now() < deadline) {
                            transport.write("data: " + std::string(65536, 'x') + "\n\n");
                            if (transport.poll()) break;
                            std::this_thread::sleep_for(1ms);
                        }
                    } catch (const ClientDisconnected&) {}
                    return false;
                }, [&](bool) { released.set_value(); });
        });
        listener.start();

        httplib::Client client("127.0.0.1", listener.port);
        client.set_read_timeout(5, 0);
        client.set_connection_timeout(2, 0);
        auto models = client.Get("/v1/models");
        require(models && models->status == 200, "GET /v1/models failed");
        const auto body = nlohmann::json::parse(models->body);
        require(body.at("object") == "list" && body.at("data").size() == 1 &&
                    body.at("data")[0].at("id") == "host-fixture",
                "model-list protocol fixture changed");

        httplib::Server collision;
        collision.set_socket_options(configure_http_server_socket);
        require(!collision.bind_to_port("127.0.0.1", listener.port),
                "occupied port allowed another listener");

        std::size_t received = 0;
        auto stream = client.Get("/stream", [&](const char*, std::size_t size) {
            received += size;
            return false; // Close a real TCP connection while the server is writing SSE.
        });
        require(!stream && stream.error() == httplib::Error::Canceled && received > 0,
                "client did not interrupt an active SSE response");
        require(release.wait_for(8s) == std::future_status::ready && cancelled,
                "SSE disconnect did not cancel and release its producer");
        models = client.Get("/v1/models");
        require(models && models->status == 200,
                "listener stopped serving after client disconnect");
        listener.stop();
        require(listener.listened(), "listener failed while serving");
        std::cout << "/v1/models, exclusive bind, SSE disconnect/cancel/release: passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
