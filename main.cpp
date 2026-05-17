#include "params.h"
#include "sender.h"
#include "switch_node.h"
#include "receiver.h"
#include "network.h"
#include "event.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cfloat>

struct SearchResult {
    double g;
    double timer_ms;
    double variance;
    double avg_throughput;
    int pfc_count;
};

static double fitness(const Metrics& m)
{
    double norm_variance   = m.throughput_variance / 2e8;
    double norm_throughput = m.avg_throughput / 40000.0;
    double norm_pfc = m.pfc_count / 1000.0;

    return + m.avg_fairness    // convergence to equal share  (paper's primary metric)
           - norm_variance     // avoid throughput oscillation
           + norm_throughput   // maximize link utilization
           - norm_pfc;        // minimize PFC events
}

static void run_search(int n_senders, double duration_ms)
{
    {
        DCQCNParams p;
        Network def(n_senders, p, 40000.0, p.k_min, p.k_max, p.p_max, 12.0*1024*1024);
        def.run(duration_ms);
        def.write_csv("metrics_default.csv");
        Metrics dm = def.get_metrics();
        std::cout << "default g=" << p.g
                << "\ndefault timer=" << p.timer_ms
                << "\nvariance=" << dm.throughput_variance
                << "\nthroughput=" << dm.avg_throughput
                << "\npfc=" << dm.pfc_count << "\n";
    }

    SearchResult best = { 0.0, 0.0, DBL_MAX, 0.0, 0 };
    double best_f = -DBL_MAX;
    std::ofstream hmap("search_heatmap.csv");
    hmap << "g,timer_ms,variance\n";

    for (double g = 1.0/512.0; g <= 1.0/8.0; g *= 2)
    {
        for (double t = 0.01; t <= 0.15; t += 0.01)
        {
            DCQCNParams p;
            p.g = g; p.timer_ms = t;
            Network net(n_senders, p, 40000.0, p.k_min, p.k_max, p.p_max, 12.0*1024*1024);
            net.run(duration_ms);
            Metrics m = net.get_metrics();
            hmap << g << "," << t << "," << m.throughput_variance << "\n";
            double f = fitness(m);
            if (f > best_f)
            {
                best_f = f;
                best = {g, t, m.throughput_variance, m.avg_throughput, m.pfc_count};
            }

        }
    }
    hmap.close();

    {
        DCQCNParams p;
        p.g = best.g;
        p.timer_ms = best.timer_ms;
        Network bnet(n_senders, p, 40000.0, p.k_min, p.k_max, p.p_max, 12.0*1024*1024);
        bnet.run(duration_ms);
        bnet.write_csv("metrics_best.csv");
    }

    std::cout << "\n\nbest g=" << best.g
              << "\nbest timer=" << best.timer_ms
              << "\nvariance=" << best.variance
              << "\nthroughput=" << best.avg_throughput
              << "\npfc=" << best.pfc_count
              << "\n";
}

static void run_single(double sim_time)
{
    std::ofstream file("data.csv");
    if (!file.is_open()) { std::cerr << "Unable to open data.csv\n"; return; }

    file << "time,rate,queue\n";

    DCQCNParams p;
    Sender se(0, p);
    Switch sw(40000.0, p.k_min, p.k_max, p.p_max, 12.0 * 1024 * 1024);
    Receiver r;
    EventQueue eq;

    Event e;
    e.time = 0.0; e.type = PACKET_SEND; e.sender_id = 0;
    eq.push(e);

    double current_time = 0.0;
    while (!eq.empty() && current_time < sim_time) {
        Event top = eq.top(); eq.pop();
        current_time = top.time;

        switch (top.type) {
            case PACKET_SEND: {
                bool ecn = sw.process_packet(top.sender_id, current_time);
                r.on_packet_arrive(top.sender_id, ecn, current_time, eq);
                se.schedule_next_packet(current_time, eq);
                file << current_time << "," << se.rate() << "," << sw.queue_depth() << "\n";
                break;
            }
            case CNP_ARRIVE:
                se.on_cnp(current_time, eq);
                break;
            case RATE_TIMER:
                if (top.gen == se.timer_gen())
                    se.on_timer(current_time, eq);
                break;
            case BYTE_COUNTER:
                se.on_byte_counter(current_time, eq);
                break;
            default: break;
        }
    }

    file.close();
    std::cout << "single: data.csv written\n";
}

static void run_multi(int n_senders, double duration_ms)
{
    DCQCNParams p;
    Network net(n_senders, p, 40000.0, p.k_min, p.k_max, p.p_max, 12.0 * 1024 * 1024);
    net.run(duration_ms);

    std::string filename = "metrics_" + std::to_string(n_senders) + ".csv";
    net.write_csv(filename);

    Metrics m = net.get_metrics();
    std::cout << "multi n=" << n_senders
              << "\navg_throughput=" << m.avg_throughput
              << "\nvariance=" << m.throughput_variance
              << "\navg_fairness=" << m.avg_fairness
              << "\npfc_count=" << m.pfc_count
              << "\n";
}

int main(int argc, char* argv[])
{
    if (argc < 2 || std::strcmp(argv[1], "single") == 0) {
        double sim_time = (argc >= 2) ? std::atof(argv[2]) : 500.0;
        run_single(sim_time);
        return 0;
    }

    if (std::strcmp(argv[1], "multi") == 0) {
        int n = (argc >= 3) ? std::atoi(argv[2]) : 50;
        double duration = (argc >= 4) ? std::atof(argv[3]) : 1000.0;
        run_multi(n, duration);
        return 0;
    }

    if (std::strcmp(argv[1], "all") == 0) {
        for (int n : {10, 50, 100})
            run_multi(n, 1000.0);
        return 0;
    }

    if (std::strcmp(argv[1], "search") == 0) {
        int n = (argc >= 3) ? std::atoi(argv[2]) : 50;
        double duration = (argc >= 4) ? std::atof(argv[3]) : 1000.0;
        run_search(n, duration);
        return 0;
    }

    std::cerr << "Usage: dcqcn_sim [single | multi <n> <duration_ms> | all | search <n> <duration_ms>]\n";
    return 1;
}
