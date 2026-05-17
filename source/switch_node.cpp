#include "switch_node.h"
#include <algorithm>
#include <cstdlib>

Switch::Switch(double bw_mbps, double k_min, double k_max, double p_max, double max_buffer)
    : _bw_mbps(bw_mbps),
      _k_min(k_min), _k_max(k_max), _p_max(p_max),
      _max_buffer(max_buffer),
      _queue_depth(0.0),
      _pfc_count(0),
      _last_update_time(0.0)
{}

bool Switch::process_packet(int /*sender_id*/, double time)
{
    double packet_bytes = 1500.0;
    double elapsed_ms = time - _last_update_time;
    double drain_bytes = (_bw_mbps * 1e6 / 8.0) * (elapsed_ms / 1000.0);

    _queue_depth = std::max(0.0, _queue_depth - drain_bytes);
    _last_update_time = time;
    _queue_depth += packet_bytes;

    if (_queue_depth >= _max_buffer)
        _pfc_count++;

    double p = 0.0;
    if (_queue_depth > _k_max)
        p = 1.0;
    else if (_queue_depth > _k_min)
        p = _p_max * (_queue_depth - _k_min) / (_k_max - _k_min);

    return p > 0.0 && (static_cast<double>(rand()) / RAND_MAX) < p;
}

int Switch::pfc_count() const { return _pfc_count; }
double Switch::queue_depth() const { return _queue_depth; }
