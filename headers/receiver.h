#pragma once

#include <map>
#include "event.h"

class Receiver {
public:
    Receiver();
    void on_packet_arrive(int sender_id, bool ecn_marked, double time, EventQueue& eq);

private:
    int _cnp_count;
    std::map<int, double> _last_cnp_time;
    static constexpr double CNP_INTERVAL_MS = 0.05;
};