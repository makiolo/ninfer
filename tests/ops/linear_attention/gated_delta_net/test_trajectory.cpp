// Production state trajectory of Gated DeltaNet: chunked prefill through gated_delta_net followed
// by width-one decode through gated_delta_net_batch_update on the same pooled state record, as the
// Qwen3.5 text path runs it. Every token output and every checkpoint state is compared with one
// independent FP64 recurrence (gdn_ref.h) over the complete represented input sequence.
#include "ops/suite.h"
#include "ops/state_fixture.h"
#include "ninfer/ops/gated_delta_net.h"
#include "cuda_fixture.h"
#include "core/buffer.h"

#include "gdn_ref.h"
#include "ops/op_tester.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

using namespace ninfer;
using namespace ninfer::test;

namespace {

constexpr int kStateDim = 128;

// The single public GDN error budget; identical to test_recurrence.cpp. A trajectory that mixes
// entries is still one recurrence over represented inputs, so the budget does not change.
constexpr ReductionCriterion kOutputCriterion{6.0e-3, 5.0e-6, 8.0e-3};
constexpr ReductionCriterion kStateCriterion{4.5e-3, 1.0e-5, 9.0e-3};

enum class Gates {
    Ordinary,
    // Per (token, head) choice of decay in {1, 1-1e-6, e^-3, e^-80} and beta in {0, 0.002, 0.5, 1}.
    Mixed,
    // g=0, beta=1: no forgetting, so FP32 state error accumulates over the whole sequence.
    ZeroDecayUnitBeta,
    // g alternates 0 and -80 per token: resets that straddle every prefill tile boundary.
    AlternatingReset,
};

struct Case {
    const char* name;
    int qk_heads;
    int value_heads;
    bool normalize_qk;
    std::vector<int> prefill; // gated_delta_net calls, in order
    int decode;               // following batch_update calls, one token each
    bool ping_pong;           // alternate state records instead of updating one record in place
    Gates gates;
    std::uint32_t seed;
};

int total_tokens(const Case& c) {
    return std::accumulate(c.prefill.begin(), c.prefill.end(), 0) + c.decode;
}

gdn_ref::Inputs make_inputs(const Case& c) {
    const int T = total_tokens(c);
    gdn_ref::Inputs in;
    in.head_dim    = kStateDim;
    in.qk_heads    = c.qk_heads;
    in.value_heads = c.value_heads;
    in.tokens      = T;
    in.q.resize(std::size_t(kStateDim) * c.qk_heads * T);
    in.k.resize(in.q.size());
    in.v.resize(std::size_t(kStateDim) * c.value_heads * T);
    in.g.resize(std::size_t(c.value_heads) * T);
    in.beta.resize(in.g.size());
    in.state.resize(std::size_t(kStateDim) * kStateDim * c.value_heads);

    std::mt19937 generator(c.seed);
    const auto fill = [&](std::vector<float>& values, float low, float high) {
        std::uniform_real_distribution<float> distribution(low, high);
        for (float& value : values) value = distribution(generator);
    };
    fill(in.q, -1.0F, 1.0F);
    fill(in.k, -1.0F, 1.0F);
    fill(in.v, -0.5F, 0.5F);
    fill(in.g, -0.10F, -0.005F);
    fill(in.beta, 0.05F, 0.95F);
    fill(in.state, -0.02F, 0.02F);
    if (!c.normalize_qk) {
        // Raw mode receives unit rows so the recurrence stays contractive; generation only.
        for (auto* values : {&in.q, &in.k}) {
            for (std::size_t row = 0; row < values->size() / kStateDim; ++row) {
                float* base  = values->data() + row * kStateDim;
                double sumsq = 0.0;
                for (int d = 0; d < kStateDim; ++d) sumsq += double(base[d]) * base[d];
                for (int d = 0; d < kStateDim; ++d) base[d] = float(base[d] / std::sqrt(sumsq));
            }
        }
    }
    round_to_bf16(in.q);
    round_to_bf16(in.k);
    round_to_bf16(in.v);

    std::uniform_int_distribution<int> pick(0, 3);
    for (std::size_t i = 0; i < in.g.size(); ++i) {
        const std::size_t t = i / std::size_t(c.value_heads);
        switch (c.gates) {
        case Gates::Ordinary:
            break;
        case Gates::Mixed:
            in.g[i]    = std::array<float, 4>{0.0F, -1.0e-6F, -3.0F, -80.0F}[pick(generator)];
            in.beta[i] = std::array<float, 4>{0.0F, 0.002F, 0.5F, 1.0F}[pick(generator)];
            break;
        case Gates::ZeroDecayUnitBeta:
            in.g[i]    = 0.0F;
            in.beta[i] = 1.0F;
            break;
        case Gates::AlternatingReset:
            in.g[i] = (t & 1U) == 0 ? 0.0F : -80.0F;
            break;
        }
    }
    return in;
}

gdn_ref::Inputs prefix(const gdn_ref::Inputs& in, int tokens) {
    gdn_ref::Inputs p = in;
    p.tokens          = tokens;
    p.q.resize(std::size_t(tokens) * in.qk_heads * kStateDim);
    p.k.resize(p.q.size());
    p.v.resize(std::size_t(tokens) * in.value_heads * kStateDim);
    p.g.resize(std::size_t(tokens) * in.value_heads);
    p.beta.resize(p.g.size());
    return p;
}

// Normwise comparison per value head, as the public GDN suite reports it.
int verify_heads(const std::string& label, std::span<const double> got,
                 std::span<const double> expected, int value_heads, int tokens, bool state) {
    const auto& criterion = state ? kStateCriterion : kOutputCriterion;
    const std::size_t head_size = std::size_t(kStateDim) * (state ? kStateDim : tokens);
    if (got.size() != head_size * value_heads || got.size() != expected.size())
        throw std::logic_error("GDN trajectory comparison shape mismatch");
    std::vector<double> head_got(head_size), head_ref(head_size);
    int failures = 0;
    for (int h = 0; h < value_heads; ++h) {
        if (state) {
            std::copy_n(got.data() + h * head_size, head_size, head_got.data());
            std::copy_n(expected.data() + h * head_size, head_size, head_ref.data());
        } else {
            for (int t = 0; t < tokens; ++t) {
                const auto source = (std::size_t(t) * value_heads + h) * kStateDim;
                std::copy_n(got.data() + source, kStateDim, head_got.data() + t * kStateDim);
                std::copy_n(expected.data() + source, kStateDim, head_ref.data() + t * kStateDim);
            }
        }
        const std::string head_label = label + " head=" + std::to_string(h);
        const auto stats = compute_reduction_stats(head_got.data(), head_ref.data(), head_size);
        report_reduction_stats(head_label, head_size, stats, criterion);
        if (!reduction_passes(stats, head_size, criterion)) {
            std::cerr << head_label << ": rel_l2=" << stats.relative_l2
                      << " max_abs=" << stats.maximum_absolute_error
                      << " gross_limit=" << gross_error_limit(stats, criterion)
                      << " non_finite=" << stats.first_non_finite << '\n';
            ++failures;
        }
    }
    return failures;
}

int run_trajectory(const Case& c) {
    const int T                  = total_tokens(c);
    const gdn_ref::Inputs in     = make_inputs(c);
    const float scale            = 1.0F / std::sqrt(static_cast<float>(kStateDim));
    const std::size_t state_size = in.state.size();
    const int Hq = c.qk_heads, Hv = c.value_heads;
    std::string label = std::string(c.name) + " H=" + std::to_string(Hq) + "/" +
                        std::to_string(Hv) + " T=" + std::to_string(T) + " prefill=[";
    for (int length : c.prefill) label += std::to_string(length) + ",";
    label += "] decode=" + std::to_string(c.decode) + (c.normalize_qk ? " normalized" : " raw") +
             (c.ping_pong ? " ping-pong" : " in-place");
    std::cout << "GDN_TRAJECTORY " << label << std::endl;

    // Outputs of token t depend only on tokens <= t, so one full evaluation covers every output.
    const gdn_ref::Result full = gdn_ref::evaluate(in, scale, c.normalize_qk);

    // Pool of three records: record 1 holds the initial state (not record 0, so a route that
    // hard-wires the first record fails); records 0 and 2 carry a sentinel unless ping-pong uses 2.
    constexpr int kSlots = 3;
    std::vector<float> pool(state_size * kSlots, 0.125F);
    std::copy(in.state.begin(), in.state.end(), pool.begin() + state_size);
    GuardedDeviceBuffer states(pool.size() * sizeof(float));
    states.copy_from_host(pool.data(), states.bytes());
    Tensor dense(states.span(), DType::FP32, {kStateDim, kStateDim, Hv, kSlots});
    RecordStateFixture history(dense, 3);

    DeviceBuffer q = to_device_bf16(in.q), k = to_device_bf16(in.k), v = to_device_bf16(in.v);
    DeviceBuffer g = to_device_f32(in.g), beta = to_device_f32(in.beta);
    GuardedDeviceBuffer out(in.v.size() * sizeof(std::uint16_t));
    out.fill(0xff);
    Tensor tq(q.span(), DType::BF16, {kStateDim, Hq, T});
    Tensor tk(k.span(), DType::BF16, {kStateDim, Hq, T});
    Tensor tv(v.span(), DType::BF16, {kStateDim, Hv, T});
    Tensor tg(g.span(), DType::FP32, {Hv, T});
    Tensor tb(beta.span(), DType::FP32, {Hv, T});
    Tensor to(out.span(), DType::BF16, {kStateDim, Hv, T});

    const int maximum_call =
        c.prefill.empty() ? 1 : *std::max_element(c.prefill.begin(), c.prefill.end());
    const auto workspace_bytes =
        ops::gated_delta_net_workspace_capacity_bytes(Hq, Hv, 1, maximum_call);
    GuardedDeviceBuffer scratch(std::max<std::size_t>(workspace_bytes, 256));
    scratch.fill(0xff);
    MemoryArena workspace(scratch.span());
    const auto execution = cuda_environment().default_execution;

    const std::vector<std::int32_t> records{1, 2};
    const auto offsets = history.record_offsets(records);
    DeviceBuffer offset_1 = to_device(std::vector<std::int64_t>{offsets[0]});
    DeviceBuffer offset_2 = to_device(std::vector<std::int64_t>{offsets[1]});
    const auto offset_tensor    = [&](int record) {
        return Tensor((record == 1 ? offset_1 : offset_2).span(),
                      DType::I64, {1});
    };

    int failures = 0, current = 1, begin = 0;
    std::vector<bool> touched(kSlots, false);
    touched[1] = true;
    const auto checkpoint = [&](const std::string& where) {
        history.copy_back();
        const auto got = from_device<float>(states.data(), pool.size());
        const auto ref = gdn_ref::evaluate(prefix(in, begin), scale, c.normalize_qk);
        const std::vector<double> state(got.begin() + current * state_size,
                                        got.begin() + (current + 1) * state_size);
        failures += verify_heads(label + " " + where + " prefix=" + std::to_string(begin) +
                                     " state",
                                 state, ref.final_state, Hv, 1, true);
        for (int slot = 0; slot < kSlots; ++slot) {
            if (touched[slot]) continue;
            failures += verify_exact(
                (label + " " + where + " untouched record " + std::to_string(slot)).c_str(),
                std::vector<float>(got.begin() + slot * state_size,
                                   got.begin() + (slot + 1) * state_size),
                std::vector<float>(pool.begin() + slot * state_size,
                                   pool.begin() + (slot + 1) * state_size));
        }
        failures += history.verify_guards(label + " " + where + " records");
    };

    for (std::size_t call = 0; call < c.prefill.size(); ++call) {
        const int length = c.prefill[call];
        const int next   = c.ping_pong ? 3 - current : current;
        touched[next]    = true;
        Tensor source = history.tensor(current), destination = history.tensor(next);
        Tensor os = to.slice(2, begin, length);
        ops::gated_delta_net(tq.slice(2, begin, length), tk.slice(2, begin, length),
                             tv.slice(2, begin, length), tg.slice(1, begin, length),
                             tb.slice(1, begin, length), scale, c.normalize_qk, workspace, source,
                             destination, os, execution);
        cuda_synchronize();
        begin += length;
        current = next;
        if (workspace.used() != 0) {
            std::cerr << label << ": workspace was not released\n";
            ++failures;
        }
        checkpoint("prefill call " + std::to_string(call));
    }

    for (int step = 0; step < c.decode; ++step) {
        const int next = c.ping_pong ? 3 - current : current;
        touched[next]  = true;
        Tensor os      = to.slice(2, begin, 1).view({kStateDim, Hv, 1, 1});
        ops::gated_delta_net_batch_update(
            tq.slice(2, begin, 1).view({kStateDim, Hq, 1, 1}),
            tk.slice(2, begin, 1).view({kStateDim, Hq, 1, 1}),
            tv.slice(2, begin, 1).view({kStateDim, Hv, 1, 1}),
            tg.slice(1, begin, 1).view({Hv, 1, 1}), tb.slice(1, begin, 1).view({Hv, 1, 1}), scale,
            c.normalize_qk, history.view(offset_tensor(current)),
            history.view(offset_tensor(next)), Tensor{}, os, execution);
        begin += 1;
        current = next;
        if (step == 0 || step + 1 == c.decode) {
            cuda_synchronize();
            checkpoint("decode step " + std::to_string(step));
        }
    }

    cuda_synchronize();
    failures += verify_heads(label + " out", from_device_bf16(out.data(), in.v.size()), full.out,
                             Hv, T, false);
    failures += out.verify_guards((label + " out").c_str());
    failures += scratch.verify_guards((label + " workspace").c_str());
    failures += states.verify_guards((label + " dense states").c_str());
    if (workspace.peak_used() > workspace_bytes) {
        std::cerr << label << ": workspace exceeded its interval capacity\n";
        ++failures;
    }
    return failures;
}

} // namespace

int main(int argc, char** argv) {
    OpSuite suite;
    const auto add = [&](Case c) {
        std::string id = std::string("gdn_trajectory/") + c.name + "/h" +
                         std::to_string(c.qk_heads) + "/v" + std::to_string(c.value_heads) +
                         "/p";
        for (int length : c.prefill) id += std::to_string(length) + "_";
        id += "d" + std::to_string(c.decode) + (c.normalize_qk ? "/normalized" : "/raw") +
              (c.ping_pong ? "/ping-pong" : "/in-place");
        suite.add(id, [=] { return run_trajectory(c); }, {{"execution", "eager"}});
    };
    // Qwen3.8-27B geometry: long prefill split across a tile boundary, then decode.
    add({"27b-ordinary", 16, 48, true, {1000, 24}, 40, false, Gates::Ordinary, 120001U});
    add({"27b-mixed-gates", 16, 48, true, {64, 1, 63}, 8, true, Gates::Mixed, 120002U});
    add({"27b-reset", 16, 48, true, {33, 95}, 5, false, Gates::AlternatingReset, 120003U});
    add({"35b-accumulate", 16, 32, true, {17}, 16, false, Gates::ZeroDecayUnitBeta, 120004U});
    // Small head maps (not multiples of 32/64) and long accumulation in the decode entry itself.
    add({"shared-qk-accumulate", 1, 3, true, {4096}, 512, false, Gates::ZeroDecayUnitBeta,
         120005U});
    add({"shared-qk-reset", 1, 3, true, {2049}, 3, true, Gates::AlternatingReset, 120006U});
    add({"grouped-raw-reset", 5, 15, false, {33, 31}, 17, true, Gates::AlternatingReset,
         120007U});
    add({"grouped-raw-mixed", 5, 15, false, {1, 2, 3, 127}, 9, false, Gates::Mixed, 120008U});
    add({"single-head-decode-only", 1, 1, true, {}, 64, true, Gates::Mixed, 120009U});
    return suite.main(argc, argv);
}
