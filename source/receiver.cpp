#include "receiver.h"

Receiver::Receiver()
    : _cnp_count(0), _last_cnp_time({{0, -CNP_INTERVAL_MS}})
{}

void Receiver::on_packet_arrive(int sender_id, bool ecn_marked, double time, EventQueue &eq)
{
    if (ecn_marked && (time - _last_cnp_time[sender_id]) >= CNP_INTERVAL_MS)
    {
        Event e;
        e.time = time + 0.05;
        e.type = CNP_ARRIVE;
        e.sender_id = sender_id;
        eq.push(e);
        _cnp_count++;
        _last_cnp_time[sender_id] = time;
    }
}
