#pragma once

#include "core/layout.h"
#include "ops/softmax_attention/kernels/causal/operands.h"

#include <algorithm>
#include <cstdint>

namespace ninfer::ops::detail::r9700 {

// Fixed tensor geometry determines the launch topology. Device positions determine
// each partition's actual key range, including during graph replay/update.
inline int causal_splits(int width, int batch) {
    const auto queries = static_cast<std::int64_t>(width) * batch;
    return queries <= 32 ? 16 : queries <= 128 ? 4 : 1;
}

inline std::size_t causal_workspace_bytes(int heads, int batch, int first, int last) {
    std::size_t maximum = 0;
    // Above 128 physical queries the direct route needs no partial storage.
    // Check every remaining width: the 16 -> 4 -> 1 split transitions are nonmonotone.
    for (int width = first; width <= std::min(last, 128 / batch); ++width) {
        WorkspaceLayoutBuilder layout;
        (void)allocate_causal_partials(layout, heads, width, causal_splits(width, batch), batch);
        maximum = std::max(maximum, layout.peak_bytes(1));
    }
    return maximum;
}

} // namespace ninfer::ops::detail::r9700
