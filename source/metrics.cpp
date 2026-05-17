#include "metrics.h"
#include <fstream>
#include <numeric>
#include <cmath>


static double jain_fairness(const std::vector<Sender>& senders)
{
    int n = senders.size(); 
    if (!n)
        return 0.0;
    double sum = 0.0, sum_sq = 0.0;
    for (const Sender& s : senders) {
        sum += s.rate();
        sum_sq += s.rate() * s.rate();
    }
    if (sum_sq == 0.0) 
        return 1.0;
    return (sum * sum) / (n * sum_sq);
}


void MetricsCollector::sample(double time,
                               const std::vector<Sender>& senders,
                               const Switch& sw)
{
    double total = 0.0, fairness = 0.0;
    for (const Sender& s : senders) 
        total += s.rate();
    fairness = jain_fairness(senders);
    _samples.push_back({time, total, fairness});
}


void MetricsCollector::write_csv(const std::string& filename) const
{
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "time,throughput,fairness\n";
        for (const auto& s : _samples)
            file << s.time << "," << s.throughput << "," << s.fairness << "\n";

    }
}

Metrics MetricsCollector::summary() const
{
    int n = _samples.size();
    if (!n) 
        return {};

    double sum_throughput = 0.0, sum_fairness = 0.0, sum_throughput_variance = 0.0;
    for (const auto& s : _samples)
    {
        sum_throughput += s.throughput;
        sum_fairness += s.fairness;
    }
    double avg_throughput = sum_throughput / n;
    double avg_fairness = sum_fairness / n;

    for (const auto& s : _samples)
        sum_throughput_variance += std::pow((s.throughput - avg_throughput), 2);
    double throughput_variance = sum_throughput_variance / n;

    return {avg_throughput, throughput_variance, avg_fairness};
}
