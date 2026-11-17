// R97-11 oracle cases for ops::argmax on r9700 (wave32). Expected indices are fixed by construction
// (documented per case), not computed by production code. Register with add_argmax_edge_cases().
#include "ops/suite.h"
#include "ninfer/ops/argmax.h"
#include "cuda_fixture.h"
#include "core/device.h"
#include "ops/op_tester.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

using namespace ninfer;
using namespace ninfer::test;

namespace {

constexpr std::uint16_t kNegInf = 0xff80, kPosInf = 0x7f80, kNaN = 0x7fc0, kNegZero = 0x8000;
constexpr std::uint16_t kPosZero = 0x0000, kMinusOne = 0xbf80, kMinusEight = 0xc100;
constexpr std::uint16_t kNine = 0x4110, kTwo = 0x4000;

struct Edge {
    const char* name;
    std::int32_t physical, valid;
    std::uint16_t fill;
    std::vector<std::pair<std::int32_t, std::uint16_t>> set; // row -> bf16 word
    std::int32_t expected;                                   // lowest index among the maxima
    std::uint16_t padding = kMinusEight;                     // rows [valid, physical)
};

int run_edge(const Edge& e) {
    std::vector<std::uint16_t> logits(static_cast<std::size_t>(e.physical), e.fill);
    for (std::int32_t row = e.valid; row < e.physical; ++row) logits[row] = e.padding;
    for (const auto& [row, word] : e.set) logits[row] = word;

    GuardedDeviceBuffer device_logits(logits.size() * sizeof(std::uint16_t));
    GuardedDeviceBuffer device_output(sizeof(std::int32_t));
    device_logits.copy_from_host(logits.data(), device_logits.bytes());
    device_output.fill(0xcd);
    Tensor logits_tensor(device_logits.span(), DType::BF16, {e.physical, 1});
    Tensor output_tensor(device_output.span(), DType::I32, {1});
    ops::argmax(logits_tensor, output_tensor, e.valid, cuda_environment().default_execution);
    cuda_synchronize();

    const std::string label = std::string("argmax edge ") + e.name;
    int failures = verify_exact(label.c_str(), from_device<std::int32_t>(device_output.data(), 1),
                                std::vector<std::int32_t>{e.expected});
    failures += verify_exact((label + " preserves logits").c_str(),
                             from_device<std::uint16_t>(device_logits.data(), logits.size()), logits);
    failures += device_logits.verify_guards((label + " logits").c_str());
    failures += device_output.verify_guards((label + " output").c_str());
    return failures;
}

const std::vector<Edge>& edges() {
    // Contract (include/ninfer/ops/argmax.h): float comparison, equal maxima -> lowest row,
    // rows >= valid_rows never participate (whatever they hold, NaN and +Inf included).
    // NaN inside [0,valid_rows) is NOT specified by the contract: deliberately no such case.
    static const std::vector<Edge> all = {
        {"tie across wave32 lanes 31/32", 100, 100, kMinusEight, {{31, kNine}, {32, kNine}, {99, kNine}}, 31},
        {"tie across 63/64", 128, 128, kMinusEight, {{63, kNine}, {64, kNine}}, 63},
        {"tie at 0 and last", 97, 97, kMinusEight, {{0, kNine}, {96, kNine}}, 0},
        {"max only in last valid row of a non multiple of 32", 37, 37, kMinusEight, {{36, kTwo}}, 36},
        {"signed zeros compare equal, -0 first", 40, 40, kMinusOne, {{5, kNegZero}, {17, kPosZero}, {30, kPosZero}}, 5},
        {"signed zeros compare equal, +0 first", 64, 64, kMinusOne, {{3, kPosZero}, {50, kNegZero}}, 3},
        {"-0 only max vs +0 later", 64, 64, kMinusOne, {{9, kNegZero}, {41, kPosZero}}, 9},
        {"all -inf", 70, 70, kNegInf, {}, 0},
        {"+inf ties", 130, 130, kMinusEight, {{7, kPosInf}, {70, kPosInf}, {5, kNine}}, 7},
        {"single valid row, hostile padding", 70, 1, kMinusEight, {{0, kMinusOne}}, 0, kPosInf},
        {"NaN padding does not participate", 64, 37, kMinusEight, {{36, kTwo}}, 36, kNaN},
        {"+inf padding does not participate", 64, 33, kMinusEight, {{32, kTwo}}, 32, kPosInf},
        {"all equal", 1000, 1000, kMinusOne, {}, 0},
        {"vocab edge: last valid row wins", 248320, 248077, kMinusEight, {{248076, kNine}}, 248076, kPosInf},
        {"vocab edge: tie last vs interior", 248320, 248077, kMinusEight, {{248076, kNine}, {12345, kNine}}, 12345, kNaN},
        {"vocab edge: row 0 vs last row", 248320, 248320, kMinusEight, {{0, kNine}, {248319, kNine}}, 0},
        {"vocab edge: last row only", 248320, 248320, kMinusEight, {{248319, kNine}}, 248319},
    };
    return all;
}

} // namespace

void add_argmax_edge_cases(ninfer::test::OpSuite& suite) {
    for (std::size_t i = 0; i < edges().size(); ++i)
        suite.add("argmax/edge/" + std::to_string(i), [i] { return run_edge(edges()[i]); });
}
