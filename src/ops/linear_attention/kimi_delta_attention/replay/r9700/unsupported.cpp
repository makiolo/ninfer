#include "ninfer/ops/kda_replay.h"

#include <stdexcept>

namespace ninfer::ops {

KdaReplayFoldPlan::KdaReplayFoldPlan(const KdaReplayRecords&, LinearAttentionStateAllLayersView) {
    throw std::invalid_argument("kda_replay_fold: not supported on r9700");
}

void KdaReplayFoldPlan::execute(std::span<const KdaReplayFoldRow>, CudaExecutionView) const {
    throw std::invalid_argument("kda_replay_fold: not supported on r9700");
}

void KdaReplayFoldPlan::execute(const Tensor&, const Tensor&, const Tensor&,
                                CudaExecutionView) const {
    throw std::invalid_argument("kda_replay_fold: not supported on r9700");
}

} // namespace ninfer::ops
