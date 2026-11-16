#include "core/buffer.h"
#include "core/io/file.h"
#include "core/io/read_queue.h"
#include "io/fixture.h"

#ifdef _WIN32
#    include "io/windows_interpose.h"
#endif

#include <algorithm>
#include <array>
#include <cerrno>
#include <fstream>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

namespace {
using namespace ninfer;
using namespace ninfer::io;

void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

template <class E = std::invalid_argument, class F>
void rejects(F fn) {
    try {
        fn();
    } catch (const E&) { return; }
    throw std::runtime_error("invalid I/O operation accepted");
}

template <class F>
void rejects_errno(int expected, F fn) {
    try {
        fn();
    } catch (const std::system_error& error) {
        require(error.code() == std::error_code(expected, std::generic_category()),
                "I/O operation reported the wrong errno");
        return;
    }
    throw std::runtime_error("invalid I/O operation accepted");
}

std::byte pattern(std::uint64_t position) {
    return std::byte((position * 37 + position / 13) & 255);
}

void write_file(const std::filesystem::path& path, std::size_t bytes) {
    std::vector<std::byte> data(bytes);
    for (std::size_t i = 0; i < bytes; ++i) data[i] = pattern(i);
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    out.close();
    require(bool(out), "fixture write failed");
}

ReadCompletion next(DirectReadQueue& queue) {
    std::array<ReadCompletion, 1> output;
    while (!queue.poll(output)) (void)queue.wait();
    return output[0];
}

void empty_and_alignment(const std::filesystem::path& root) {
    const auto path = root / "empty";
    write_file(path, 0);
    ReadOnlyFile buffered(path);
    require(!buffered.bytes(), "empty file has bytes");
    buffered.read_exact(0, {});
    rejects<std::system_error>([&] { buffered.read_exact(1, {}); });
    ReadOnlyFile direct(path, ReadMode::Direct);
    HostBuffer storage(8192, 8192);
    auto* data = static_cast<std::byte*>(storage.data());
    require(direct.read_direct(0, {data, 4096}) == 0, "empty direct read differs");
    require(direct.read_direct(4096, {data, 4096}) == 0, "past-EOF direct read differs");
    require(direct.read_direct(0, {}) == 0, "zero-length direct read differs");
    rejects([&] { (void)direct_read_window(direct.view(), 0, 1); });
    rejects([&] { (void)direct.read_direct(0, {data + 1, 4096}); });
    rejects([&] { (void)direct.read_direct(0, {data, 4095}); });
    rejects([&] { (void)direct.read_direct(1ULL << 63, {data, 4096}); });
    rejects([&] { (void)buffered.read_direct(0, {data, 4096}); });
    rejects([&] { direct.read_exact(0, {}); });
    auto stricter             = direct.view();
    stricter.memory_alignment = 8192;
    rejects([&] { validate_direct_read(stricter, 0, {data + 4096, 4096}); });
    stricter                  = direct.view();
    stricter.offset_alignment = 8192;
    rejects([&] { validate_direct_read(stricter, 4096, {data, 8192}); });
    rejects([&] { validate_direct_read(stricter, 0, {data, 4096}); });

    const std::array files{direct.view()};
    const std::array buffers{storage.span().subspan(0, 4096)};
    DirectReadQueue queue(files, buffers, 1);
    for (const auto offset : {0ULL, 4096ULL}) {
        require(queue.try_enqueue({0, 0, offset, 4096, offset}) == ReadAdmission::Accepted,
                "EOF enqueue rejected");
        queue.submit();
        require(queue.wait() == ReadWake::Completion, "EOF did not complete");
        require(queue.wait() == ReadWake::Completion, "wait lost a cached completion");
        require(queue.outstanding() == 1, "wait released a buffer before poll");
        const auto c = next(queue);
        require(c.tag == offset && !c.bytes && !c.error, "EOF completion differs");
    }
    require(!queue.outstanding(), "EOF completion retained capacity");
    require(queue.poll({}) == 0, "empty poll returned data");
}

void paths_and_moves(const std::filesystem::path& root) {
    const auto path = root / std::filesystem::path(u8"pesos-\u00e1-\u6743\u91cd-\U0001f680.bin");
    write_file(path, 4096);
    ReadOnlyFile first(path);
    ReadOnlyFile second(std::move(first));
    require(first.view().descriptor == -1, "move did not invalidate source");
    rejects([&] { first.read_exact(0, {}); });
    ReadOnlyFile third(path, ReadMode::Direct);
    third = std::move(second);
    require(second.view().descriptor == -1 && third.bytes() == 4096,
            "move assignment lost file ownership");
    std::array<std::byte, 90> data;
    third.read_exact(3990, data);
    for (std::size_t i = 0; i < data.size(); ++i)
        require(data[i] == pattern(3990 + i), "Unicode buffered read differs");
    third.read_exact(4096, {});
    rejects<std::system_error>([&] { third.read_exact(4096, data); });
    ReadOnlyFile direct(path, ReadMode::Direct);
    HostBuffer storage(4096, 4096);
    require(direct.read_direct(0, {static_cast<std::byte*>(storage.data()), 4096}) == 4096,
            "Unicode direct read differs");
    rejects<std::system_error>([&] { ReadOnlyFile missing(root / "absent"); });
    for (const auto mode : {ReadMode::Buffered, ReadMode::Direct}) {
        rejects_errno(ENOENT, [&] { ReadOnlyFile empty_path({}, mode); });
        // file.cpp rejects non-regular files with EINVAL after fstat (or at O_DIRECT open).
        rejects_errno(EINVAL, [&] { ReadOnlyFile directory(root, mode); });
    }
    rejects([&] { ReadOnlyFile invalid(path, static_cast<ReadMode>(99)); });
#ifdef _WIN32
    const auto locked_path = root / "access-denied";
    write_file(locked_path, 4096);
    const auto locked = ::CreateFileW(locked_path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL, nullptr);
    require(locked != INVALID_HANDLE_VALUE, "cannot open access-denied fixture");
    try {
        rejects<std::system_error>([&] { ReadOnlyFile denied(locked_path); });
    } catch (...) {
        ::CloseHandle(locked);
        throw;
    }
    require(::CloseHandle(locked), "cannot close access-denied fixture");
#endif
}

void many_and_independent_queues(const std::filesystem::path& root) {
    const auto path               = root / "many";
    constexpr std::uint32_t depth = 32;
    constexpr std::uint32_t count = 257;
    write_file(path, count * 4096);
    ReadOnlyFile owner(path, ReadMode::Direct);
    const std::array files{owner.view()};
    HostBuffer storage((depth + 1) * 4096, 4096);
    std::vector<MemorySpan> buffers;
    for (std::size_t i = 0; i < depth; ++i)
        buffers.push_back(storage.span().subspan(i * 4096, 4096));
    const std::array other_buffers{storage.span().subspan(depth * 4096, 4096)};
    DirectReadQueue queue(files, buffers, depth);
    DirectReadQueue other(files, other_buffers, 1);
    require(other.try_enqueue({0, 0, 4096, 4096, 999}) == ReadAdmission::Accepted,
            "second queue rejected read");
    other.submit();
    std::uint32_t issued = 0, completed = 0;
    std::array<bool, count> seen{};
    auto enqueue = [&](std::uint32_t index) {
        require(queue.try_enqueue({0, index, std::uint64_t(issued) * 4096, 4096, issued}) ==
                    ReadAdmission::Accepted,
                "many-read enqueue rejected");
        ++issued;
    };
    for (std::uint32_t i = 0; i < depth; ++i) enqueue(i);
    queue.submit();
    while (completed < count) {
        const auto c = next(queue);
        require(c.tag < count && !seen[c.tag] && !c.error && c.bytes == 4096,
                "many-read completion differs");
        seen[c.tag]      = true;
        const auto* data = static_cast<const std::byte*>(buffers[c.buffer_index].data);
        for (std::size_t j = 0; j < 4096; ++j)
            require(data[j] == pattern(c.tag * 4096 + j), "many-read bytes differ");
        ++completed;
        if (issued < count) enqueue(c.buffer_index);
        queue.submit();
    }
    const auto c = next(other);
    require(c.tag == 999 && c.bytes == 4096 && !c.error, "independent queue lost completion");
#ifdef _WIN32
    DWORD handles_before = 0;
    require(::GetProcessHandleCount(::GetCurrentProcess(), &handles_before),
            "cannot count handles before queue teardown");
#endif
    for (unsigned round = 0; round < 8; ++round) {
        // A new queue on the same borrowed file must also work after previous IOCP associations.
        std::fill_n(static_cast<std::byte*>(other_buffers[0].data), 4096, std::byte{});
        {
            DirectReadQueue closing(files, other_buffers, 1);
            require(closing.try_enqueue({0, 0, round * 4096ULL, 4096, round}) ==
                        ReadAdmission::Accepted,
                    "pending destructor probe rejected");
            closing.submit();
        }
        const auto* data = static_cast<const std::byte*>(other_buffers[0].data);
        for (std::size_t i = 0; i < 4096; ++i)
            require(data[i] == pattern(round * 4096ULL + i), "pending destructor lost bytes");
    }
#ifdef _WIN32
    DWORD handles_after = 0;
    require(::GetProcessHandleCount(::GetCurrentProcess(), &handles_after) &&
                handles_after == handles_before,
            "queue teardown leaked Windows handles");
#endif
    // poll may observe a wake packet before wait: it must preserve the notification.
    queue.wake();
    std::array<ReadCompletion, 1> output;
    require(queue.poll(output) == 0, "wake produced a read completion");
    require(queue.wait() == ReadWake::Signalled, "poll lost wake notification");
    std::jthread notifier([&] { queue.wake(); });
    require(queue.wait() == ReadWake::Signalled, "cross-thread idle wake failed");
}

#ifdef _WIN32
unsigned fail_after = 0;

BOOL WINAPI limited_read(HANDLE file, LPVOID buffer, DWORD bytes, LPDWORD received,
                         LPOVERLAPPED operation) {
    if (fail_after && !--fail_after) {
        ::SetLastError(ERROR_NOT_ENOUGH_QUOTA);
        return FALSE;
    }
    static const auto real = reinterpret_cast<decltype(&::ReadFile)>(
        ::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "ReadFile"));
    return real(file, buffer, bytes, received, operation);
}

void windows_failures(const std::filesystem::path& root) {
    const auto path = root / "failures";
    write_file(path, 3 * 4096);
    ReadOnlyFile owner(path, ReadMode::Direct);
    const std::array files{owner.view()};
    HostBuffer storage(3 * 4096, 4096);
    const std::array buffers{storage.span().subspan(0, 4096), storage.span().subspan(4096, 4096),
                             storage.span().subspan(8192, 4096)};
    DirectReadQueue queue(files, buffers, 3);
    for (std::uint32_t i = 0; i < 3; ++i)
        require(queue.try_enqueue({0, i, i * 4096ULL, 4096, i}) == ReadAdmission::Accepted,
                "partial-submit probe rejected");
    {
        ninfer::test::io::ImportReplacement injection(
            reinterpret_cast<const void*>(&validate_direct_read), "ReadFile",
            reinterpret_cast<void*>(&limited_read));
        fail_after = 2;
        rejects<std::system_error>([&] { queue.submit(); });
    }
    require(queue.outstanding() == 3, "failed submit lost accepted work");
    queue.submit();
    std::array<bool, 3> seen{};
    for (unsigned i = 0; i < 3; ++i) {
        const auto c = next(queue);
        require(c.tag < 3 && !seen[c.tag] && !c.error && c.bytes == 4096,
                "partial-submit retry duplicated or lost work");
        seen[c.tag] = true;
    }
    require(!queue.outstanding(), "partial-submit retry retained capacity");

    // Real kernel error: a mandatory byte-range lock held through a different file handle.
    const auto lock = ::CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    require(lock != INVALID_HANDLE_VALUE, "cannot open locking handle");
    OVERLAPPED operation{};
    if (!::LockFileEx(lock, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 4096, 0,
                      &operation)) {
        ::CloseHandle(lock);
        throw std::runtime_error("cannot lock error-probe range");
    }
    try {
        require(queue.try_enqueue({0, 0, 0, 4096, 123}) == ReadAdmission::Accepted,
                "locked-read probe rejected");
        queue.submit();
        const auto c = next(queue);
        require(c.tag == 123 && c.error == EACCES && !c.bytes,
                "kernel read error did not preserve errno contract");
    } catch (...) {
        ::CloseHandle(lock);
        throw;
    }
    require(::CloseHandle(lock), "cannot close locking handle");
    require(queue.try_enqueue({0, 0, 0, 4096, 124}) == ReadAdmission::Accepted,
            "queue did not recover after kernel error");
    queue.submit();
    const auto c = next(queue);
    require(c.tag == 124 && !c.error && c.bytes == 4096, "recovered read failed");
}
#endif
} // namespace

int main() {
    try {
        ninfer::test::io::Directory directory;
        empty_and_alignment(directory.path);
        paths_and_moves(directory.path);
        many_and_independent_queues(directory.path);
#ifdef _WIN32
        windows_failures(directory.path);
#endif
        std::cout << "I/O edge cases passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
