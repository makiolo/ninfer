// R97-11 oracle cases (generated tables: .agent-tmp/R97-11-oracle/gen2.py, independent Python reference).
// RMSNorm edge cases for r9700 (wave32 block reductions): D not a multiple of 32/64, large scales
// inside the FP32 contract (sum of squares must stay < 3.4e38), tiny eps, subnormal inputs.
// Inputs come from a mirrored LCG (see lcg()); inv = 1/sqrt(mean(x^2)+eps) per row comes from the
// table (exact rational sum of squares, 60-digit sqrt). The oracle here is only out = x * inv * gain.
// Register with add_rmsnorm_edge_cases(); criterion = BF16 RNE profile of rmsnorm.cpp with no
// absolute floor (gross_absolute = 0) so tiny outputs cannot pass vacuously.
#include "ops/suite.h"
#include "ninfer/ops/rmsnorm.h"
#include "cuda_fixture.h"
#include "core/device.h"
#include "ops/op_tester.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using namespace ninfer;
using namespace ninfer::test;

namespace {

struct Case {
    std::int32_t d, rows;
    bool unit_offset;
    std::uint32_t eps_bits;
    double scale;
    std::uint32_t seed;
    std::vector<double> inverse; // per row, from the independent reference
};

std::vector<std::uint16_t> lcg_bf16(std::uint32_t& state, std::size_t n, double scale) {
    std::vector<std::uint16_t> out(n);
    for (auto& word : out) {
        state = state * 1664525u + 1013904223u;
        word = f32_to_bf16(static_cast<float>((static_cast<double>(state >> 8) / 8388608.0 - 1.0) * scale));
    }
    return out;
}

const std::vector<Case>& cases() {
    static const std::vector<Case> all = {
        {33, 2, false, 0x358637bdu, 4.0, 1137u, {0.5141492704679775, 0.4147725408952401}},
        {33, 2, false, 0x358637bdu, 1e+17, 2137u, {1.7513118536279406e-17, 1.633286368555553e-17}},
        {33, 2, false, 0x358637bdu, 100000000.0, 3137u, {1.6169149984041365e-08, 2.013582813943277e-08}},
        {33, 2, false, 0x358637bdu, 0.003, 4137u, {495.8082334653294, 533.940843819777}},
        {63, 2, false, 0x358637bdu, 4.0, 5137u, {0.44219047144762225, 0.4248667946056124}},
        {63, 2, false, 0x358637bdu, 1e+17, 6137u, {2.0041819490969875e-17, 1.6173392646879693e-17}},
        {63, 2, false, 0x358637bdu, 100000000.0, 7137u, {1.598764728711065e-08, 1.739404584238674e-08}},
        {63, 2, false, 0x358637bdu, 0.003, 8137u, {476.6306001149039, 519.5434146357288}},
        {65, 2, false, 0x358637bdu, 4.0, 9137u, {0.43258883352894695, 0.4404128930913611}},
        {65, 2, false, 0x358637bdu, 1e+17, 10137u, {1.7234701389298306e-17, 1.7354313788299266e-17}},
        {65, 2, false, 0x358637bdu, 100000000.0, 11137u, {1.851102284238111e-08, 1.8324863178287078e-08}},
        {65, 2, false, 0x358637bdu, 0.003, 12137u, {489.2091271878807, 533.7022205519131}},
        {127, 2, false, 0x358637bdu, 4.0, 13137u, {0.4641958823818496, 0.4332380335941816}},
        {127, 2, false, 0x358637bdu, 1e+17, 14137u, {1.621785026511225e-17, 1.8381883227712456e-17}},
        {127, 2, false, 0x358637bdu, 100000000.0, 15137u, {1.7560693758135128e-08, 1.684744301787355e-08}},
        {127, 2, false, 0x358637bdu, 0.003, 16137u, {512.2504931723199, 488.8467869993255}},
        {131, 2, false, 0x358637bdu, 4.0, 17137u, {0.4242532899709577, 0.41379007394185086}},
        {131, 2, false, 0x358637bdu, 1e+17, 18137u, {1.60430667408187e-17, 1.798007207239158e-17}},
        {131, 2, false, 0x358637bdu, 100000000.0, 19137u, {1.7395767969105828e-08, 1.743693309318977e-08}},
        {131, 2, false, 0x358637bdu, 0.003, 20137u, {483.29877376403414, 500.5723115410465}},
        {1000, 2, true, 0x358637bdu, 4.0, 21137u, {0.4303660701680223, 0.4412957514987771}},
        {1000, 2, true, 0x358637bdu, 1e+17, 22137u, {1.7053334807704013e-17, 1.747503990941789e-17}},
        {1000, 2, true, 0x358637bdu, 100000000.0, 23137u, {1.730862077874299e-08, 1.7403577795147027e-08}},
        {1000, 2, true, 0x358637bdu, 0.003, 24137u, {493.1067267884797, 503.5773938109542}},
        {1025, 2, false, 0x358637bdu, 4.0, 25137u, {0.4296661530952583, 0.4380866818963661}},
        {1025, 2, false, 0x358637bdu, 1e+17, 26137u, {1.7127746843093013e-17, 1.7585971674410853e-17}},
        {1025, 2, false, 0x358637bdu, 100000000.0, 27137u, {1.7322114592065824e-08, 1.7467260803058474e-08}},
        {1025, 2, false, 0x358637bdu, 0.003, 28137u, {497.84664299909673, 507.2646010230855}},
        {5120, 2, true, 0x358637bdu, 4.0, 29137u, {0.4322282784271692, 0.4314462221387483}},
        {5120, 2, true, 0x358637bdu, 1e+17, 30137u, {1.7418511775782623e-17, 1.7215968238067903e-17}},
        {5120, 2, true, 0x358637bdu, 100000000.0, 31137u, {1.730863539412584e-08, 1.7099931161434718e-08}},
        {5120, 2, true, 0x358637bdu, 0.003, 32137u, {498.0869497183218, 497.39735089069217}},
        {128, 3, true, 0x03aa2425u, 1e-15, 33137u, {1737280084870695.8, 1799739588359504.8, 1696040057629085.2}},
        {131, 2, false, 0x00000001u, 1e-39, 34137u, {2.671373890628154e+22, 2.671373890628154e+22}},
        {64, 2, true, 0x358637bdu, 1e-39, 35137u, {1000.0000012623786, 1000.0000012623786}},
        {33, 2, false, 0x0da24260u, 1e-20, 36137u, {999999998400031.2, 999999998398229.0}},
    };
    return all;
}

int run_case(std::size_t index) {
    const Case& c = cases()[index];
    const auto d = static_cast<std::size_t>(c.d), rows = static_cast<std::size_t>(c.rows);
    std::uint32_t state = c.seed;
    const auto x_words = lcg_bf16(state, d * rows, c.scale);
    const auto w_words = lcg_bf16(state, d, 1.5);
    float eps;
    std::memcpy(&eps, &c.eps_bits, sizeof(eps));

    GuardedDeviceBuffer input(x_words.size() * 2), weight(w_words.size() * 2), output(x_words.size() * 2);
    input.copy_from_host(x_words.data(), input.bytes());
    weight.copy_from_host(w_words.data(), weight.bytes());
    output.fill(0xff);
    Tensor x(input.span(), DType::BF16, {c.d, c.rows}), w(weight.span(), DType::BF16, {c.d}),
        y(output.span(), DType::BF16, {c.d, c.rows});
    ops::rmsnorm(x, w, eps, c.unit_offset, y, cuda_environment().default_execution);
    cuda_synchronize();

    std::vector<double> reference(x_words.size());
    for (std::size_t r = 0; r < rows; ++r)
        for (std::size_t k = 0; k < d; ++k) {
            const double gain = static_cast<double>(bf16_to_f32(w_words[k])) + (c.unit_offset ? 1.0 : 0.0);
            reference[r * d + k] = static_cast<double>(bf16_to_f32(x_words[r * d + k])) * c.inverse[r] * gain;
        }
    const auto got_words = from_device<std::uint16_t>(output.data(), x_words.size());
    std::vector<double> got(got_words.size());
    for (std::size_t i = 0; i < got.size(); ++i) got[i] = static_cast<double>(bf16_to_f32(got_words[i]));

    const std::string label = "rmsnorm edge #" + std::to_string(index) + " d=" + std::to_string(c.d) +
                              " scale=" + std::to_string(c.scale) + (c.unit_offset ? " unit" : " plain");
    const ReductionCriterion criterion{1.85e-3, 0.0, 3.95e-3};
    int failures = verify_reduction(label, got, reference, criterion);
    failures += verify_exact((label + " preserves input").c_str(),
                             from_device<std::uint16_t>(input.data(), x_words.size()), x_words);
    failures += verify_exact((label + " preserves weight").c_str(),
                             from_device<std::uint16_t>(weight.data(), w_words.size()), w_words);
    failures += input.verify_guards((label + " input").c_str()) + weight.verify_guards((label + " weight").c_str()) +
                output.verify_guards((label + " output").c_str());
    return failures;
}

} // namespace

void add_rmsnorm_edge_cases(ninfer::test::OpSuite& suite) {
    for (std::size_t i = 0; i < cases().size(); ++i)
        suite.add("rmsnorm/edge/" + std::to_string(i), [i] { return run_case(i); });
}
