// Host-only dump of the oracle cases and C++ expectations, consumed by crosscheck_reference.py.
// Build: any C++17 compiler, e.g. `zig c++ -std=c++17 reference_dump.cpp -o reference_dump`.
#include "primitives_reference.h"

#include <cstdio>

using namespace ninfer::test::common_ref;

int main() {
    for (const auto& c : make_vec_cases()) {
        const auto e = expect_sum(c.x, c.exact);
        std::printf("S %s %d %zu", c.name.c_str(), int(c.exact), c.x.size());
        for (float f : c.x) std::printf(" %08x", bits_of(f));
        std::printf(" | %d %a %a %d\n", int(e.kind), e.exact, e.bound, int(e.neg_zero));
        float m = c.x[0];
        for (float f : c.x) m = maxnum(m, f);
        std::printf("M %s %zu", c.name.c_str(), c.x.size());
        for (float f : c.x) std::printf(" %08x", bits_of(f));
        std::printf(" | %08x\n", bits_of(m));
    }
    for (const auto& c : make_arg_cases()) {
        if (c.has_nan) continue;
        std::printf("A %s %zu", c.name.c_str(), c.v.size());
        for (std::size_t i = 0; i < c.v.size(); ++i) std::printf(" %08x:%d", bits_of(c.v[i]), c.id[i]);
        std::printf(" | %d\n", ref_argmax(c, int(c.v.size())));
    }
    for (const auto& c : make_moment_cases()) {
        const auto r = ref_moments(c.x);
        std::printf("W %s %zu", c.name.c_str(), c.x.size());
        for (float f : c.x) std::printf(" %08x", bits_of(f));
        std::printf(" | %d %a %a %d\n", r.n, r.mean, r.m2, r.nan ? 1 : r.inf ? 2 : 0);
    }
}
