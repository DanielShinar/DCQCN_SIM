#include "sender.h"
#include <algorithm>

Sender::Sender(int id, DCQCNParams p)
    : _id(id), _params(p),
      _current_rate(p.line_rate),
      _target_rate(p.line_rate),
      _alpha(p.alpha),
      _T(0), _BC(0),
      _bytes_since_bc(0.0),
      _timer_gen(0)
{}

void Sender::on_cnp(double time, EventQueue &eq)
{
    _target_rate = _current_rate;
    _alpha = (1.0 - _params.g) * _alpha + _params.g;
    _current_rate = std::max(1.0, _current_rate * (1.0 - _alpha / 2.0));
    _T = 0;
    _BC = 0;
    _bytes_since_bc = 0.0;
    _timer_gen++;

    Event e;
    e.time = time + _params.timer_ms;
    e.type = RATE_TIMER;
    e.sender_id = _id;
    e.gen = _timer_gen;
    eq.push(e);
}

void Sender::do_rate_increase()
{
    if (std::max(_T, _BC) < _params.k_threshold) {
        _current_rate = (_current_rate + _target_rate) / 2.0;
    } else {
        _target_rate  = std::min(_target_rate + _params.rai_mbps, _params.line_rate);
        _current_rate = (_current_rate + _target_rate) / 2.0;
    }
    _current_rate = std::min(_current_rate, _params.line_rate);
}

void Sender::on_timer(double time, EventQueue &eq)
{
    _alpha = (1.0 - _params.g) * _alpha;
    _T++;
    do_rate_increase();

    if (_current_rate < _params.line_rate) {
        Event e;
        e.time = time + _params.timer_ms;
        e.type = RATE_TIMER;
        e.sender_id = _id;
        e.gen = _timer_gen;
        eq.push(e);
    }
}

void Sender::on_byte_counter(double /*time*/, EventQueue& /*eq*/)
{
    _BC++;
    do_rate_increase();
}

void Sender::schedule_next_packet(double time, EventQueue& eq)
{
    double packet_bytes = 1500.0;
    double rate_bps = _current_rate * 1e6;
    double gap_ms = (packet_bytes * 8.0 / rate_bps) * 1000.0;

    _bytes_since_bc += packet_bytes;
    if (_bytes_since_bc >= _params.byte_counter_mb * 1e6) {
        _bytes_since_bc = 0.0;
        Event bc;
        bc.time = time;
        bc.type = BYTE_COUNTER;
        bc.sender_id = _id;
        eq.push(bc);
    }

    Event e;
    e.time = time + gap_ms;
    e.type = PACKET_SEND;
    e.sender_id = _id;
    eq.push(e);
}

double Sender::rate() const { return _current_rate; }
int Sender::id() const { return _id; }
int Sender::timer_gen() const { return _timer_gen; }
