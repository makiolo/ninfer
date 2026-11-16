#pragma once

namespace ninfer::product {

// The application's single process-signal registration. Declare before the resources whose
// shutdown it requests, then install when ready to serve. Poll requested() from normal code;
// callbacks never call application code. Repeated requests remain graceful.
// On Windows, console close waits for destruction (up to the OS shutdown time allowance).
class ProcessSignals {
public:
    ProcessSignals() = default;
    ~ProcessSignals();
    ProcessSignals(const ProcessSignals&) = delete;
    ProcessSignals& operator=(const ProcessSignals&) = delete;

    void install();
    [[nodiscard]] bool requested() const noexcept;

private:
    bool installed_ = false;
};

} // namespace ninfer::product
