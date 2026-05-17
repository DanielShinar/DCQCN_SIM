#pragma once

struct DCQCNParams {
    double alpha = 1.0; // initial alpha; updated per eq. (1)/(2)
    double g = 1.0 / 256.0; // alpha decay factor (Table 14)
    double rai_mbps = 40.0; // R_AI additive increase step (Table 2)
    double timer_ms = 0.055; // τ' = 55 μs rate-increase timer (Table 14)
    double byte_counter_mb = 10.0; // B = 10 MB byte counter (Table 14)
    int k_threshold = 5; // F = 5 fast-recovery steps (Table 2)
    double k_min = 5.0 * 1024.0; // K_min = 5 KB (Table 14)
    double k_max = 200.0 * 1024.0; // K_max = 200 KB (Table 14)
    double p_max = 0.01; // P_max = 1% (Table 14)
    double line_rate = 40000.0; // 40 Gbps in Mbps
};
