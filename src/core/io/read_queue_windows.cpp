#include "core/io/read_queue.h"
#include "core/io/windows.h"

#include <algorithm>
#include <array>
#include <exception>
#include <vector>

namespace ninfer::io {

struct DirectReadQueue::Impl {
    enum class State { Free, Prepared, Issued, Ready };

    struct Slot {
        OVERLAPPED operation{}; // Stable address until the completion has been consumed.
        DirectRead request{};
        State state = State::Free;
    };

    // ReOpenFile gives each queue its own file objects: an IOCP association cannot be changed
    // or removed. Neither another queue nor synchronous reads touch these handles.
    windows::Handle port;
    std::vector<windows::Handle> handles;
    std::vector<FileView> files;
    std::vector<MemorySpan> buffers;
    std::vector<Slot> slots;
    std::vector<ReadCompletion> ready;
    std::size_t ready_begin = 0, ready_count = 0;
    std::size_t pending    = 0;
    std::uint32_t capacity = 0;
    bool closed            = false;

    void complete(std::uint32_t index, DWORD bytes, DWORD error) {
        auto& slot = slots[index];
        if (slot.state != State::Issued && slot.state != State::Prepared)
            throw std::logic_error("unmatched direct read completion");
        if (error == ERROR_HANDLE_EOF) error = ERROR_SUCCESS;
        ready[(ready_begin + ready_count) % ready.size()] = {
            slot.request.tag, index, error ? 0 : bytes, windows::error_number(error)};
        ++ready_count;
        slot.state = State::Ready;
    }

    // IOCP removes a packet when waiting, so retain read completions until poll releases buffers.
    // Wake packets have no OVERLAPPED; they never consume request capacity.
    enum class Packet { None, Completion, Signal };

    Packet receive(DWORD timeout) {
        DWORD bytes           = 0;
        ULONG_PTR key         = 0;
        OVERLAPPED* operation = nullptr;
        const auto success =
            ::GetQueuedCompletionStatus(port.get(), &bytes, &key, &operation, timeout);
        const auto error = success ? ERROR_SUCCESS : ::GetLastError();
        if (!operation) {
            if (!success) {
                if (error == WAIT_TIMEOUT) return Packet::None;
                windows::fail("wait for direct read IOCP", error);
            }
            if (key != 1) throw std::logic_error("invalid direct read notification");
            return Packet::Signal;
        }
        // Only this queue submits to its handles, and operation is the first member of Slot.
        const auto* slot = reinterpret_cast<Slot*>(operation);
        const auto index = static_cast<std::uint32_t>(slot - slots.data());
        if (index >= slots.size() || slots[index].state != State::Issued)
            throw std::logic_error("unmatched direct read IOCP packet");
        complete(index, bytes, error);
        return Packet::Completion;
    }

    bool signalled = false;
};

DirectReadQueue::DirectReadQueue(std::span<const FileView> files,
                                 std::span<const MemorySpan> buffers, std::uint32_t capacity)
    : impl_(std::make_unique<Impl>()) {
    if (!capacity || capacity > buffers.size() || files.empty() ||
        files.size() > std::numeric_limits<int>::max() || buffers.size() > 65535)
        throw std::invalid_argument("invalid direct read queue capacity");
    auto& p    = *impl_;
    p.capacity = capacity;
    p.files.assign(files.begin(), files.end());
    p.buffers.assign(buffers.begin(), buffers.end());
    p.slots.resize(buffers.size());
    p.ready.resize(buffers.size());
    for (const auto file : files)
        if (file.mode != ReadMode::Direct || file.descriptor < 0)
            throw std::invalid_argument("read queue requires direct files");
    for (std::size_t i = 0; i < buffers.size(); ++i) {
        const auto buffer = buffers[i];
        validate_memory(buffer);
        if (buffer.device != Device::cpu() || !buffer.bytes ||
            buffer.bytes > std::numeric_limits<unsigned>::max())
            throw std::invalid_argument("read queue requires nonempty Host buffers");
        for (const auto file : files)
            validate_direct_read(file, 0, {static_cast<std::byte*>(buffer.data), buffer.bytes});
        const auto begin = reinterpret_cast<std::uintptr_t>(buffer.data);
        for (std::size_t j = 0; j < i; ++j) {
            const auto other = reinterpret_cast<std::uintptr_t>(buffers[j].data);
            if (begin < other + buffers[j].bytes && other < begin + buffer.bytes)
                throw std::invalid_argument("read queue buffers overlap");
        }
    }
    p.port = windows::Handle(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1));
    if (!p.port.valid()) windows::fail("initialize direct read IOCP");
    p.handles.reserve(files.size());
    for (std::size_t i = 0; i < files.size(); ++i) {
        const auto context = "register direct read file " + std::to_string(i);
        windows::Handle handle(::ReOpenFile(windows::native_handle(files[i].descriptor),
                                            GENERIC_READ,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                            FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING));
        if (!handle.valid()) windows::fail(context, "ReOpenFile");
        if (!::CreateIoCompletionPort(handle.get(), p.port.get(), 0, 1))
            windows::fail(context, "associate IOCP");
        p.handles.push_back(std::move(handle));
    }
}

DirectReadQueue::~DirectReadQueue() {
    try {
        close();
        drain();
    } catch (...) {
        // As on Linux, never release borrowed buffers while kernel writers may still exist.
        std::terminate();
    }
}

ReadAdmission DirectReadQueue::try_enqueue(DirectRead request) {
    auto& p = *impl_;
    if (p.closed) return ReadAdmission::Closed;
    if (p.pending == p.capacity) return ReadAdmission::Full;
    if (request.file_index >= p.files.size() || request.buffer_index >= p.buffers.size() ||
        !request.bytes)
        throw std::invalid_argument("invalid direct read request");
    auto& slot        = p.slots[request.buffer_index];
    const auto buffer = p.buffers[request.buffer_index];
    if (slot.state != Impl::State::Free || request.bytes > buffer.bytes)
        throw std::invalid_argument("direct read buffer is busy or too small");
    validate_direct_read(p.files[request.file_index], request.file_offset,
                         {static_cast<std::byte*>(buffer.data), request.bytes});
    slot.operation = windows::overlapped(request.file_offset);
    slot.request   = request;
    slot.state     = Impl::State::Prepared;
    ++p.pending;
    return ReadAdmission::Accepted;
}

void DirectReadQueue::submit() {
    auto& p = *impl_;
    for (std::uint32_t i = 0; i < p.slots.size(); ++i) {
        auto& slot = p.slots[i];
        if (slot.state != Impl::State::Prepared) continue;
        const auto& request = slot.request;
        slot.operation      = windows::overlapped(request.file_offset);
        const auto success  = ::ReadFile(p.handles[request.file_index].get(), p.buffers[i].data,
                                         request.bytes, nullptr, &slot.operation);
        const auto error    = success ? ERROR_SUCCESS : ::GetLastError();
        if (!success && error != ERROR_IO_PENDING) {
            // Resource exhaustion did not issue this read. Keep it prepared for a later retry,
            // including when previous reads in this submit call have already been issued.
            if (error == ERROR_NOT_ENOUGH_MEMORY || error == ERROR_NOT_ENOUGH_QUOTA ||
                error == ERROR_NO_SYSTEM_RESOURCES) {
                windows::fail("submit direct read file " + std::to_string(request.file_index) +
                                  " offset " + std::to_string(request.file_offset),
                              error);
            }
            p.complete(i, 0, error);
        } else {
            // A synchronous success also posts exactly one IOCP packet (skip-on-success is off).
            slot.state = Impl::State::Issued;
        }
    }
}

std::size_t DirectReadQueue::poll(std::span<ReadCompletion> output) {
    auto& p           = *impl_;
    std::size_t count = 0;
    while (count < output.size()) {
        if (p.ready_count) {
            const auto completion = p.ready[p.ready_begin];
            p.ready_begin         = (p.ready_begin + 1) % p.ready.size();
            --p.ready_count;
            output[count++]                        = completion;
            p.slots[completion.buffer_index].state = Impl::State::Free;
            --p.pending;
            continue;
        }
        const auto packet = p.receive(0);
        if (packet == Impl::Packet::None) break;
        if (packet == Impl::Packet::Signal) p.signalled = true;
    }
    return count;
}

ReadWake DirectReadQueue::wait() {
    auto& p = *impl_;
    if (std::exchange(p.signalled, false)) return ReadWake::Signalled;
    if (p.ready_count) return ReadWake::Completion;
    const auto packet = p.receive(INFINITE);
    return packet == Impl::Packet::Signal ? ReadWake::Signalled : ReadWake::Completion;
}

void DirectReadQueue::wake() {
    if (!::PostQueuedCompletionStatus(impl_->port.get(), 0, 1, nullptr))
        windows::fail("wake direct read queue");
}

void DirectReadQueue::close() {
    impl_->closed = true;
    submit();
}

void DirectReadQueue::drain() {
    submit();
    std::array<ReadCompletion, 128> completions;
    while (impl_->pending)
        if (!poll(completions)) (void)wait();
}

std::size_t DirectReadQueue::outstanding() const noexcept { return impl_->pending; }

} // namespace ninfer::io
