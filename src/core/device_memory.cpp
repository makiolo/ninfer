#include "core/device.h"
#include "core/platform.h"

#include <algorithm>
#include <fstream>
#include <string>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <system_error>
#if NINFER_TARGET_R9700
#include <d3dkmthk.h>
#include <bit>
#include <cstdio>
#include <cstring>
#endif
#endif

namespace ninfer {
namespace {
void read_system_memory(DeviceMemorySnapshot& snapshot) {
#if defined(_WIN32)
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    if (!GlobalMemoryStatusEx(&status)) {
        const auto error = GetLastError();
        throw std::system_error(static_cast<int>(error), std::system_category(),
                                "cannot read system memory availability");
    }
    snapshot.system_available_bytes = status.ullAvailPhys;
    snapshot.system_total_bytes = status.ullTotalPhys;
#else
    std::ifstream input("/proc/meminfo");
    std::string key, rest;
    std::uint64_t kib;
    while (input >> key >> kib) {
        std::getline(input, rest);
        if (key == "MemAvailable:") snapshot.system_available_bytes = kib * 1024;
        if (key == "MemTotal:") snapshot.system_total_bytes = kib * 1024;
        if (snapshot.system_available_bytes && snapshot.system_total_bytes) return;
    }
    throw std::runtime_error("cannot read system memory availability");
#endif
}

#if defined(_WIN32) && NINFER_TARGET_R9700
void check_wddm(NTSTATUS status, const char* operation) {
    if (status < 0)
        throw std::runtime_error(std::string(operation) + " failed (NTSTATUS " +
                                 std::to_string(static_cast<std::uint32_t>(status)) + ")");
}

void read_local_memory_budget(Device device, DeviceMemorySnapshot& snapshot) {
    cudaDeviceProp properties{};
    CUDA_CHECK(cudaGetDeviceProperties(&properties, device.index()));
    // HIP supplies the Windows adapter identity, independent of visible-device ordinals.
    const auto mask = properties.luidDeviceNodeMask;
    if (!std::has_single_bit(mask))
        throw std::runtime_error("WDDM memory query requires one physical adapter node");
    D3DKMT_OPENADAPTERFROMLUID adapter{};
    static_assert(sizeof(adapter.AdapterLuid) == sizeof(properties.luid));
    std::memcpy(&adapter.AdapterLuid, properties.luid, sizeof(adapter.AdapterLuid));
    check_wddm(D3DKMTOpenAdapterFromLuid(&adapter), "D3DKMTOpenAdapterFromLuid");
    struct AdapterLease {
        D3DKMT_HANDLE handle;
        ~AdapterLease() {
            D3DKMT_CLOSEADAPTER close{handle};
            const auto status = D3DKMTCloseAdapter(&close);
            if (status < 0)
                std::fprintf(stderr, "D3DKMTCloseAdapter failed (NTSTATUS %u)\n",
                             static_cast<unsigned>(status));
        }
    } lease{adapter.hAdapter};
    D3DKMT_QUERYVIDEOMEMORYINFO info{};
    info.hAdapter = adapter.hAdapter;
    info.MemorySegmentGroup = D3DKMT_MEMORY_SEGMENT_GROUP_LOCAL;
    info.PhysicalAdapterIndex = std::countr_zero(mask);
    check_wddm(D3DKMTQueryVideoMemoryInfo(&info), "D3DKMTQueryVideoMemoryInfo(local)");
    snapshot.local_budget_bytes = info.Budget;
    snapshot.local_usage_bytes = info.CurrentUsage;
}
#endif
} // namespace

DeviceMemorySnapshot device_memory_snapshot(Device device) {
    CudaDeviceScope scope(device);
    DeviceMemorySnapshot snapshot;
    CUDA_CHECK(cudaMemGetInfo(&snapshot.cuda_free_bytes, &snapshot.total_bytes));
    read_system_memory(snapshot);
#if defined(_WIN32) && NINFER_TARGET_R9700
    // hipMemGetInfo on Windows only deducts allocations by this process. WDDM's
    // dynamic residency budget accounts for OS/other-process reservations. Use
    // min(HIP free, budget - usage), never deduct the same reservation twice.
    read_local_memory_budget(device, snapshot);
#endif
    snapshot.shared_physical_memory = NINFER_SHARED_PHYSICAL_MEMORY != 0;
    return snapshot;
}

std::int64_t device_memory_growth(DeviceMemorySnapshot before, DeviceMemorySnapshot after) {
    return static_cast<std::int64_t>(before.available_bytes()) -
           static_cast<std::int64_t>(after.available_bytes());
}

} // namespace ninfer
