// Q5 LinearAdd cases for the 27B geometries [5120, 6144] (attention and GDN output) and
// [5120, 17408] (MLP down) that cases_q5.cpp leaves out: signed (Hashed) activations with Hashed
// residuals, where the projection and the residual cancel, every row at decode and short odd T,
// the A16 zero-workspace contract, and the public rejections. Expected values come from the
// independent FP64 oracle of runner.cpp (FP32-exact Q5 decode, FP64 dot plus the BF16 residual,
// no intermediate projection rounding).
//
// Wiring (author): declare `void add_q5_signed_cases(Suite& suite);` in runner.h next to
// add_q5_cases, call it from test.cpp after add_q5_cases, and append cases_q5_signed.cpp to the
// ninfer_linear_suite(add ...) source list in tests/ops/linear/tests.cmake.

#include "core/arena.h"
#include "core/buffer.h"
#include "core/platform.h"
#include "cuda_fixture.h"
#include "ninfer/ops/linear_add.h"
#include "ops/linear/add/runner.h"
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
#include <utility>
#include <vector>

namespace ninfer::test::linear_add {
void add_q5_signed_cases(Suite& suite);
} // namespace ninfer::test::linear_add

namespace {
using namespace ninfer;
using namespace ninfer::test;
using namespace ninfer::test::linear_add;
using Policy = ops::LinearPolicy;

constexpr std::array<std::int32_t, 2> kInputRows{6144, 17408};

constexpr std::array kFullRows{
    Invocation{.tokens = 1, .policy = Policy::A16Only},
    Invocation{.tokens = 3},
    Invocation{.tokens = 5, .policy = Policy::A16Only},
};
constexpr std::array kSampledRows{
    Invocation{.tokens = 2, .policy = Policy::A16Only},
    Invocation{.tokens = 31, .policy = Policy::A16Only},
    Invocation{.tokens = 33, .policy = Policy::A16Only, .graph = GraphReplay::OriginalNegative},
    Invocation{.tokens = 255},
    Invocation{.tokens = 1023, .policy = Policy::A16Only},
};

int verify_zero_workspace(std::int32_t k) {
    int failures = 0;
    for (const auto [first, last] : {std::pair{1, 1}, std::pair{1, 4096}, std::pair{2047, 2049}}) {
        if (ops::linear_add_workspace_capacity_bytes(QType::Q5_G64_FP16, 5120, k, Policy::A16Only,
                                                     first, last) != 0 ||
            ops::linear_add_workspace_capacity_bytes(QType::Q5_G64_FP16, 5120, k, first, last) !=
                0) {
            std::cerr << "q5 LinearAdd [5120," << k << "]: A16 must require no scratch\n";
            ++failures;
        }
    }
    for (const auto [first, last] : {std::pair{0, 1}, std::pair{-1, 8}, std::pair{8, 7}}) {
        try {
            (void)ops::linear_add_workspace_capacity_bytes(QType::Q5_G64_FP16, 5120, k,
                                                           Policy::A16Only, first, last);
            std::cerr << "q5 LinearAdd: accepted the invalid interval [" << first << ',' << last
                      << "]\n";
            ++failures;
        } catch (const std::invalid_argument&) {}
    }
    return failures;
}

bool query_rejects(QType qtype, std::int32_t n, std::int32_t k, std::string_view label) {
    try {
        (void)ops::linear_add_workspace_capacity_bytes(qtype, n, k, Policy::A16Only, 1, 1);
    } catch (const std::invalid_argument&) {
        return true;
    } catch (const std::exception& error) {
        std::cerr << label << ": workspace query threw " << error.what() << '\n';
        return false;
    }
    std::cerr << label << ": workspace query accepted the profile\n";
    return false;
}

// Geometries that no target registers for Q5 LinearAdd.
int verify_query_rejections() {
    int failures = 0;
    failures += !query_rejects(QType::Q5_G64_FP16, 5120, 6100, "q5 [5120,6100] K off-group");
    failures += !query_rejects(QType::Q5_G64_FP16, 4096, 6144, "q5 [4096,6144]");
    return failures;
}

#if NINFER_TARGET_R9700
// r9700 registers LinearAdd only for Q5; the CUDA targets also register Q4 and Q8 at [5120,6144]
// (cases_q4.cpp, cases_q8.cpp). The residual must be left untouched by both public forms.
int verify_r9700_rejection(QType qtype) {
    constexpr std::int32_t n = 5120, k = 6144, tokens = 3;
    const std::string label  = std::string(linear_suite::format_name(qtype)) +
                              " LinearAdd [5120,6144] rejected on r9700";
    int failures   = query_rejects(qtype, n, k, label) ? 0 : 1;
    auto& cuda     = cuda_environment();
    const auto host = qtype == QType::Q4_G64_FP16
                          ? linear_weights::make_q4_g64_fp16_weight(n, k, 2941U)
                          : linear_weights::make_q8_g32_fp16_weight(n, k, 2941U);
    GuardedDeviceBuffer weight_storage(host.payload.size());
    weight_storage.copy_from_host(host.payload.data(), host.payload.size());
    const Weight weight = host.device_weight(weight_storage.span());
    GuardedDeviceBuffer input(std::size_t(k) * tokens * 2);
    input.fill(0);
    GuardedDeviceBuffer residual(std::size_t(n) * tokens * 2);
    GuardedDeviceBuffer scratch(256);
    MemoryArena workspace(scratch.span());
    const Tensor x(input.span(), DType::BF16, {k, tokens});
    Tensor y(residual.span(), DType::BF16, {n, tokens});
    const auto attempt = [&](const char* form, auto&& call) {
        residual.fill(0x3c); // 0x3c3c is a finite BF16 residual
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
        std::vector<std::uint8_t> after(residual.bytes());
        residual.copy_to_host(after.data(), after.size());
        if (!std::all_of(after.begin(), after.end(), [](auto byte) { return byte == 0x3c; })) {
            std::cerr << label << ": " << form << " modified the residual before rejecting\n";
            ++failures;
        }
    };
    attempt("policy form", [&] {
        ops::linear_add(x, weight, y, Policy::A16Only, workspace, cuda.default_execution);
    });
    attempt("A16 form", [&] { ops::linear_add(x, weight, y, workspace, cuda.default_execution); });
    failures += residual.verify_guards(label);
    return failures;
}
#endif

} // namespace

void ninfer::test::linear_add::add_q5_signed_cases(Suite& suite) {
    for (const auto k : kInputRows) {
        add_cases(suite, QType::Q5_G64_FP16,
                  InvocationCase{.n           = 5120,
                                 .k           = k,
                                 .seed        = 2933U,
                                 .invocations = kFullRows,
                                 .fixture     = FixturePattern::Hashed,
                                 .rows        = Samples::All});
        add_cases(suite, QType::Q5_G64_FP16,
                  InvocationCase{.n           = 5120,
                                 .k           = k,
                                 .seed        = 2937U,
                                 .invocations = kSampledRows,
                                 .fixture     = FixturePattern::Hashed,
                                 .rows        = Samples::Uniform32});
        suite.add(linear_suite::make_case("q5", 5120, k, 0, Policy::A16Only,
                                          linear_suite::Execution::Eager, "zero-workspace"),
                  [k] { return verify_zero_workspace(k); });
    }
    suite.add(linear_suite::make_case("q5", 5120, 6144, 1, Policy::A16Only,
                                      linear_suite::Execution::Eager, "rejected-geometries"),
              verify_query_rejections);
#if NINFER_TARGET_R9700
    suite.add(linear_suite::make_case("q5", 5120, 6144, 3, Policy::A16Only,
                                      linear_suite::Execution::Eager, "r9700-rejects-q4"),
              [] { return verify_r9700_rejection(QType::Q4_G64_FP16); });
    suite.add(linear_suite::make_case("q5", 5120, 6144, 3, Policy::A16Only,
                                      linear_suite::Execution::Eager, "r9700-rejects-q8"),
              [] { return verify_r9700_rejection(QType::Q8_G32_FP16); });
#endif
}
