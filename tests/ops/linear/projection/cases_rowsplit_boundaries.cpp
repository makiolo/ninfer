// Boundary and rejection cases for the row-split Q4/Q5/Q8 Linear projection.
//
// Every numeric case runs the public Linear entry against the independent FP64 oracle in
// runner.cpp (row-split weights decoded from their public representation by
// tests/ops/quantized_weight.h). The cases are target-neutral: they exercise registered geometries
// on every target, so the same expectations hold whichever private mechanism a target selects.
// The final group checks the public rejection of combinations that a target does not register.

#include "core/arena.h"
#include "core/buffer.h"
#include "core/platform.h"
#include "cuda_fixture.h"
#include "ninfer/ops/linear.h"
#include "ops/linear/projection/runner.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Wiring (author): declare in runner.h next to add_q8_cases and call from test.cpp.
namespace ninfer::test::linear {
void add_rowsplit_boundary_cases(Suite& suite);
} // namespace ninfer::test::linear

namespace {
using namespace ninfer;
using namespace ninfer::test::linear;
using namespace ninfer::test::linear_weights;
namespace linear_suite = ninfer::test::linear_suite;

constexpr Invocation a16(std::int32_t t) { return {t}; }

constexpr Invocation graph(std::int32_t t) {
    return {t, CallForm::Policy, ops::LinearPolicy::A16Only, true};
}

constexpr Invocation convenience(std::int32_t t) { return {t, CallForm::A16Convenience}; }

constexpr Invocation allow_a8(std::int32_t t) {
    return {t, CallForm::Policy, ops::LinearPolicy::AllowA8};
}

constexpr Invocation allow_a4(std::int32_t t) {
    return {t, CallForm::Policy, ops::LinearPolicy::AllowA4};
}

constexpr std::array kPatterns{ActivationPattern::KTail, ActivationPattern::Cancellation};

// Decode (T = 1), every small extent and both sides of each power of two up to 129, so a
// target's private decode/prefill switch is bracketed wherever it is placed.
constexpr std::array kTokenSweep{
    convenience(1), graph(1),  a16(2),    a16(3),   a16(4),   a16(5),   a16(6),   a16(7),
    a16(8),         a16(9),    a16(10),   a16(11),  a16(12),  a16(13),  a16(14),  a16(15),
    a16(16),        graph(17), a16(18),   a16(23),  a16(24),  a16(25),  a16(31),  a16(32),
    graph(33),      a16(47),   a16(48),   a16(49),  a16(63),  a16(64),  graph(65), a16(95),
    a16(96),        a16(97),   a16(127),  a16(128), graph(129), allow_a8(1), allow_a4(17)};

// Registered geometries whose public T domain is a multiple of 4 in [4,131072].
constexpr std::array kQuadTokenSweep{
    graph(4),  a16(8),   a16(12),  a16(16),  a16(20),  a16(28),   a16(32),   graph(36),
    a16(60),   a16(64),  a16(68),  a16(96),  a16(100), a16(128),  graph(132), a16(256),
    a16(260),  allow_a8(4), allow_a4(68)};

// Long reductions are compared on sampled rows; decode and the first prefill extents suffice.
constexpr std::array kLongKCalls{graph(1), a16(2), a16(3), a16(4), a16(8), a16(9), a16(17),
                                 a16(33), a16(65)};

void add_numeric_cases(Suite& suite) {
    // Q4: K-tail and cancellation activations are otherwise target-guarded for Q4.
    for (const auto pattern : kPatterns) {
        add_shape(suite, QType::Q4_G64_FP16, ActivationRecipe::Hashed, make_q4_g64_fp16_weight,
                  {1024, 5120, 4001U, Comparison::Full, true, kTokenSweep, pattern});
        // N = 4304 is not a multiple of 32 or 64; K = 1152 is not a multiple of 256.
        add_shape(suite, QType::Q4_G64_FP16, ActivationRecipe::Hashed, make_q4_g64_fp16_weight,
                  {4304, 1152, 4003U, Comparison::Full, true, kQuadTokenSweep, pattern});
    }
    add_shape(suite, QType::Q4_G64_FP16, ActivationRecipe::Hashed, make_q4_g64_fp16_weight,
              {1024, 5120, 4005U, Comparison::Full, true, kTokenSweep});

    // Q5: K = 4304 ends in a 16-lane partial group inside a 128-column padded row; the K-tail
    // pattern places every nonzero activation in exactly that partial group.
    for (const auto pattern : kPatterns) {
        add_shape(suite, QType::Q5_G64_FP16, ActivationRecipe::Hashed, make_q5_g64_fp16_weight,
                  {1152, 4304, 5001U, Comparison::Full, true, kQuadTokenSweep, pattern});
        add_shape(suite, QType::Q5_G64_FP16, ActivationRecipe::Hashed, make_q5_g64_fp16_weight,
                  {1152, 1152, 5003U, Comparison::Full, true, kQuadTokenSweep, pattern});
        add_shape(suite, QType::Q5_G64_FP16, ActivationRecipe::Hashed, make_q5_g64_fp16_weight,
                  {1024, 5120, 5005U, Comparison::Full, true, kTokenSweep, pattern});
        add_shape(suite, QType::Q5_G64_FP16, ActivationRecipe::Hashed, make_q5_g64_fp16_weight,
                  {5120, 17408, 5007U, Comparison::Sampled, true, kLongKCalls, pattern});
    }
    add_shape(suite, QType::Q5_G64_FP16, ActivationRecipe::Hashed, make_q5_g64_fp16_weight,
              {1152, 4304, 5009U, Comparison::Full, true, kQuadTokenSweep});
    add_shape(suite, QType::Q5_G64_FP16, ActivationRecipe::Hashed, make_q5_g64_fp16_weight,
              {1024, 5120, 5011U, Comparison::Full, true, kTokenSweep});

    // Q8: K = 4608 and K = 2560 are not multiples of 1024.
    for (const auto pattern : kPatterns) {
        add_shape(suite, QType::Q8_G32_FP16, ActivationRecipe::Hashed, make_q8_g32_fp16_weight,
                  {2048, 4608, 8001U, Comparison::Full, true, kTokenSweep, pattern});
        add_shape(suite, QType::Q8_G32_FP16, ActivationRecipe::Hashed, make_q8_g32_fp16_weight,
                  {1024, 5120, 8003U, Comparison::Full, true, kTokenSweep, pattern});
        add_shape(suite, QType::Q8_G32_FP16, ActivationRecipe::Hashed, make_q8_g32_fp16_weight,
                  {6144, 2560, 8005U, Comparison::Sampled, true, kLongKCalls, pattern});
    }
    add_shape(suite, QType::Q8_G32_FP16, ActivationRecipe::Hashed, make_q8_g32_fp16_weight,
              {1024, 2048, 8007U, Comparison::Full, true, kTokenSweep});
}

// Row-split Linear needs no transient storage across its whole public domain; intervals that are
// not positive and ordered are invalid on every target.
struct Domain {
    QType qtype;
    std::int32_t n;
    std::int32_t k;
    std::int32_t step;
    std::int32_t maximum;
};

constexpr std::array kDomains{
    Domain{QType::Q4_G64_FP16, 1024, 5120, 1, std::numeric_limits<std::int32_t>::max()},
    Domain{QType::Q4_G64_FP16, 5120, 6144, 1, std::numeric_limits<std::int32_t>::max()},
    Domain{QType::Q4_G64_FP16, 34816, 5120, 1, std::numeric_limits<std::int32_t>::max()},
    Domain{QType::Q4_G64_FP16, 3456, 1152, 4, 131072},
    Domain{QType::Q4_G64_FP16, 4304, 1152, 4, 131072},
    Domain{QType::Q5_G64_FP16, 1024, 5120, 1, std::numeric_limits<std::int32_t>::max()},
    Domain{QType::Q5_G64_FP16, 5120, 17408, 1, std::numeric_limits<std::int32_t>::max()},
    Domain{QType::Q5_G64_FP16, 1152, 1152, 4, 131072},
    Domain{QType::Q5_G64_FP16, 1152, 4304, 4, 131072},
};

constexpr std::array kPolicies{ops::LinearPolicy::A16Only, ops::LinearPolicy::AllowA8,
                               ops::LinearPolicy::AllowA4};

bool workspace_rejects(QType qtype, std::int32_t n, std::int32_t k, ops::LinearPolicy policy,
                       std::int32_t first, std::int32_t last, std::string_view label) {
    try {
        (void)ops::linear_workspace_capacity_bytes(qtype, n, k, policy, first, last);
        std::cerr << label << ": workspace query accepted [" << first << ',' << last << "]\n";
        return false;
    } catch (const std::invalid_argument&) {
        return true;
    } catch (const std::exception& error) {
        std::cerr << label << ": workspace query threw a non-argument error: " << error.what()
                  << '\n';
        return false;
    }
}

int verify_domain(const Domain& domain) {
    const std::string label = std::string(linear_suite::format_name(domain.qtype)) + " [" +
                              std::to_string(domain.n) + ',' + std::to_string(domain.k) + ']';
    int failures = 0;
    for (const auto policy : kPolicies) {
        try {
            if (ops::linear_workspace_capacity_bytes(domain.qtype, domain.n, domain.k, policy,
                                                     domain.step, domain.maximum) != 0) {
                std::cerr << label << ": expected zero workspace across the public T domain\n";
                ++failures;
            }
        } catch (const std::exception& error) {
            std::cerr << label << ": public T domain rejected: " << error.what() << '\n';
            ++failures;
        }
        std::vector<std::pair<std::int32_t, std::int32_t>> invalid{{0, domain.step},
                                                                   {-1, domain.step},
                                                                   {domain.step * 2, domain.step}};
        if (domain.step != 1) {
            invalid.insert(invalid.end(), {{1, 4}, {4, 5}, {3, 8}, {4, domain.maximum + 4}});
        }
        for (const auto [first, last] : invalid) {
            failures +=
                workspace_rejects(domain.qtype, domain.n, domain.k, policy, first, last, label)
                    ? 0
                    : 1;
        }
    }
    return failures;
}

// A rejected projection throws std::invalid_argument from every public form, from the workspace
// query for the same point, and leaves the caller's output untouched.
struct Rejection {
    QType qtype;
    std::int32_t n;
    std::int32_t k;
    std::int32_t t;
    WeightGenerator generator;
    bool execute; // false: query only, to avoid materializing very large fixtures
};

int verify_rejection(const Rejection& rejection) {
    const std::string label = std::string(linear_suite::format_name(rejection.qtype)) + " [" +
                              std::to_string(rejection.n) + ',' + std::to_string(rejection.k) +
                              "] T=" + std::to_string(rejection.t);
    int failures = 0;
    for (const auto policy : kPolicies) {
        failures += workspace_rejects(rejection.qtype, rejection.n, rejection.k, policy,
                                      rejection.t, rejection.t, label)
                        ? 0
                        : 1;
    }
    if (!rejection.execute) return failures;

    auto& cuda          = ninfer::test::cuda_environment();
    const Device device = cuda.context.device();
    const auto host     = rejection.generator(rejection.n, rejection.k, 911U);
    DeviceBuffer weight_storage(device, host.payload.size());
    weight_storage.copy_from_host(host.payload.data(), host.payload.size());
    const Weight weight = host.device_weight(weight_storage.span());

    const std::size_t input_bytes = std::size_t(rejection.k) * rejection.t * 2;
    const std::size_t output_bytes = std::size_t(rejection.n) * rejection.t * 2;
    DeviceBuffer input(device, input_bytes);
    input.fill(0);
    DeviceBuffer output(device, output_bytes);
    DeviceBuffer scratch_storage(device, 256);
    MemoryArena scratch(scratch_storage.span());
    Tensor x(input.span(), DType::BF16, {rejection.k, rejection.t});
    Tensor y(output.span(), DType::BF16, {rejection.n, rejection.t});

    const auto attempt = [&](const char* form, auto&& call) {
        output.fill(0xff);
        try {
            call();
            std::cerr << label << ": " << form << " accepted an unsupported projection\n";
            ++failures;
        } catch (const std::invalid_argument&) {
        } catch (const std::exception& error) {
            std::cerr << label << ": " << form << " threw a non-argument error: " << error.what()
                      << '\n';
            ++failures;
        }
        ninfer::test::cuda_check(cudaDeviceSynchronize(), "synchronize after rejection");
        std::vector<std::uint8_t> after(output_bytes);
        output.copy_to_host(after.data(), after.size());
        if (!std::all_of(after.begin(), after.end(), [](auto byte) { return byte == 0xff; })) {
            std::cerr << label << ": " << form << " wrote output before rejecting\n";
            ++failures;
        }
    };
    for (const auto policy : kPolicies) {
        attempt("policy form",
                [&] { ops::linear(x, weight, y, policy, scratch, cuda.default_execution); });
    }
    attempt("A16 convenience form", [&] { ops::linear(x, weight, y, cuda.default_execution); });
    return failures;
}

// Rejected on every target: extents outside a registered geometry's public T domain and
// geometries that no target registers.
constexpr std::array kRejections{
    Rejection{QType::Q4_G64_FP16, 3456, 1152, 1, make_q4_g64_fp16_weight, true},
    Rejection{QType::Q4_G64_FP16, 4304, 1152, 6, make_q4_g64_fp16_weight, true},
    Rejection{QType::Q4_G64_FP16, 4304, 1152, 131076, make_q4_g64_fp16_weight, false},
    Rejection{QType::Q5_G64_FP16, 1152, 4304, 3, make_q5_g64_fp16_weight, true},
    Rejection{QType::Q5_G64_FP16, 1152, 1152, 131076, make_q5_g64_fp16_weight, false},
    Rejection{QType::Q4_G64_FP16, 1000, 5120, 1, make_q4_g64_fp16_weight, true},
    Rejection{QType::Q5_G64_FP16, 1024, 5100, 1, make_q5_g64_fp16_weight, true},
    Rejection{QType::Q8_G32_FP16, 1000, 5120, 1, make_q8_g32_fp16_weight, true},
#if NINFER_TARGET_R9700
    // Geometries registered only by targets whose prefill mechanisms are MMA/TMA; r9700 does not
    // register them, at decode or prefill extents.
    Rejection{QType::Q4_G64_FP16, 77440, 4096, 1, make_q4_g64_fp16_weight, false},
    Rejection{QType::Q4_G64_FP16, 248320, 5120, 1, make_q4_g64_fp16_weight, false},
    Rejection{QType::Q8_G32_FP16, 4096, 16384, 1, make_q8_g32_fp16_weight, true},
    Rejection{QType::Q8_G32_FP16, 4096, 16384, 64, make_q8_g32_fp16_weight, false},
#endif
};

void add_contract_cases(Suite& suite) {
    for (const auto& domain : kDomains) {
        auto test = linear_suite::make_case(linear_suite::format_name(domain.qtype), domain.n,
                                            domain.k, 0, ops::LinearPolicy::A16Only,
                                            linear_suite::Execution::Eager, "rowsplit-domain");
        test.policies = 7;
        suite.add(std::move(test), [domain] { return verify_domain(domain); });
    }
    for (const auto& rejection : kRejections) {
        auto test     = linear_suite::make_case(linear_suite::format_name(rejection.qtype),
                                                rejection.n, rejection.k, rejection.t,
                                                ops::LinearPolicy::A16Only,
                                                linear_suite::Execution::Eager, "rejected");
        test.policies = 7;
        suite.add(std::move(test), [rejection] { return verify_rejection(rejection); });
    }
}

} // namespace

void ninfer::test::linear::add_rowsplit_boundary_cases(Suite& suite) {
    add_numeric_cases(suite);
    add_contract_cases(suite);
}
