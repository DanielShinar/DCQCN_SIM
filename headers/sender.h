#pragma once
#include "params.h"
#include "event.h"

class Sender {
public:
    Sender(int id, DCQCNParams p);

    void on_cnp(double time, EventQueue& eq);
    void on_timer(double time, EventQueue& eq);
    void on_byte_counter(double time, EventQueue& eq);
    void schedule_next_packet(double time, EventQueue& eq);

    double rate() const;
    int id() const;
    int timer_gen() const;

private:
    void do_rate_increase();

    int _id;
    double _current_rate;
    double _target_rate;
    double _alpha;
    int _T;
    int _BC;
    double _bytes_since_bc;
    int _timer_gen;
    DCQCNParams _params;
};
