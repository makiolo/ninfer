#include "causal/fixture.h"
#include "core/platform.h"

void add_plain_attention_cases(ninfer::test::OpSuite&);
void add_context_attention_cases(ninfer::test::OpSuite&);

int main(int argc, char** argv) {
    ninfer::test::OpSuite suite;
    ninfer::test::causal_attention::add_cases(suite);
    ninfer::test::causal_attention::add_r9700_cases(suite);
#if !NINFER_TARGET_R9700
    add_plain_attention_cases(suite);
    add_context_attention_cases(suite);
#endif
    return suite.main(argc, argv);
}
