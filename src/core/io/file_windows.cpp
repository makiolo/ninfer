#include "core/io/file.h"
#include "core/io/windows.h"

#include <algorithm>
#include <bit>
#include <fcntl.h>

namespace ninfer::io {
namespace {

std::string utf8_path(const std::filesystem::path& path) {
    const auto text = path.u8string();
    return {reinterpret_cast<const char*>(text.data()), text.size()};
}

std::wstring wide_path(const std::string& utf8) {
    if (utf8.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::invalid_argument("file path is too long");
    const auto count = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
                                             static_cast<int>(utf8.size()), nullptr, 0);
    if (!count) windows::fail("convert UTF-8 file path");
    std::wstring result(count, L'\0');
    if (!::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
                               static_cast<int>(utf8.size()), result.data(), count))
        windows::fail("convert UTF-8 file path");
    return result;
}

std::size_t read_at(FileView file, std::uint64_t offset, std::span<std::byte> destination,
                    const std::string& context) {
    auto operation = windows::overlapped(offset);
    windows::Handle event(::CreateEventW(nullptr, TRUE, FALSE, nullptr));
    if (!event.valid()) windows::fail(context, "create read event");
    operation.hEvent  = event.get();
    DWORD bytes       = 0;
    const auto handle = windows::native_handle(file.descriptor);
    if (!::ReadFile(handle, destination.data(), static_cast<DWORD>(destination.size()), nullptr,
                    &operation)) {
        const auto error = ::GetLastError();
        if (error == ERROR_HANDLE_EOF) return 0;
        if (error != ERROR_IO_PENDING) windows::fail(context, "ReadFile", error);
    }
    if (!::GetOverlappedResult(handle, &operation, &bytes, TRUE)) {
        const auto completion_error = ::GetLastError();
        if (completion_error == ERROR_HANDLE_EOF) return 0;
        windows::fail(context, "complete ReadFile", completion_error);
    }
    return bytes;
}

} // namespace

ReadOnlyFile::ReadOnlyFile(const std::filesystem::path& path, ReadMode mode) : path_(path) {
    if (mode != ReadMode::Buffered && mode != ReadMode::Direct)
        throw std::invalid_argument("invalid file read mode");
    const auto context = utf8_path(path);
    const auto wide    = wide_path(context);
    windows::Handle handle(::CreateFileW(
        wide.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED |
            (mode == ReadMode::Direct ? FILE_FLAG_NO_BUFFERING : 0),
        nullptr));
    if (!handle.valid()) windows::fail(context, "CreateFileW");
    BY_HANDLE_FILE_INFORMATION information{};
    if (!::GetFileInformationByHandle(handle.get(), &information))
        windows::fail(context, "GetFileInformationByHandle");
    if (::GetFileType(handle.get()) != FILE_TYPE_DISK ||
        (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        windows::fail(context, "expected regular file", ERROR_INVALID_PARAMETER);
    LARGE_INTEGER length{};
    if (!::GetFileSizeEx(handle.get(), &length)) windows::fail(context, "GetFileSizeEx");
    view_.bytes = static_cast<std::uint64_t>(length.QuadPart);
    view_.mode  = mode;
    if (mode == ReadMode::Direct) {
        FILE_STORAGE_INFO storage{};
        if (!::GetFileInformationByHandleEx(handle.get(), FileStorageInfo, &storage,
                                            sizeof(storage)))
            windows::fail(context, "query direct I/O sector alignment");
        // Windows requires logical-sector offsets/lengths and physical-sector buffer addresses.
        view_.offset_alignment = storage.LogicalBytesPerSector;
        view_.memory_alignment =
            std::max(storage.LogicalBytesPerSector, storage.PhysicalBytesPerSectorForPerformance);
        if (!std::has_single_bit(view_.memory_alignment) ||
            !std::has_single_bit(view_.offset_alignment))
            windows::fail(context, "unsupported direct I/O alignment", ERROR_NOT_SUPPORTED);
    }
    // The CRT descriptor retains the public FileView representation and owns this HANDLE.
    const auto descriptor = ::_open_osfhandle(reinterpret_cast<std::intptr_t>(handle.get()),
                                              _O_RDONLY | _O_BINARY | _O_NOINHERIT);
    if (descriptor < 0)
        throw std::system_error(errno, std::generic_category(), context + ": _open_osfhandle");
    handle.release();
    view_.descriptor = descriptor;
}

ReadOnlyFile::~ReadOnlyFile() {
    if (view_.descriptor >= 0) ::_close(view_.descriptor);
}

ReadOnlyFile::ReadOnlyFile(ReadOnlyFile&& other) noexcept
    : path_(std::move(other.path_)), view_(std::exchange(other.view_, {})) {}

ReadOnlyFile& ReadOnlyFile::operator=(ReadOnlyFile&& other) noexcept {
    if (this != &other) {
        if (view_.descriptor >= 0) ::_close(view_.descriptor);
        path_ = std::move(other.path_);
        view_ = std::exchange(other.view_, {});
    }
    return *this;
}

void ReadOnlyFile::read_exact(std::uint64_t offset, std::span<std::byte> destination) const {
    if (view_.mode != ReadMode::Buffered || view_.descriptor < 0)
        throw std::invalid_argument("read_exact requires a buffered file");
    const auto context = utf8_path(path_);
    if (offset > view_.bytes || destination.size() > view_.bytes - offset)
        windows::fail(context, "read exceeds file length", ERROR_HANDLE_EOF);
    while (!destination.empty()) {
        const auto count = std::min<std::size_t>(destination.size(), 64ULL * 1024 * 1024);
        const auto n     = read_at(view_, offset, destination.first(count), context);
        if (!n) windows::fail(context, "unexpected EOF", ERROR_HANDLE_EOF);
        offset += n;
        destination = destination.subspan(n);
    }
}

void validate_direct_read(FileView file, std::uint64_t offset, std::span<std::byte> destination) {
    if (file.descriptor < 0 || file.mode != ReadMode::Direct ||
        !std::has_single_bit(file.memory_alignment) ||
        !std::has_single_bit(file.offset_alignment) || offset % file.offset_alignment ||
        destination.size() % file.offset_alignment ||
        (!destination.empty() &&
         (!destination.data() ||
          reinterpret_cast<std::uintptr_t>(destination.data()) % file.memory_alignment)) ||
        destination.size() > static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max()))
        throw std::invalid_argument("invalid direct read range or alignment");
    windows::validate_offset(offset);
}

std::size_t ReadOnlyFile::read_direct(std::uint64_t offset,
                                      std::span<std::byte> destination) const {
    validate_direct_read(view_, offset, destination);
    // ReadFile accepts a DWORD count. Split only at aligned boundaries, retaining short-read EOF.
    const auto alignment = std::max(view_.offset_alignment, view_.memory_alignment);
    const auto limit     = std::numeric_limits<DWORD>::max() / alignment * alignment;
    if (!limit) throw std::invalid_argument("direct read alignment exceeds Windows read limit");
    std::size_t total = 0;
    while (!destination.empty()) {
        const auto count = std::min(destination.size(), limit);
        const auto n     = read_at(view_, offset, destination.first(count), utf8_path(path_));
        total += n;
        if (n < count) break;
        destination = destination.subspan(n);
        offset += n;
    }
    return total;
}

DirectReadWindow direct_read_window(FileView file, std::uint64_t offset, std::size_t bytes) {
    if (file.descriptor < 0 || file.mode != ReadMode::Direct || !bytes ||
        !std::has_single_bit(file.offset_alignment) || offset > file.bytes ||
        bytes > file.bytes - offset)
        throw std::invalid_argument("invalid direct read window");
    const auto alignment = file.offset_alignment;
    const auto prefix    = offset % alignment;
    if (bytes > std::numeric_limits<std::size_t>::max() - prefix - (alignment - 1))
        throw std::overflow_error("direct read window size overflow");
    const auto required = bytes + prefix;
    const auto count    = (required + alignment - 1) / alignment * alignment;
    windows::validate_offset(offset - prefix);
    return {offset - prefix, count, prefix, required};
}

} // namespace ninfer::io
