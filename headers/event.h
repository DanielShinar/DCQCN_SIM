#pragma once

#include <queue>
#include <vector>

enum EventType {
  PACKET_SEND,
  PACKET_ARRIVE,
  CNP_SEND,
  CNP_ARRIVE,
  RATE_TIMER,
  BYTE_COUNTER,
  SAMPLE_METRICS
};

struct Event {
    double time;
    EventType type;
    int sender_id;
    int gen = 0;
};

struct EventComparator {
    bool operator()(const Event& a, const Event& b) {
        return a.time > b.time;
    }
};

using EventQueue = std::priority_queue<Event, std::vector<Event>, EventComparator>;
