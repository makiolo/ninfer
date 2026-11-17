#pragma once

#include "serve/http_transport.h"

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>

// Own the listener before starting its thread, then stop and join before destroying it.
class LoopbackServer {
public:
    httplib::Server server;
    int port = -1;

    explicit LoopbackServer(const char* host = "127.0.0.1") {
        server.set_socket_options(ninfer::serve::configure_http_server_socket);
        server.new_task_queue = [] {
            return ninfer::serve::make_http_task_queue(2, 4).release();
        };
        server.set_write_timeout(2, 0);
        port = server.bind_to_any_port(host);
        if (port <= 0) throw std::runtime_error("cannot bind loopback listener");
    }

    void start() {
        worker_ = std::thread([this] { listened_ = server.listen_after_bind(); });
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!server.is_running()) {
            if (std::chrono::steady_clock::now() >= deadline)
                throw std::runtime_error("loopback listener did not start");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void stop() {
        server.stop();
        if (worker_.joinable()) worker_.join();
    }

    bool listened() const { return listened_; }
    ~LoopbackServer() { stop(); }

private:
    std::thread worker_;
    std::atomic<bool> listened_{false};
};
