#include "network.h"

// --- Network constructor -------------------------------------------------

// Goal: set up N senders, one switch, one receiver, and seed the event queue

Network::Network(int n_senders, DCQCNParams p,
                 double bw_mbps, double k_min, double k_max, double p_max, double max_buf)
    : _sw(bw_mbps, k_min, k_max, p_max, max_buf)
{   
    Event e;
    for (int i = 0; i<n_senders; i++)
    {
        _senders.emplace_back(i, p);
        e.time = 0.0;
        e.sender_id = i;
        e.type = PACKET_SEND;
        _eq.push(e);
    }
    e.time=SAMPLE_INTERVAL_MS;
    e.sender_id = 0;
    e.type=SAMPLE_METRICS;
    _eq.push(e);
}


void Network::run(double duration_ms)
{
    double current_time = 0.0;
    while (!_eq.empty() && current_time < duration_ms)
        {
            Event top = _eq.top();
            _eq.pop();
            current_time = top.time;

            switch (top.type) {
                case PACKET_SEND: {
                    bool ecn = _sw.process_packet(top.sender_id, current_time);
                    _receiver.on_packet_arrive(top.sender_id, ecn, current_time, _eq);
                    _senders[top.sender_id].schedule_next_packet(current_time, _eq);
                    break;
                }
                case CNP_ARRIVE:
                    _senders[top.sender_id].on_cnp(current_time, _eq);
                    break;
                case RATE_TIMER:
                    if (top.gen == _senders[top.sender_id].timer_gen())
                        _senders[top.sender_id].on_timer(current_time, _eq);
                    break;
                case BYTE_COUNTER:
                    _senders[top.sender_id].on_byte_counter(current_time, _eq);
                    break;
                case SAMPLE_METRICS: {
                    _metrics.sample(current_time, _senders, _sw);
                    Event e;
                    e.time=current_time + SAMPLE_INTERVAL_MS;
                    e.sender_id = 0;
                    e.type=SAMPLE_METRICS;
                    _eq.push(e);
                    break;
                }
                default:
                    break;
                }
        }
}


void Network::write_csv(const std::string& filename) const
{
    _metrics.write_csv(filename);
}


Metrics Network::get_metrics() const
{
    auto m = _metrics.summary();
    m.pfc_count = _sw.pfc_count();
    return m;
}
