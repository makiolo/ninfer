// Q4+Q5 LinearSplit cases for the two 27B segmentations (attention: Q4 [7168] + Q5 [7168] into
// 6144/1024/6144/1024; GDN: Q4 [4096] + Q5 [12288] into 10240/6144, K=5120) that the existing
// profiles in cases_two_output.cpp and cases_multi_output.cpp leave out: every row (both banks and
// the bank boundary inside an output) at decode and a short odd T, GDN prefill extents above 193,
// Hashed/Small weights, workspace intervals, and the public rejections. Expected values come from
// the independent FP64 oracle of runner.cpp (logical_weight_fp64 of the parent row, FP64 dot).
//
// Wiring (author): declare `void add_q4_q5_rowsplit_cases(Suite& suite);` in runner.h next to
// add_multi_output_cases, call it from test.cpp after add_multi_output_cases, and append
// cases_q4_q5_rowsplit.cpp to the ninfer_linear_suite(split ...) source list in
// tests/ops/linear/tests.cmake.

#include "core/arena.h"
#include "core/buffer.h"
#include "core/platform.h"
#include "cuda_fixture.h"
#include "ninfer/ops/linear_split.h"
#include "ops/linear/split/runner.h"
#include "ops/linear/weights.h"
#include "ops/op_tester.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ninfer::test::linear_split {
void add_q4_q5_rowsplit_cases(Suite& suite);
} // namespace ninfer::test::linear_split

namespace {
using namespace ninfer;
using namespace ninfer::test;
using namespace ninfer::test::linear_split;
namespace qw = ninfer::test::quantized_weight;
using Policy = ops::LinearPolicy;

// The A16 policy criterion of every Linear-family Op (op-development.md, numerical validation).
constexpr ReductionCriterion kA16{1.0 / 256.0, 1.0 / 256.0, 2.0 / 256.0};
constexpr qw::PatternedWeightOptions kHashed{.row_split_scale = qw::RowSplitScalePattern::Small,
                                             .row_split_codes = qw::RowSplitCodePattern::Hashed};

void add_numeric_cases(Suite& suite) {
    {
        // GDN: Direct paired binding, as parameters.cpp:107-113 builds it from four inputs.
        Profile profile{5120,
                        {{QType::Q4_G64_FP16, 4096, 2913U, kHashed},
                         {QType::Q5_G64_FP16, 12288, 2917U, kHashed}},
                        {10240, 6144},
                        {{0, 4096}, {4096, 6144}, {10240, 6144}},
                        kA16};
        std::vector<Invocation> calls;
        for (int t : {1, 5})
            calls.push_back({.t = t, .input_seed = 2919U, .full_rows = true});
        for (int t : {255, 256, 257, 511, 512, 513, 1023, 1024, 1025})
            calls.push_back({.t = t, .input_seed = 2919U, .sample_rows = 15});
        calls.push_back({.t            = 1024,
                         .graph        = true,
                         .input_seed   = 2919U,
                         .changed_seed = 2921U,
                         .columns      = Columns::Seven});
        calls.push_back({.t = 0, .workspace_intervals = true});
        add_profile(suite, profile, std::move(calls));
    }
    {
        // Attention: four ProjectionInputs over two parents, as parameters.cpp:94-99 builds it.
        Profile profile{5120,
                        {{QType::Q4_G64_FP16, 7168, 2923U, kHashed},
                         {QType::Q5_G64_FP16, 7168, 2927U, kHashed}},
                        {6144, 1024, 6144, 1024},
                        {{0, 6144}, {6144, 1024}, {7168, 6144}, {13312, 1024}},
                        kA16,
                        1.0 / 256.0,
                        Binding::ProjectionInputs};
        std::vector<Invocation> calls;
        for (int t : {1, 3})
            calls.push_back({.t = t, .input_seed = 2929U, .full_rows = true});
        calls.push_back({.t = 0, .workspace_intervals = true});
        add_profile(suite, profile, std::move(calls));
    }
}

// A rejected LinearSplit throws std::invalid_argument from the workspace query and, when executed,
// from the call, leaving every output untouched.
bool query_rejects(const ops::ProjectionProfile& profile, std::span<const std::int32_t> rows,
                   std::string_view label) {
    try {
        (void)ops::linear_split_workspace_capacity_bytes(profile, rows, 1, 1);
    } catch (const std::invalid_argument&) {
        return true;
    } catch (const std::exception& error) {
        std::cerr << label << ": workspace query threw " << error.what() << '\n';
        return false;
    }
    std::cerr << label << ": workspace query accepted the profile\n";
    return false;
}

int verify_query_rejections() {
    using P = ops::LinearProfile;
    constexpr auto Q4 = QType::Q4_G64_FP16, Q5 = QType::Q5_G64_FP16;
    constexpr auto R  = QuantLayout::RowSplit;
    constexpr std::array<std::int32_t, 4> attention{6144, 1024, 6144, 1024};
    constexpr std::array<std::int32_t, 2> gdn{10240, 6144};
    constexpr std::array<std::int32_t, 2> halves{7168, 7168};
    int failures = 0;
    // Registered paired weights require A16 on every target (dispatch contract).
    failures += !query_rejects({P{Q4, 7168, 5120, Policy::AllowA8, R},
                                P{Q5, 7168, 5120, Policy::AllowA8, R}},
                               attention, "q4_q5 AllowA8");
    // Segmentation that no target registers for this parent pair.
    failures += !query_rejects({P{Q4, 7168, 5120, Policy::A16Only, R},
                                P{Q5, 7168, 5120, Policy::A16Only, R}},
                               halves, "q4_q5 7168/7168 segmentation");
    // K that is neither the registered 5120 nor a multiple of the 64-lane group.
    failures += !query_rejects({P{Q4, 4096, 5100, Policy::A16Only, R},
                                P{Q5, 12288, 5100, Policy::A16Only, R}},
                               gdn, "q4_q5 K=5100");
    // Swapped bank order: Q5 first is not a registered pair.
    failures += !query_rejects({P{Q5, 4096, 5120, Policy::A16Only, R},
                                P{Q4, 12288, 5120, Policy::A16Only, R}},
                               gdn, "q5_q4 order");
    return failures;
}

#if NINFER_TARGET_R9700
// r9700 registers LinearSplit only for the Q4+Q5 pairs above; the CUDA targets also register BF16
// [14336,5120] into 6144/1024/6144/1024 (cases_multi_output.cpp).
int verify_r9700_bf16_rejection() {
    constexpr std::array<std::int32_t, 4> rows{6144, 1024, 6144, 1024};
    const std::string label = "bf16 LinearSplit [14336,5120] rejected on r9700";
    int failures            = query_rejects(
                                  {ops::LinearProfile{QType::BF16, 14336, 5120, Policy::A16Only,
                                                      QuantLayout::Contiguous},
                                   {}},
                                  rows, label)
                                  ? 0
                                  : 1;
    auto& cuda      = cuda_environment();
    const auto host = linear_weights::make_bf16_weight(14336, 5120, 2931U);
    GuardedDeviceBuffer weight_storage(host.payload.size());
    weight_storage.copy_from_host(host.payload.data(), host.payload.size());
    const Weight weight   = host.device_weight(weight_storage.span());
    constexpr int kTokens = 3;
    GuardedDeviceBuffer input(std::size_t(5120) * kTokens * 2);
    input.fill(0);
    std::vector<std::unique_ptr<GuardedDeviceBuffer>> storage;
    std::vector<Tensor> outputs;
    for (int count : rows) {
        storage.push_back(std::make_unique<GuardedDeviceBuffer>(std::size_t(count) * kTokens * 2));
        outputs.emplace_back(storage.back()->span(), DType::BF16,
                             std::initializer_list<std::int32_t>{count, kTokens});
    }
    GuardedDeviceBuffer scratch(256);
    MemoryArena workspace(scratch.span());
    const Tensor x(input.span(), DType::BF16, {5120, kTokens});
    const auto attempt = [&](const char* form, auto&& call) {
        for (auto& buffer : storage) buffer->fill(0xff);
        try {
            call();
            std::cerr << label << ": " << form << " accepted the format\n";
            ++failures;
        } catch (const std::invalid_argument&) {
        } catch (const std::exception& error) {
            std::cerr << label << ": " << form << " threw " << error.what() << '\n';
            ++failures;
        }
        cuda_synchronize();
        for (const auto& buffer : storage) {
            std::vector<std::uint8_t> after(buffer->bytes());
            buffer->copy_to_host(after.data(), after.size());
            if (!std::all_of(after.begin(), after.end(), [](auto byte) { return byte == 0xff; })) {
                std::cerr << label << ": " << form << " wrote an output before rejecting\n";
                ++failures;
            }
            failures += buffer->verify_guards(label);
        }
    };
    attempt("A16 single-parent form",
            [&] { ops::linear_split(x, weight, outputs, cuda.default_execution); });
    attempt("prepared form", [&] {
        ops::linear_split(x, ops::SingleProjectionWeight{weight, Policy::A16Only}, outputs,
                          workspace, cuda.default_execution);
    });
    return failures;
}
#endif

} // namespace

void ninfer::test::linear_split::add_q4_q5_rowsplit_cases(Suite& suite) {
    add_numeric_cases(suite);
    suite.add(linear_suite::make_case("q4_q5", 16384, 5120, 1, Policy::A16Only,
                                      linear_suite::Execution::Eager, "rejected-profiles"),
              verify_query_rejections);
#if NINFER_TARGET_R9700
    suite.add(linear_suite::make_case("q4_q5", 14336, 5120, 3, Policy::A16Only,
                                      linear_suite::Execution::Eager, "r9700-rejects-bf16"),
              verify_r9700_bf16_rejection);
#endif
}
