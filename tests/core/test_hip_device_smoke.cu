#include "core/buffer.h"
#include "core/device.h"
#include "core/transfer.h"

#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

// Integer arithmetic gives an exact host reference, including the partial last block.
__global__ void transform(std::uint32_t* values, unsigned int count) {
    const unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < count) { values[i] = values[i] * 3u + i; }
}

void smoke() {
    ninfer::DeviceContext context(0);
    context.bind_to_current_thread();
    ninfer::CudaDeviceScope device_scope(context.device());
    int selected = -1;
    CUDA_CHECK(cudaGetDevice(&selected));
    require(selected == 0 && context.device() == ninfer::Device::cuda(0),
            "context did not select device 0");
    require(std::string_view(context.properties().gcnArchName).starts_with("gfx1201"),
            "device 0 is not gfx1201");
    std::printf("device=0 name=%s arch=%s pci_bus=%d\n", context.properties().name,
                context.properties().gcnArchName, context.properties().pciBusID);

    constexpr unsigned int count = 1000;
    constexpr unsigned int block = 256;
    constexpr auto bytes = count * sizeof(std::uint32_t);
    ninfer::PinnedHostBuffer upload(bytes);
    ninfer::PinnedHostBuffer download(bytes);
    auto* input = static_cast<std::uint32_t*>(upload.data());
    auto* output = static_cast<std::uint32_t*>(download.data());
    for (unsigned int i = 0; i < count; ++i) {
        input[i] = 7u * i + 11u;
        output[i] = 0xffffffffu;
    }

    // Keep backing alive until the event/stream retire, including on exceptions.
    ninfer::DeviceBuffer buffer(context.device(), bytes);
    require(buffer.data() != nullptr && buffer.size() == bytes &&
                buffer.device() == context.device(), "invalid DeviceBuffer allocation");
    {
        ninfer::CudaStream stream(context);
        const auto execution = context.execution_view(stream.stream());
        require(execution.stream() != nullptr && execution.device() == context.device(),
                "invalid borrowed execution view");
        ninfer::CudaCompletionEvent completion(context);
        try {
            ninfer::copy_bytes_async(buffer.span(), upload.span(), execution);
            ninfer::copy_bytes_async(download.span(), buffer.span(), execution);
            completion.record(execution.stream());
            completion.synchronize();
            require(completion.ready(), "roundtrip event is not ready after synchronization");
            for (unsigned int i = 0; i < count; ++i) {
                require(output[i] == input[i], "H2D/D2H roundtrip differs from input");
            }

            transform<<<(count + block - 1) / block, block, 0, execution.stream()>>>(
                static_cast<std::uint32_t*>(buffer.data()), count);
            CUDA_CHECK(cudaGetLastError());
            ninfer::copy_bytes_async(download.span(), buffer.span(), execution);
            completion.record(execution.stream());
            completion.synchronize();
            CUDA_CHECK(cudaStreamSynchronize(execution.stream()));
            require(completion.ready(), "kernel event is not ready after synchronization");
            for (unsigned int i = 0; i < count; ++i) {
                // Independent closed form of 3 * (7*i + 11) + i.
                if (output[i] != 22u * i + 33u) {
                    throw std::runtime_error("kernel mismatch at index " + std::to_string(i));
                }
            }
        } catch (...) {
            // Finish every borrowed use before event, stream or backing destruction.
            CUDA_CHECK(cudaStreamSynchronize(execution.stream()));
            throw;
        }
    }
    // Exercise core's release path before reporting success.
    buffer = ninfer::DeviceBuffer{};
    require(buffer.data() == nullptr && buffer.size() == 0, "DeviceBuffer release did not reset owner");
    CUDA_CHECK(cudaGetLastError());
}

} // namespace

int main(int argc, char** argv) {
    bool require_device = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--require-device") {
            require_device = true;
        } else {
            std::fprintf(stderr, "unknown argument: %s\n", argv[i]);
            return 1;
        }
    }
    int devices = 0;
    const auto status = cudaGetDeviceCount(&devices);
    if (status == hipErrorNoDevice || (status == cudaSuccess && devices == 0)) {
        std::fprintf(stderr, "no visible device%s\n", require_device ? " (--require-device)" : " (SKIP)");
        return require_device ? 1 : 77;
    }
    // Driver/runtime errors must not turn into a successful skip.
    CUDA_CHECK(status);
    try {
        smoke();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "device smoke failed: %s\n", error.what());
        return 1;
    }
    std::puts("PASS: exact roundtrip and kernel results for 1000 elements (block 256); core resources released");
    return 0;
}
