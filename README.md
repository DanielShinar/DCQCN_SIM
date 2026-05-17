# DCQCN Congestion Control Simulator

A C++ discrete-event simulation of **DCQCN** (Datacenter Quantized Congestion Notification) — the end-to-end congestion control algorithm running inside NVIDIA/Mellanox ConnectX NICs for RoCEv2 RDMA networks.

Built from scratch based on the original paper:
> Zhu et al. *Congestion Control for Large-Scale RDMA Deployments*. ACM SIGCOMM 2015.

---

## What It Does

The simulator implements the complete DCQCN feedback loop across three components:

- **Sender (RP)** — NIC-side rate controller with dynamic alpha, dual rate-increase triggers (timer + byte counter), fast recovery and additive increase phases
- **Switch (CP)** — RED probabilistic ECN marking on an egress queue draining at 40 Gbps
- **Receiver (NP)** — CNP generation with 50 μs rate limiting per flow

All parameters match the paper's deployment values (Table 14):

| Parameter | Value |
|-----------|-------|
| Timer (τ') | 55 μs |
| Byte Counter (B) | 10 MB |
| K_min | 5 KB |
| K_max | 200 KB |
| P_max | 1% |
| g (alpha decay) | 1/256 |
| R_AI | 40 Mbps |
| Link rate | 40 Gbps |

---

## Results

Running 50 senders sharing a single 40 Gbps bottleneck link reproduces the **global synchronization problem** described in the paper: all senders react to congestion simultaneously, causing synchronized rate cuts and recoveries that produce persistent throughput oscillation.

A grid search over `g` (1/512 → 1/8) and timer (10 → 150 μs) using a fitness function derived from the paper's four stated goals (fairness, low oscillation, high utilization, low PFC) found parameters that reduce throughput variance by **45%** at the cost of a 3% drop in average utilization — the stability vs. utilization trade-off the paper describes in Figures 11 and 12.

| Metric | Default | Tuned |
|--------|---------|-------|
| Avg throughput | 36.97 Gbps | 35.80 Gbps |
| Throughput variance | 59.3M | 32.3M |
| Min throughput (p10) | 27.77 Gbps | 29.27 Gbps |
| Jain's fairness | 0.998 | 0.993 |

---

## Build & Run

**Requirements:** C++17, CMake ≥ 3.10

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**Single sender** (validate feedback loop):
```bash
./build/dcqcn_sim single 500
```

**Multi-sender** (observe oscillation):
```bash
./build/dcqcn_sim multi 50 1000
```

**Grid search** (find best parameters):
```bash
./build/dcqcn_sim search 50 1000
```

---

## Project Structure

```
dcqcn_sim/
├── main.cpp               # entry point, run modes, grid search
├── headers/
│   ├── params.h           # DCQCNParams — all algorithm parameters
│   ├── sender.h/          # RP: rate controller (NIC sender side)
│   ├── switch_node.h      # CP: RED/ECN queue (switch ASIC)
│   ├── receiver.h         # NP: CNP generator (NIC receiver side)
│   ├── network.h          # multi-sender simulation harness
│   ├── metrics.h          # throughput, variance, Jain's fairness
│   └── event.h            # discrete event queue
├── source/                # implementations
└── CMakeLists.txt
```

---

## References

- Zhu et al. (2015). [Congestion Control for Large-Scale RDMA Deployments](https://dl.acm.org/doi/10.1145/2785956.2787484). ACM SIGCOMM.
- NVIDIA. [RoCEv2 Congestion Management](https://docs.nvidia.com/networking).
