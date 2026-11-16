#include "core/buffer.h"
#include "core/io/file.h"
#include "core/io/read_queue.h"

#ifndef NOMINMAX
#    define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <optional>
#include <vector>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

void check(NTSTATUS status) { require(status >= 0, "BCrypt SHA-256 operation failed"); }

struct Sha256 {
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash     = nullptr;

    ~Sha256() {
        if (hash) ::BCryptDestroyHash(hash);
        if (algorithm) ::BCryptCloseAlgorithmProvider(algorithm, 0);
    }

    void initialize() {
        check(::BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0));
        check(::BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0));
    }

    void update(void* data, ULONG bytes) {
        check(::BCryptHashData(hash, static_cast<PUCHAR>(data), bytes, 0));
    }

    std::array<unsigned char, 32> finish() {
        std::array<unsigned char, 32> result{};
        check(::BCryptFinishHash(hash, result.data(), result.size(), 0));
        return result;
    }
};

void run(const std::filesystem::path& path) {
    using namespace ninfer;
    using namespace ninfer::io;
    constexpr std::uint32_t depth = 8;
    // This stride makes one real request start 4 KiB before 4 GiB and end beyond it.
    constexpr std::uint32_t chunk    = 4 * 1024 * 1024 + 4096;
    constexpr std::uint64_t boundary = 1ULL << 32;
    ReadOnlyFile owner(path, ReadMode::Direct);
    const auto length = owner.bytes();
    const std::array files{owner.view()};
    HostBuffer storage(std::size_t(depth) * chunk,
                       std::max<std::size_t>(4096, files[0].memory_alignment));
    std::array<MemorySpan, depth> buffers;
    for (std::size_t i = 0; i < depth; ++i) buffers[i] = storage.span().subspan(i * chunk, chunk);
    DirectReadQueue queue(files, buffers, depth);
    std::array<std::optional<ReadCompletion>, depth> finished;
    Sha256 sha;
    sha.initialize();
    std::uint64_t issued = 0, consumed = 0, hashed_bytes = 0;
    bool crossed = false;
    auto enqueue = [&](std::uint32_t index) {
        const auto offset = issued * chunk;
        if (offset >= length) return;
        require(queue.try_enqueue({0, index, offset, chunk, issued}) == ReadAdmission::Accepted,
                "large-file enqueue rejected");
        crossed |= offset < boundary &&
                   offset + std::min<std::uint64_t>(chunk, length - offset) > boundary;
        ++issued;
    };
    for (std::uint32_t i = 0; i < depth; ++i) enqueue(i);
    queue.submit();
    while (hashed_bytes < length) {
        std::array<ReadCompletion, depth> completions;
        const auto count = queue.poll(completions);
        if (!count) {
            (void)queue.wait();
            continue;
        }
        for (std::size_t i = 0; i < count; ++i) {
            const auto c = completions[i];
            require(c.buffer_index < depth && !finished[c.buffer_index] && c.tag >= consumed &&
                        c.tag < issued && c.buffer_index == c.tag % depth && !c.error,
                    "large-file completion identity/error differs");
            require(c.bytes == std::min<std::uint64_t>(chunk, length - c.tag * chunk),
                    "large-file completion length differs");
            finished[c.buffer_index] = c;
        }
        while (finished[consumed % depth]) {
            const auto index = static_cast<std::uint32_t>(consumed % depth);
            const auto c     = *finished[index];
            require(c.tag == consumed, "large-file hash order differs");
            sha.update(buffers[index].data, c.bytes);
            hashed_bytes += c.bytes;
            ++consumed;
            finished[index].reset();
            enqueue(index);
        }
        queue.submit();
    }
    require(!queue.outstanding(), "large-file read retained work");
    require(length <= boundary || crossed, "no read crossed the 4 GiB boundary");
    std::cout << "bytes=" << hashed_bytes << " crossed_4gib=" << crossed << " sha256=";
    for (auto value : sha.finish())
        std::cout << std::hex << std::setfill('0') << std::setw(2) << unsigned(value);
    std::cout << '\n';
}
} // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 2, "usage: ninfer_io_large_file_test FILE");
        run(std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(argv[1]))));
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
