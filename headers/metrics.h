#pragma once

#include "sender.h"
#include "switch_node.h"
#include <vector>
#include <string> 

struct Metrics {
    double avg_throughput = 0;
    double throughput_variance = 0;
    double avg_fairness = 0;
    int pfc_count = 0;
};

struct Sample { 
    double time, 
    throughput, 
    fairness; 
};


class MetricsCollector {
public:
    void sample(double time, const std::vector<Sender>& senders, const Switch& sw);
    void write_csv(const std::string& filename) const;
    Metrics summary() const;

private:
    std::vector<Sample> _samples;
};