#pragma once

#include "sender.h"
#include "switch_node.h"
#include "receiver.h"
#include "metrics.h"
#include "event.h"
#include "params.h"
#include <vector>
#include <string>

class Network {
public:
    Network(int n_senders, DCQCNParams p,
            double bw_mbps, double k_min, double k_max, double p_max, double max_buf);

    void run(double duration_ms);
    void write_csv(const std::string& filename) const;
    Metrics get_metrics() const;

private:
    std::vector<Sender> _senders;
    Switch _sw;
    Receiver _receiver;
    EventQueue _eq;
    MetricsCollector _metrics;

    static constexpr double SAMPLE_INTERVAL_MS = 10.0;
};
