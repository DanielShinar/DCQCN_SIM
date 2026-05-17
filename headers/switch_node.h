#pragma once

class Switch {
public:
    Switch(double bw_mbps, double k_min, double k_max, double p_max, double max_buffer);

    bool process_packet(int sender_id, double time);
    int pfc_count()   const;
    double queue_depth() const;

private:
    double _bw_mbps;
    double _k_min;
    double _k_max;
    double _p_max;
    double _max_buffer;
    double _queue_depth;
    int _pfc_count;
    double _last_update_time;
};
