// CausalConv1dSiLU coverage for channel counts that are not multiples of 32/64 (the public entry
// admits every positive C), state carried across several calls (split prefill followed by decode),
// long sequences, and pre-activations wide enough to saturate SiLU. Every output is compared with
// one FP64 oracle over the complete concatenated sequence; every state transition is exact.
#include "ops/suite.h"
#include "ninfer/ops/causal_conv1d_silu.h"
#include "cuda_fixture.h"
#include "ops/op_tester.h"

#include <cuda_runtime.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace ninfer;
using namespace ninfer::test;

namespace {

// The single public A16 criterion of causal_conv1d_silu; identical to causal_conv1d_silu.cpp.
constexpr ReductionCriterion kCriterion{1.85e-3, 1.0e-3, 3.7e-3};

// Over thousands of outputs the ideal output (BF16 RNE of the FP64 oracle) sits at rel_l2
// ~1.67e-3, inside kCriterion; over short prefixes its statistics scatter, and on the prefixes
// below the ideal output itself fails kCriterion, so those calls use the ideal statistic of that
// exact prefix rounded up to three significant digits. The bound also covers FP32 evaluation:
// rounding every output within 16 FP32 ulps of (sum |w*u| + |y|) of a BF16 tie to the far
// neighbour keeps each statistic below it. Every override still rejects all outputs one BF16 ulp up, a shifted tap, an ignored initial
// state, and one output one BF16 ulp off at its largest reference.
constexpr ReductionCriterion relative_l2(double bound) { return {bound, 1.0e-3, 3.7e-3}; }

std::vector<ReductionCriterion> single_call_criteria(std::int32_t C, std::int32_t T) {
    if (C == 1 && T == 2) return {relative_l2(2.41e-3)};  // ideal 2.4027e-3
    if (C == 1 && T == 64) return {relative_l2(1.93e-3)}; // ideal 1.9258e-3
    if (C == 3 && T == 2) return {relative_l2(3.16e-3)};  // ideal 3.1532e-3
    if (C == 3 && T == 3) return {relative_l2(2.36e-3)};  // ideal 2.3551e-3
    if (C == 3 && T == 65) return {relative_l2(1.90e-3)}; // ideal 1.8908e-3
    if (C == 33 && T == 2) return {relative_l2(1.94e-3)}; // ideal 1.9332e-3
    return {};
}

// {1,1,1,1,257,1} at C=33: prefixes of 33..132 outputs (ideal 2.1655, 2.1256, 2.0039, 1.8875e-3).
const std::vector<ReductionCriterion> kC33PingPongCriteria{
    relative_l2(2.17e-3), relative_l2(2.13e-3), relative_l2(2.01e-3),
    relative_l2(1.89e-3), kCriterion,           kCriterion};

// {1,1,1,1,257,1} at C=10240, calls 2-3: the largest error is the BF16 half ulp of 8.09348
// (8.0625, error 0.0309806), above 1e-3 + 3.7e-3 * 8.09348; (0.0309806 - 1e-3) / 8.09348 =
// 3.7043e-3, rounded up. Calls 4-5 reach max reference 9.46 and pass kCriterion again.
const std::vector<ReductionCriterion> kC10240PingPongCriteria{
    kCriterion, kCriterion, {1.85e-3, 1.0e-3, 3.71e-3}, {1.85e-3, 1.0e-3, 3.71e-3},
    kCriterion, kCriterion};

std::size_t at(std::int32_t c, std::int32_t column, std::int32_t C) {
    return std::size_t(column) * std::size_t(C) + std::size_t(c);
}

std::vector<std::uint16_t> bf16_bits(const std::vector<float>& values) {
    std::vector<std::uint16_t> bits(values.size());
    for (std::size_t i = 0; i < values.size(); ++i) bits[i] = f32_to_bf16(values[i]);
    return bits;
}

std::vector<float> values(std::size_t count, std::uint32_t seed, float bound) {
    std::vector<float> result(count);
    fill_uniform(result, seed, -bound, bound);
    round_to_bf16(result);
    return result;
}

struct Oracle {
    std::vector<double> output;
    std::vector<std::vector<float>> windows; // windows[t]: [C,3] state after token t
};

// FP64 depthwise width-4 causal convolution plus SiLU over the represented inputs.
Oracle oracle(const std::vector<float>& x, const std::vector<float>& weight,
              const std::vector<float>& state, std::int32_t C, std::int32_t T) {
    Oracle result;
    result.output.resize(std::size_t(C) * T);
    std::vector<float> window = state;
    for (std::int32_t t = 0; t < T; ++t) {
        for (std::int32_t c = 0; c < C; ++c) {
            double sum = double(weight[at(c, 3, C)]) * x[at(c, t, C)];
            for (int j = 0; j < 3; ++j) sum += double(weight[at(c, j, C)]) * window[at(c, j, C)];
            result.output[at(c, t, C)] = sum / (1.0 + std::exp(-sum));
        }
        for (std::int32_t c = 0; c < C; ++c) {
            window[at(c, 0, C)] = window[at(c, 1, C)];
            window[at(c, 1, C)] = window[at(c, 2, C)];
            window[at(c, 2, C)] = x[at(c, t, C)];
        }
        result.windows.push_back(window);
    }
    return result;
}

enum class Form { InPlace, Distinct, ExactAlias, PingPong };

const char* form_name(Form form) {
    switch (form) {
    case Form::InPlace: return "in-place";
    case Form::Distinct: return "distinct";
    case Form::ExactAlias: return "exact-alias";
    case Form::PingPong: return "ping-pong";
    }
    return "?";
}

// Runs consecutive calls over `chunks` of one sequence, carrying the conv state between calls.
// `x_bound`/`w_bound` set the input ranges; wide ranges drive |pre-activation| past FP32 exp range.
int run(std::int32_t C, std::vector<std::int32_t> chunks, Form form, float x_bound, float w_bound,
        std::uint32_t seed, std::vector<ReductionCriterion> criteria) {
    const std::int32_t T = std::accumulate(chunks.begin(), chunks.end(), 0);
    const auto x         = values(std::size_t(C) * T, seed, x_bound);
    const auto weight    = values(std::size_t(C) * 4, seed + 1, w_bound);
    const auto state     = values(std::size_t(C) * 3, seed + 2, x_bound);
    const Oracle ref     = oracle(x, weight, state, C, T);

    std::string label = "causal_conv C=" + std::to_string(C) + " T=" + std::to_string(T) +
                        " chunks=[";
    for (auto length : chunks) label += std::to_string(length) + ",";
    label += std::string("] ") + form_name(form) + " x_bound=" + std::to_string(x_bound);
    std::cout << "CAUSAL_CONV_TRAJECTORY " << label << std::endl;

    const auto x_bits = bf16_bits(x), weight_bits = bf16_bits(weight);
    const auto state_bits = bf16_bits(state);
    GuardedDeviceBuffer dx(x_bits.size() * 2), dw(weight_bits.size() * 2);
    GuardedDeviceBuffer state_a(state_bits.size() * 2), state_b(state_bits.size() * 2);
    GuardedDeviceBuffer out(x_bits.size() * 2);
    dx.copy_from_host(x_bits.data(), dx.bytes());
    dw.copy_from_host(weight_bits.data(), dw.bytes());
    state_a.copy_from_host(state_bits.data(), state_a.bytes());
    state_b.fill(0x5a);
    out.fill(0xff);
    Tensor tx(dx.span(), DType::BF16, {C, T});
    Tensor tw(dw.span(), DType::BF16, {C, 4});
    Tensor tout(out.span(), DType::BF16, {C, T});
    const auto execution = cuda_environment().default_execution;

    int failures = 0;
    std::int32_t begin = 0;
    GuardedDeviceBuffer* current = &state_a;
    for (std::size_t call = 0; call < chunks.size(); ++call) {
        const std::int32_t length = chunks[call];
        const bool fresh          = form == Form::Distinct || form == Form::PingPong;
        GuardedDeviceBuffer* next = fresh ? (current == &state_a ? &state_b : &state_a) : current;
        const auto source_before  = from_device<std::uint16_t>(current->data(), state_bits.size());
        if (fresh) next->fill(0x5a);
        Tensor input(current->span(), DType::BF16, {C, 3});
        Tensor output(next->span(), DType::BF16, {C, 3});
        Tensor xs = tx.slice(1, begin, length), os = tout.slice(1, begin, length);
        if (form == Form::InPlace) {
            ops::causal_conv1d_silu(xs, tw, input, os, execution);
        } else if (form == Form::ExactAlias) {
            ops::causal_conv1d_silu(xs, tw, input, input, os, execution);
        } else {
            ops::causal_conv1d_silu(xs, tw, input, output, os, execution);
        }
        cuda_synchronize();
        begin += length;
        const std::string where = label + " call=" + std::to_string(call);
        const auto got = from_device_bf16(out.data(), std::size_t(C) * begin);
        failures += verify_reduction(
            (where + " output prefix").c_str(), got,
            std::vector<double>(ref.output.begin(), ref.output.begin() + got.size()),
            criteria.empty() ? kCriterion : criteria[call]);
        failures += verify_exact((where + " state").c_str(),
                                 from_device<std::uint16_t>(next->data(), state_bits.size()),
                                 bf16_bits(ref.windows[begin - 1]));
        if (fresh) {
            failures += verify_exact((where + " source state preserved").c_str(),
                                     from_device<std::uint16_t>(current->data(),
                                                                state_bits.size()),
                                     source_before);
        }
        if (begin < T) {
            failures += verify_exact(
                (where + " unconsumed output untouched").c_str(),
                from_device<std::uint16_t>(static_cast<const std::uint16_t*>(out.data()) +
                                               std::size_t(C) * begin,
                                           std::size_t(C) * (T - begin)),
                std::vector<std::uint16_t>(std::size_t(C) * (T - begin), 0xffffU));
        }
        current = next;
    }
    failures += verify_exact((label + " x preserved").c_str(),
                             from_device<std::uint16_t>(dx.data(), x_bits.size()), x_bits);
    failures += verify_exact((label + " weight preserved").c_str(),
                             from_device<std::uint16_t>(dw.data(), weight_bits.size()),
                             weight_bits);
    failures += dx.verify_guards((label + " x").c_str());
    failures += dw.verify_guards((label + " weight").c_str());
    failures += state_a.verify_guards((label + " state a").c_str());
    failures += state_b.verify_guards((label + " state b").c_str());
    failures += out.verify_guards((label + " output").c_str());
    return failures;
}

} // namespace

int main(int argc, char** argv) {
    OpSuite suite;
    const auto add = [&](std::int32_t C, std::vector<std::int32_t> chunks, Form form,
                         float x_bound, float w_bound, std::uint32_t seed,
                         std::vector<ReductionCriterion> criteria = {}) {
        std::string id = "causal_conv_trajectory/c" + std::to_string(C) + "/";
        for (auto length : chunks) id += std::to_string(length) + "_";
        id += std::string("/") + form_name(form) + (x_bound > 3.0F ? "/wide" : "");
        suite.add(id, [=] { return run(C, chunks, form, x_bound, w_bound, seed, criteria); });
    };
    // Channel counts outside every 32/64 lane multiple, single call, each state form.
    std::uint32_t seed = 130000U;
    for (const std::int32_t C : {1, 3, 33, 127, 1000, 4097}) {
        for (const std::int32_t T : {1, 2, 3, 17, 64, 65, 257}) {
            const Form form = std::array<Form, 3>{Form::InPlace, Form::Distinct,
                                                  Form::ExactAlias}[seed % 3];
            add(C, {T}, form, 3.0F, 1.0F, seed, single_call_criteria(C, T));
            seed += 10U;
        }
    }
    // Split prefill then decode: chunks shorter than the width-3 window mix old state and input.
    for (const std::int32_t C : {33, 10240}) {
        add(C, {17, 1, 1, 2, 64, 1, 3, 1}, Form::InPlace, 3.0F, 1.0F, 131000U + C);
        add(C, {1, 1, 1, 1, 257, 1}, Form::PingPong, 3.0F, 1.0F, 132000U + C,
            C == 33 ? kC33PingPongCriteria : kC10240PingPongCriteria);
        add(C, {2, 300, 1, 1}, Form::ExactAlias, 3.0F, 1.0F, 133000U + C);
    }
    // Long sequences through the ordinary entry.
    add(33, {16385}, Form::Distinct, 3.0F, 1.0F, 134001U);
    add(10240, {4096, 1}, Form::InPlace, 3.0F, 1.0F, 134002U);
    add(8192, {2048, 2048, 1}, Form::PingPong, 3.0F, 1.0F, 134003U);
    // |pre-activation| up to ~1000: SiLU must saturate to x and to -0 without non-finite values.
    add(127, {65}, Form::InPlace, 120.0F, 4.0F, 135001U);
    add(10240, {17, 1}, Form::Distinct, 120.0F, 4.0F, 135002U);
    return suite.main(argc, argv);
}
