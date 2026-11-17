#include "ops/suite.h"
#include "core/platform.h"

void add_rope_cases(ninfer::test::OpSuite&);
void add_rope_large_position_cases(ninfer::test::OpSuite&);
void add_rmsnorm_rope_cases(ninfer::test::OpSuite&);
void add_rmsnorm_rope_2d_cases(ninfer::test::OpSuite&);

int main(int argc, char** argv) {
    ninfer::test::OpSuite suite;
    add_rope_cases(suite);
    add_rope_large_position_cases(suite);
#if !NINFER_TARGET_R9700
    add_rmsnorm_rope_cases(suite);
#endif
    add_rmsnorm_rope_2d_cases(suite);
    return suite.main(argc, argv);
}
