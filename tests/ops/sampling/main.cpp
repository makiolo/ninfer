#include "ops/suite.h"
#include "core/platform.h"

void add_sampling_cases(ninfer::test::OpSuite&);
void add_r9700_edge_cases(ninfer::test::OpSuite&);
void add_candidate_selector_cases(ninfer::test::OpSuite&);
void add_greedy_accept_cases(ninfer::test::OpSuite&);
void add_sparse_accept_cases(ninfer::test::OpSuite&);

void add_vocabulary_cases(ninfer::test::OpSuite&);
void add_vocabulary_sparse_cases(ninfer::test::OpSuite&);

int main(int argc, char** argv) {
    ninfer::test::OpSuite suite;
    add_sampling_cases(suite);
#if !NINFER_TARGET_R9700
    add_vocabulary_cases(suite);
    add_vocabulary_sparse_cases(suite);
    add_candidate_selector_cases(suite);
#endif
    add_greedy_accept_cases(suite);
#if !NINFER_TARGET_R9700
    add_sparse_accept_cases(suite);
#endif
    add_r9700_edge_cases(suite);
    return suite.main(argc, argv);
}
