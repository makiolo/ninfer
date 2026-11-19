#include "ninfer/ops/sliding_window_attention.h"

#include <stdexcept>

namespace ninfer::ops {

SlidingWindowAttentionGraphProfile
sliding_window_attention_graph_profile(AttentionHeadGeometry, std::uint32_t window,
                                       SlidingWindowAttentionExecutionEnvelope, std::int32_t tokens,
                                       std::int32_t batch) {
    throw std::invalid_argument("sliding_window_attention_graph_profile: not supported on r9700");
}

} // namespace ninfer::ops
