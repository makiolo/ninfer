#include "ops/suite.h"
// R97-15 edge cases for the HIP wave32 port of sample() and speculative_accept_greedy_drafts().
//
// Every reference below is independent of src/: a CPU argmax with the lowest-id tie rule, support
// sets derived by hand from the documented top-k / top-p rules, and an acceptance loop written from
// the sampling.h / speculative_accept.h contract.  Selections are compared exactly.  The RNG is
// never reproduced; stochastic rows are only constrained to an exactly known support, or to
// top_k=1 where the law is deterministic.  Logits are BF16-representable and penalties exactly
// representable, so float and double references agree bit for bit and no tolerance is used.
#include "ninfer/ops/sampling.h"
#include "ninfer/ops/speculative_accept.h"
#include "cuda_fixture.h"
#include "core/buffer.h"
#include "ops/op_tester.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <set>
#include <string>
#include <vector>

using namespace ninfer;
using namespace ninfer::test;

namespace {

constexpr float kInf = std::numeric_limits<float>::infinity();

std::vector<std::uint16_t> to_bits(const std::vector<float>& values) {
    std::vector<std::uint16_t> bits(values.size());
    for (std::size_t i = 0; i < values.size(); ++i) bits[i] = f32_to_bf16(values[i]);
    return bits;
}

// Lowest id among the maxima; -inf is an ordinary (losing) value, NaN is out of contract.
int argmax_lowest(const float* column, int domain) {
    int best = 0;
    for (int v = 1; v < domain; ++v)
        if (column[v] > column[best]) best = v;
    return best;
}

// One sample() call over `positions.size()` rows of identical or distinct columns.
std::vector<int> run_sample(const std::vector<float>& logits, int physical_rows, int domain,
                            std::vector<ops::SamplingConfig> configs,
                            const std::vector<int>& positions, int purpose, int& failures) {
    const int batch = static_cast<int>(configs.size());
    const auto bits = to_bits(logits);
    DeviceBuffer d_logits    = to_device(bits);
    DeviceBuffer d_configs   = to_device(configs);
    DeviceBuffer d_positions = to_device(positions);
    GuardedDeviceBuffer out(static_cast<std::size_t>(batch) * sizeof(std::int32_t));
    const std::vector<int> sentinel(static_cast<std::size_t>(batch), -777777);
    out.copy_from_host(sentinel.data(), sentinel.size() * sizeof(int));
    Tensor x(d_logits.span(), DType::BF16, {physical_rows, batch});
    Tensor y(out.span(), DType::I32, {batch});
    Tensor p(d_positions.span(), DType::I32, {batch});
    const std::size_t bytes = ops::sampling_workspace_capacity_bytes(domain, batch, batch);
    DeviceBuffer storage(cuda_environment().context.device(), std::max<std::size_t>(256, bytes));
    MemoryArena workspace(storage.span());
    ops::sample(x, y, domain, static_cast<const ops::SamplingConfig*>(d_configs.data()), p, purpose,
                {}, workspace, cuda_environment().default_execution);
    cuda_synchronize();
    failures += out.verify_guards("r9700 sample output");
    failures += verify_exact("r9700 sample logits read-only",
                             from_device<std::uint16_t>(d_logits, bits.size()), bits);
    return from_device<int>(out.data(), static_cast<std::size_t>(batch));
}

// Greedy rows are exact against the lowest-id argmax for domains / batches that are not multiples
// of 32 or 64, with exact ties placed on both sides of every 32/64 lane boundary, +-0, and -inf.
int greedy_wave_boundaries(int domain, int batch) {
    const int physical_rows = domain + 3;
    std::vector<float> logits(static_cast<std::size_t>(physical_rows) * batch, -2.0f);
    std::vector<int> expected(batch);
    for (int b = 0; b < batch; ++b) {
        float* col = &logits[static_cast<std::size_t>(b) * physical_rows];
        for (int v = 0; v < domain; v += 7) col[v] = -kInf;  // legal non-finite losers
        const int hi = std::min(domain - 1, 31 + 32 * (b % 3)); // 31, 63, 95 (clamped)
        const int lo = std::min(domain - 1, hi + 1);            // 32, 64, 96: tie across the boundary
        col[hi] = col[lo] = 3.0f + (b % 5);
        col[domain - 1] = std::max(col[domain - 1], 1.0f);       // last real token, below the tie
        col[domain] = col[domain + 1] = col[domain + 2] = 1000.0f; // physical padding is never read
        if (b % 4 == 3) { col[0] = -0.0f; col[1] = 0.0f; std::fill(col + 2, col + domain, -kInf);
                          col[domain - 1] = -kInf; }              // +-0 tie -> id 0
        expected[b] = argmax_lowest(col, domain);
    }
    std::vector<ops::SamplingConfig> configs(batch);
    std::vector<int> positions(batch, 5);
    int failures = 0;
    const auto got = run_sample(logits, physical_rows, domain, configs, positions,
                                ops::kSamplePurposeDecode, failures);
    return failures + verify_exact(("r9700 greedy ties v" + std::to_string(domain) + " b" +
                                    std::to_string(batch)).c_str(), got, expected);
}

int greedy_all_equal_and_inf() {
    int failures = 0;
    for (int domain : {1, 2, 33, 65, 1000}) {
        const int batch = 5;
        std::vector<float> logits(static_cast<std::size_t>(domain) * batch, 0.5f);
        std::vector<int> expected(batch, 0);
        for (int b = 1; b < batch; ++b) { // a single finite survivor among -inf
            float* col = &logits[static_cast<std::size_t>(b) * domain];
            std::fill(col, col + domain, -kInf);
            expected[b] = (b * 29) % domain;
            col[expected[b]] = -3.0f;
        }
        std::vector<ops::SamplingConfig> configs(batch);
        const auto got = run_sample(logits, domain, domain, configs, std::vector<int>(batch, 9),
                                    ops::kSamplePurposeDecode, failures);
        failures += verify_exact(("r9700 all-equal / single finite v" + std::to_string(domain)).c_str(),
                                 got, expected);
    }
    return failures;
}

// Stochastic rows with an exactly known support: every draw lies in it and every member appears
// (2048 draws; a miss has probability < 1e-100 for support <= 4 with equal weights).
int check_support(const char* label, const std::vector<float>& column, int domain,
                  ops::SamplingConfig config, const std::set<int>& support) {
    constexpr int batch = 8, rounds = 256;
    int failures = 0;
    std::vector<float> logits;
    for (int b = 0; b < batch; ++b) logits.insert(logits.end(), column.begin(), column.end());
    std::set<int> seen;
    for (int r = 0; r < rounds; ++r) {
        std::vector<int> positions(batch);
        for (int b = 0; b < batch; ++b) positions[b] = r * batch + b;
        const auto got = run_sample(logits, static_cast<int>(column.size()), domain,
                                    std::vector<ops::SamplingConfig>(batch, config), positions,
                                    ops::kSamplePurposeDecode, failures);
        for (int t : got) {
            if (!support.count(t)) { std::cerr << label << ": token " << t << " outside support\n"; ++failures; }
            seen.insert(t);
        }
    }
    if (seen != support) { std::cerr << label << ": support not fully reached\n"; ++failures; }
    return failures;
}

int topk_tie_frontier() {
    // ranks: id 0 (5.0) then ids 1,2,3,5,6 tied at 3.0 -> top_k keeps the LOWEST tied ids.
    std::vector<float> column(67, 0.0f);
    column[0] = 5.0f;
    for (int id : {1, 2, 3, 5, 6}) column[id] = 3.0f;
    column[4] = 1.0f;
    int failures = 0;
    for (int k : {2, 3, 4}) {
        ops::SamplingConfig config;
        config.temperature = 1.0f;
        config.top_k       = k;
        config.seed        = 77 + k;
        const std::set<int> support = k == 2 ? std::set<int>{0, 1} : k == 3 ? std::set<int>{0, 1, 2}
                                                                              : std::set<int>{0, 1, 2, 3};
        failures += check_support(("r9700 top-k frontier tie k=" + std::to_string(k)).c_str(), column,
                                  67, config, support);
    }
    return failures;
}

int topp_exact_boundary() {
    // Four equal logits: weights are exactly 1.0 and the cumulative sums exactly 1,2,3,4 in any
    // reduction order, so the ">= top_p * total" comparison is exact.  The rest is -inf (weight 0).
    std::vector<float> column(70, -kInf);
    for (int id = 0; id < 4; ++id) column[id] = 0.0f;
    struct Case { float p; int n; } cases[] = {
        {0.25f, 1}, {std::nextafter(0.25f, 1.0f), 2}, {0.5f, 2}, {std::nextafter(0.5f, 1.0f), 3},
        {0.75f, 3}, {std::nextafter(0.75f, 1.0f), 4}, {0.99f, 4}};
    int failures = 0;
    for (const auto& c : cases) {
        ops::SamplingConfig config;
        config.temperature = 1.0f;
        config.top_p       = c.p;
        config.seed        = 31337;
        std::set<int> support;
        for (int id = 0; id < c.n; ++id) support.insert(id);
        failures += check_support(("r9700 top-p exact boundary n=" + std::to_string(c.n)).c_str(),
                                  column, 70, config, support);
    }
    return failures;
}

// Same (seed, position, purpose) -> same token, run to run and independent of batch composition
// and of the row's slot; positions that differ must be able to differ (not all rows identical).
int determinism_across_batches() {
    int failures = 0;
    for (int domain : {257, 1000}) {
        std::vector<float> column(domain);
        for (int v = 0; v < domain; ++v) column[v] = static_cast<float>((v * 37) % 16) * 0.25f;
        ops::SamplingConfig config;
        config.temperature = 1.0f;
        config.top_k       = 20;
        config.seed        = 20261117;
        std::vector<int> reference;
        for (int p = 0; p < 33; ++p) {
            reference.push_back(run_sample(column, domain, domain, {config}, {1000 + p},
                                           ops::kSamplePurposeDecode, failures).front());
        }
        for (int batch : {5, 17, 33}) {
            std::vector<float> logits;
            std::vector<int> positions, expected;
            for (int b = 0; b < batch; ++b) {
                const int p = (b * 7) % 33; // permuted slots
                logits.insert(logits.end(), column.begin(), column.end());
                positions.push_back(1000 + p);
                expected.push_back(reference[p]);
            }
            const std::vector<ops::SamplingConfig> configs(batch, config);
            for (int run = 0; run < 2; ++run) {
                const auto got = run_sample(logits, domain, domain, configs, positions,
                                            ops::kSamplePurposeDecode, failures);
                failures += verify_exact(("r9700 sample determinism v" + std::to_string(domain) +
                                          " b" + std::to_string(batch)).c_str(), got, expected);
            }
        }
        if (std::set<int>(reference.begin(), reference.end()).size() < 2) {
            std::cerr << "r9700 determinism fixture produced a constant stream\n";
            ++failures;
        }
    }
    return failures;
}

// ---- speculative_accept_greedy_drafts: independent loop from the public contract -------------

struct AcceptRow {
    std::vector<int> drafts;       // [K]
    int extent = 0;
    ops::SamplingConfig config;
};

int accept_case(const char* label, int k, int domain, std::vector<AcceptRow> rows,
                const std::vector<float>& logits /* [domain][K+1][B], BF16-exact */,
                const std::vector<int>& counts /* [domain] shared */, bool raw_targets) {
    const int batch = static_cast<int>(rows.size()), cols = k + 1;
    std::vector<int> targets(static_cast<std::size_t>(cols) * batch), steps(static_cast<std::size_t>(k) * batch);
    std::vector<int> extents(batch), positions(batch), expected(static_cast<std::size_t>(cols) * batch, 0),
        expected_counts(batch);
    std::vector<ops::SamplingConfig> configs;
    DeviceBuffer d_counts = to_device(counts);
    for (int b = 0; b < batch; ++b) {
        auto& row = rows[b];
        row.config.token_counts = static_cast<const std::int32_t*>(d_counts.data());
        configs.push_back(row.config);
        extents[b]   = row.extent;
        positions[b] = 100 + b;
        const float pres = row.config.presence_penalty, freq = row.config.frequency_penalty;
        auto adjusted = [&](int col, int v) { // penalty overlay = counts + drafts[0:col)
            int c = counts[v];
            for (int j = 0; j < col; ++j) c += row.drafts[j] == v;
            return logits[(static_cast<std::size_t>(b) * cols + col) * domain + v] - pres * (c > 0) - freq * c;
        };
        auto top = [&](int col) {
            int best = 0;
            for (int v = 1; v < domain; ++v) if (adjusted(col, v) > adjusted(col, best)) best = v;
            return best;
        };
        int a = 0;
        while (a < row.extent && top(a) == row.drafts[a]) ++a;
        for (int i = 0; i < a; ++i) expected[b * cols + i] = row.drafts[i];
        expected[b * cols + a] = top(a);
        expected_counts[b]     = a + 1;
        for (int i = 0; i < k; ++i) steps[b * k + i] = row.drafts[i];
        for (int i = 0; i < cols; ++i) { // raw argmax over token_domain, lowest id on ties
            int best = 0;
            for (int v = 1; v < domain; ++v)
                if (logits[(static_cast<std::size_t>(b) * cols + i) * domain + v] >
                    logits[(static_cast<std::size_t>(b) * cols + i) * domain + best]) best = v;
            targets[b * cols + i] = raw_targets ? best : -1; // only raw rows consume them
        }
    }
    const auto bits = to_bits(logits);
    DeviceBuffer d_targets = to_device(targets), d_logits = to_device(bits), d_drafts = to_device(steps),
                 d_extents = to_device(extents), d_pos = to_device(positions), d_cfg = to_device(configs);
    GuardedDeviceBuffer tokens(expected.size() * sizeof(int)), count(batch * sizeof(int));
    tokens.fill(0x5a);
    count.fill(0x5a);
    Tensor t(d_targets.span(), DType::I32, {cols, batch}), x(d_logits.span(), DType::BF16, {domain, cols, batch}),
        d(d_drafts.span(), DType::I32, {k, batch}), e(d_extents.span(), DType::I32, {batch}),
        p(d_pos.span(), DType::I32, {batch}), y(tokens.span(), DType::I32, {cols, batch}),
        n(count.span(), DType::I32, {batch});
    const std::size_t bytes =
        ops::speculative_accept_greedy_drafts_workspace_capacity_bytes(domain, k, k, batch, batch);
    DeviceBuffer storage(cuda_environment().context.device(), std::max<std::size_t>(256, bytes));
    MemoryArena workspace(storage.span());
    ops::speculative_accept_greedy_drafts(t, x, d, e, p, y, n, domain,
                                          static_cast<const ops::SamplingConfig*>(d_cfg.data()), {},
                                          workspace, cuda_environment().default_execution);
    cuda_synchronize();
    int failures = verify_exact((std::string(label) + " tokens").c_str(),
                                from_device<int>(tokens.data(), expected.size()), expected);
    failures += verify_exact((std::string(label) + " counts").c_str(), from_device<int>(count.data(), batch), expected_counts);
    failures += verify_exact((std::string(label) + " positions unchanged").c_str(), from_device<int>(d_pos, batch), positions);
    failures += verify_exact((std::string(label) + " logits unchanged").c_str(), from_device<std::uint16_t>(d_logits, bits.size()), bits);
    failures += tokens.verify_guards(label) + count.verify_guards(label);
    return failures;
}

// Rejection at every position (and none / all accepted) for odd batch sizes; raw-target route.
int accept_reject_each_position(int k, int batch, int domain) {
    const int cols = k + 1;
    std::vector<float> logits(static_cast<std::size_t>(domain) * cols * batch, -2.0f);
    std::vector<AcceptRow> rows(batch);
    for (int b = 0; b < batch; ++b) {
        const int reject = b % (k + 2);         // 0..k+1: k+1 => every draft matches
        rows[b].extent   = (b / (k + 2)) % 2 ? std::max(0, k - 1) : k; // some rows exercise extent < K
        for (int i = 0; i < cols; ++i) {
            const int winner = (11 * b + 5 * i + 3) % domain;
            logits[(static_cast<std::size_t>(b) * cols + i) * domain + winner] = 4.0f;
            if (i < k) rows[b].drafts.push_back(i == reject ? (winner + 1) % domain : winner);
        }
    }
    return accept_case(("r9700 accept raw k" + std::to_string(k) + " b" + std::to_string(batch) +
                        " v" + std::to_string(domain)).c_str(), k, domain, rows, logits,
                       std::vector<int>(domain, 0), true);
}

// Non-raw route: penalties, counts, prefix overlay, temperature<=0 and top_k=1 stochastic rows
// (the latter has a deterministic law), mixed in one batch; logits on a quarter grid, so
// presence 1.0 / frequency 0.5 arithmetic is exact in float.
int accept_penalised_mixed(int k, int batch, int domain) {
    const int cols = k + 1;
    std::vector<float> logits(static_cast<std::size_t>(domain) * cols * batch);
    std::vector<AcceptRow> rows(batch);
    std::vector<int> counts(domain);
    for (int v = 0; v < domain; ++v) counts[v] = (v * 5) % 4 == 0 ? 2 : 0;
    for (std::size_t i = 0; i < logits.size(); ++i) logits[i] = static_cast<float>((i * 2654435761u >> 7) % 17) * 0.25f - 2.0f;
    for (int b = 0; b < batch; ++b) {
        rows[b].extent = b % (k + 1);
        rows[b].config.presence_penalty  = b % 2 ? 1.0f : 0.0f;
        rows[b].config.frequency_penalty = b % 3 ? 0.5f : 0.0f;
        rows[b].config.temperature       = b % 4 < 2 ? 0.0f : 1.0f;
        rows[b].config.top_k             = 1;
        rows[b].config.seed              = 1234 + b;
        for (int i = 0; i < k; ++i) { // drafts follow the penalised argmax for a few steps, then diverge
            int best = 0;
            const float pres = rows[b].config.presence_penalty, freq = rows[b].config.frequency_penalty;
            auto adj = [&](int v) {
                int c = counts[v];
                for (int j = 0; j < i; ++j) c += rows[b].drafts[j] == v;
                return logits[(static_cast<std::size_t>(b) * cols + i) * domain + v] - pres * (c > 0) - freq * c;
            };
            for (int v = 1; v < domain; ++v) if (adj(v) > adj(best)) best = v;
            rows[b].drafts.push_back(i == (b % (k + 1)) ? (best + 1) % domain : best);
        }
    }
    return accept_case(("r9700 accept penalised k" + std::to_string(k) + " b" + std::to_string(batch) +
                        " v" + std::to_string(domain)).c_str(), k, domain, rows, logits, counts, false);
}

} // namespace

void add_r9700_edge_cases(ninfer::test::OpSuite& suite) {
    for (int domain : {31, 33, 63, 65, 95, 257, 1000})
        for (int batch : {1, 3, 5, 33, 65})
            suite.add("r9700/sample/greedy_ties/v" + std::to_string(domain) + "/b" + std::to_string(batch),
                      [=] { return greedy_wave_boundaries(domain, batch); });
    suite.add("r9700/sample/greedy_all_equal_and_inf", greedy_all_equal_and_inf);
    suite.add("r9700/sample/topk_tie_frontier", topk_tie_frontier);
    suite.add("r9700/sample/topp_exact_boundary", topp_exact_boundary);
    suite.add("r9700/sample/determinism_across_batches", determinism_across_batches);
    for (auto [k, batch, domain] : {std::array{1, 3, 65}, {3, 5, 95}, {7, 33, 257}, {15, 17, 1000}})
        suite.add("r9700/accept_greedy/reject_each_position/k" + std::to_string(k) + "/b" +
                      std::to_string(batch) + "/v" + std::to_string(domain),
                  [=] { return accept_reject_each_position(k, batch, domain); });
    for (auto [k, batch, domain] : {std::array{3, 5, 65}, {5, 33, 257}, {1, 7, 33}})
        suite.add("r9700/accept_greedy/penalised_mixed/k" + std::to_string(k) + "/b" +
                      std::to_string(batch) + "/v" + std::to_string(domain),
                  [=] { return accept_penalised_mixed(k, batch, domain); });
}
