#include "ops/linear_attention/gated_delta_net/recurrence/plan.h"
#include "ops/linear_attention/gated_delta_net/recurrence/chunked/launch.h"
#include <stdexcept>

namespace ninfer::ops::detail::gated_delta_net {
RecurrenceRoute select_recurrence_route(int, int, int) { return RecurrenceRoute::Recurrent; }

int select_value_tile(int, int, int, int) {
    throw std::logic_error("GDN r9700: chunked MMA recurrence is unavailable");
}

namespace chunked {
void launch_prepare(const RecurrenceOperands&, QkChunk*, ControlChunk*, bool, cudaStream_t) {
    throw std::logic_error("GDN r9700: chunked MMA preparation is unavailable");
}

void launch_recurrence(const RecurrenceOperands&, const QkChunk*, const ControlChunk*, int,
                       cudaStream_t) {
    throw std::logic_error("GDN r9700: chunked MMA recurrence is unavailable");
}
} // namespace chunked
} // namespace ninfer::ops::detail::gated_delta_net
