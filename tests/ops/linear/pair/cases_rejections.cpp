// Public rejections of LinearPair: a Q8 geometry that no target registers, and on r9700 the BF16
// encoding, which the CUDA targets register (bf16 cases in test.cpp) but r9700 does not.
//
// Wiring (author): in test.cpp declare `void add_rejection_cases(ninfer::test::linear_suite::Suite&);`
// at namespace scope and call `add_rejection_cases(suite);` in main before `suite.main`; append
// cases_rejections.cpp to `ninfer_linear_suite(pair test.cpp ...)` in tests/ops/linear/tests.cmake.

#include "core/arena.h"
#include "core/buffer.h"
#include "core/platform.h"
#include "cuda_fixture.h"
#include "ninfer/ops/linear_pair.h"
#include "ops/linear/support.h"
#include "ops/linear/weights.h"
#include "ops/op_tester.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

void add_rejection_cases(ninfer::test::linear_suite::Suite& suite);

namespace {
using namespace ninfer;
using namespace ninfer::test;

bool query_rejects(const ops::LinearProfile& profile, std::string_view label) {
    try {
        (void)ops::linear_pair_workspace_capacity_bytes(profile, profile, 1, 1);
    } catch (const std::invalid_argument&) {
        return true;
    } catch (const std::exception& error) {
        std::cerr << label << ": workspace query threw " << error.what() << '\n';
        return false;
    }
    std::cerr << label << ": workspace query accepted the profile\n";
    return false;
}

int verify_unregistered_q8() {
    return query_rejects({QType::Q8_G32_FP16, 1024, 4096, ops::LinearPolicy::A16Only,
                          QuantLayout::RowSplit},
                         "q8 LinearPair [1024,4096]")
               ? 0
               : 1;
}

#if NINFER_TARGET_R9700
int verify_r9700_bf16_rejection() {
    constexpr std::int32_t n = 4096, k = 128, tokens = 3;
    const std::string label  = "bf16 LinearPair [4096,128] rejected on r9700";
    int failures             = query_rejects({QType::BF16, n, k, ops::LinearPolicy::A16Only,
                                              QuantLayout::Contiguous},
                                             label)
                                   ? 0
                                   : 1;
    auto& cuda      = cuda_environment();
    const auto host = linear_weights::make_bf16_weight(n, k, 2951U);
    GuardedDeviceBuffer weight_storage(host.payload.size());
    weight_storage.copy_from_host(host.payload.data(), host.payload.size());
    const Weight weight = host.device_weight(weight_storage.span());
    GuardedDeviceBuffer input(std::size_t(k) * tokens * 2);
    input.fill(0);
    GuardedDeviceBuffer first_output(std::size_t(n) * tokens * 2);
    GuardedDeviceBuffer second_output(std::size_t(n) * tokens * 2);
    first_output.fill(0xff);
    second_output.fill(0xff);
    const Tensor x(input.span(), DType::BF16, {k, tokens});
    Tensor first(first_output.span(), DType::BF16, {n, tokens});
    Tensor second(second_output.span(), DType::BF16, {n, tokens});
    try {
        ops::linear_pair(x, x, weight, weight, first, second, cuda.default_execution);
        std::cerr << label << ": execution accepted the format\n";
        ++failures;
    } catch (const std::invalid_argument&) {
    } catch (const std::exception& error) {
        std::cerr << label << ": execution threw " << error.what() << '\n';
        ++failures;
    }
    cuda_synchronize();
    for (auto* output : {&first_output, &second_output}) {
        std::vector<std::uint8_t> after(output->bytes());
        output->copy_to_host(after.data(), after.size());
        if (!std::all_of(after.begin(), after.end(), [](auto byte) { return byte == 0xff; })) {
            std::cerr << label << ": an output was written before rejecting\n";
            ++failures;
        }
        failures += output->verify_guards(label);
    }
    return failures;
}
#endif

} // namespace

void add_rejection_cases(ninfer::test::linear_suite::Suite& suite) {
    namespace linear_suite = ninfer::test::linear_suite;
    using ninfer::ops::LinearPolicy;
    suite.add(linear_suite::make_case("q8", 1024, 4096, 1, LinearPolicy::A16Only,
                                      linear_suite::Execution::Eager, "rejected-geometry"),
              verify_unregistered_q8);
#if NINFER_TARGET_R9700
    suite.add(linear_suite::make_case("q8", 4096, 128, 3, LinearPolicy::A16Only,
                                      linear_suite::Execution::Eager, "r9700-rejects-bf16"),
              verify_r9700_bf16_rejection);
#endif
}
