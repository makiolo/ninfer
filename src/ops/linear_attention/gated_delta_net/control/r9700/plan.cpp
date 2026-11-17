#include "ops/linear_attention/gated_delta_net/control/plan.h"
#include <stdexcept>

namespace ninfer::ops::detail::gdn_control {
Plan select_projection_plan(Geometry geometry, std::int32_t columns) {
    if (columns <= 0) throw std::invalid_argument("gdn_control: nonpositive projection extent");
    if ((geometry.heads == 48 && (geometry.input_rows == 5120 || geometry.input_rows == 2560)) ||
        (geometry.heads == 32 && geometry.input_rows == 2048))
        return {Kernel::Gemv};
    throw std::invalid_argument("gdn_control r9700: unsupported geometry");
}

NormPlan select_norm_plan(Geometry geometry, std::int32_t columns) {
    const auto projection = select_projection_plan(geometry, columns);
    if (geometry.input_rows == 2560)
        throw std::invalid_argument("gdn_control r9700: unsupported norm geometry");
    return {NormKernel::Simt, projection, {}};
}

std::size_t capacity(Geometry geometry, bool norm, std::int32_t first, std::int32_t last) {
    validate_interval(first, last);
    if (norm)
        (void)select_norm_plan(geometry, first);
    else
        (void)select_projection_plan(geometry, first);
    return 0;
}
} // namespace ninfer::ops::detail::gdn_control
