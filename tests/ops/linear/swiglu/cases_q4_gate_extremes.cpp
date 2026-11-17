// Q4 LinearSwiGLU cases for the 27B MLP geometry [2*17408, 5120] that the dense-sparse profile in
// cases_q4.cpp does not reach: gate pre-activations far outside its range (about |g| < 4), in both
// nibbles and in the first and last quantization group, and the public rejection of formats that
// r9700 does not register. Expected values come from the independent FP64 oracle in runner.cpp
// (run_negative_gate: logical_weight_fp64 decode, sign-split FP64 SiLU, unrounded product).
//
// Wiring (author): declare `void add_q4_gate_extreme_cases(Suite& suite);` in runner.h next to
// add_q4_cases, call it from test.cpp after add_q4_cases, and append cases_q4_gate_extremes.cpp to
// the ninfer_linear_suite(swiglu ...) source list in tests/ops/linear/tests.cmake.

#include "core/arena.h"
#include "core/buffer.h"
#include "core/platform.h"
#include "cuda_fixture.h"
#include "ninfer/ops/linear_swiglu.h"
#include "ops/linear/swiglu/runner.h"
#include "ops/linear/weights.h"
#include "ops/op_tester.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ninfer::test::linear_swiglu {
void add_q4_gate_extreme_cases(Suite& suite);
} // namespace ninfer::test::linear_swiglu

namespace {
using namespace ninfer;
using namespace ninfer::test;
namespace qw = ninfer::test::quantized_weight;

constexpr std::int32_t kGateUpRows = 34816;
constexpr std::int32_t kInputRows  = 5120;
constexpr std::int32_t kOutputRows = kGateUpRows / 2;

struct GateRecipe {
    std::uint32_t seed;
    std::int32_t column; // the single unit activation column; also the only non-zero weight column
    int gate_code;       // Q4 codes are signed 4-bit: [-8, 7]
    float gate_scale;    // exact in FP16
    int up_code;
    float up_scale;
};

// gate = gate_code * gate_scale and up = up_code * up_scale for every output row. -20 and -60 need
// a SiLU that does not lose the negative tail (an FP32 0.5*tanh(x/2)+0.5 sigmoid returns 0 there);
// +100 overflows exp(x) in FP32, so x*e^x/(1+e^x) would produce NaN. Column 5119 places the code in
// the high nibble (odd lane) of the last group of each row.
constexpr std::array kRecipes{
    GateRecipe{2901U, 0, -5, 4.0F, 5, 4.0F},       // gate -20, up 20
    GateRecipe{2903U, 5119, -6, 10.0F, 5, 4.0F},   // gate -60, up 20
    GateRecipe{2905U, 0, 5, 20.0F, -5, 4.0F},      // gate 100, up -20
    GateRecipe{2907U, 5119, 7, 0.5F, -8, 0.125F},  // gate 3.5, up -1 (code extremes 7 and -8)
};

qw::PackedWeight gate_weight(const GateRecipe& recipe) {
    auto packed = qw::make_patterned_weight(QType::Q4_G64_FP16, kGateUpRows, kInputRows,
                                            recipe.seed);
    std::fill(packed.payload.begin(), packed.payload.end(), 0);
    const auto spec            = qw::detail::quant_spec(QType::Q4_G64_FP16);
    const int nibble_bytes     = qw::detail::nibble_bytes_per_group(spec);
    const std::int32_t groups  = packed.weight.padded_shape[1] / spec.group_size;
    const std::int32_t group   = recipe.column / spec.group_size;
    const std::int32_t lane    = recipe.column % spec.group_size;
    const auto set             = [&](std::int32_t row, int code, float scale) {
        std::array<std::int8_t, 64> codes{};
        codes[lane]              = static_cast<std::int8_t>(code);
        const std::size_t index = static_cast<std::size_t>(row) * groups + group;
        qw::detail::pack_lowbit_group(codes.data(), spec,
                                      packed.payload.data() + index * nibble_bytes, nullptr);
        qw::detail::store_u16_le(packed.payload, packed.scale_plane_offset + index * 2,
                                 qw::detail::f32_to_f16(scale));
    };
    for (std::int32_t row = 0; row < kOutputRows; ++row) {
        set(row, recipe.gate_code, recipe.gate_scale);
        set(kOutputRows + row, recipe.up_code, recipe.up_scale);
    }
    return packed;
}

// A rejected LinearSwiGLU throws std::invalid_argument from the workspace query for every policy
// and from both execution forms, and leaves the caller's output untouched.
int verify_rejection(QType qtype, bool execute) {
    const std::string label = std::string(linear_suite::format_name(qtype)) +
                              " LinearSwiGLU [34816,5120] rejected on this target";
    int failures = 0;
    for (const auto policy : {ops::LinearPolicy::A16Only, ops::LinearPolicy::AllowA8,
                              ops::LinearPolicy::AllowA4}) {
        try {
            (void)ops::linear_swiglu_workspace_capacity_bytes(qtype, kGateUpRows, kInputRows,
                                                              policy, 1, 1);
            std::cerr << label << ": workspace query accepted the format\n";
            ++failures;
        } catch (const std::invalid_argument&) {
        } catch (const std::exception& error) {
            std::cerr << label << ": workspace query threw " << error.what() << '\n';
            ++failures;
        }
    }
    if (!execute) return failures;

    auto& cuda           = cuda_environment();
    const auto host      = linear_weights::make_q8_g32_fp16_weight(kGateUpRows, kInputRows, 2911U);
    GuardedDeviceBuffer weight_storage(host.payload.size());
    weight_storage.copy_from_host(host.payload.data(), host.payload.size());
    const Weight weight = host.device_weight(weight_storage.span());
    constexpr int kTokens = 3;
    GuardedDeviceBuffer input(std::size_t(kInputRows) * kTokens * 2);
    input.fill(0);
    GuardedDeviceBuffer output(std::size_t(kOutputRows) * kTokens * 2);
    GuardedDeviceBuffer scratch(256);
    MemoryArena workspace(scratch.span());
    const Tensor x(input.span(), DType::BF16, {kInputRows, kTokens});
    Tensor y(output.span(), DType::BF16, {kOutputRows, kTokens});
    const auto attempt = [&](const char* form, auto&& call) {
        output.fill(0xff);
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
        std::vector<std::uint8_t> after(output.bytes());
        output.copy_to_host(after.data(), after.size());
        if (!std::all_of(after.begin(), after.end(), [](auto byte) { return byte == 0xff; })) {
            std::cerr << label << ": " << form << " wrote output before rejecting\n";
            ++failures;
        }
    };
    attempt("policy form", [&] {
        ops::linear_swiglu(x, weight, y, ops::LinearPolicy::A16Only, workspace,
                           cuda.default_execution);
    });
    attempt("A16 form", [&] { ops::linear_swiglu(x, weight, y, workspace, cuda.default_execution); });
    failures += output.verify_guards(label);
    return failures;
}

} // namespace

void ninfer::test::linear_swiglu::add_q4_gate_extreme_cases(Suite& suite) {
    constexpr std::array<std::int32_t, 9> kTokens{1, 2, 3, 5, 8, 17, 33, 129, 1024};
    for (const auto& recipe : kRecipes)
        add_negative_gate(suite,
                          {QType::Q4_G64_FP16, kGateUpRows, kInputRows, kOutputRows, recipe.seed,
                           ops::LinearPolicy::A16Only},
                          kTokens, std::array{recipe.column}, [recipe] { return gate_weight(recipe); });
#if NINFER_TARGET_R9700
    // r9700 registers LinearSwiGLU only for Q4 [34816,5120]; the CUDA targets also register Q8 and
    // NVFP4 at this geometry (cases_q8.cpp, cases_nvfp4.cpp).
    suite.add(linear_suite::make_case("q4", kGateUpRows, kInputRows, 3, ops::LinearPolicy::A16Only,
                                      linear_suite::Execution::Eager, "r9700-rejects-q8"),
              [] { return verify_rejection(QType::Q8_G32_FP16, true); });
    suite.add(linear_suite::make_case("q4", kGateUpRows, kInputRows, 1, ops::LinearPolicy::A16Only,
                                      linear_suite::Execution::Eager, "r9700-rejects-nvfp4"),
              [] { return verify_rejection(QType::NVFP4, false); });
#endif
}
