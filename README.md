# NR-V2X Mode 2 Resource Scheduler Simulator

A system-level simulator for 3GPP NR sidelink Mode 2 autonomous resource
selection, written in C++17 with Python analysis scripts.

In Mode 2 there is no gNB scheduling sidelink resources — each UE senses the
channel and picks its own time-frequency resource. This simulator implements
that selection procedure and measures how well it avoids collisions as vehicle
density grows.

## Status

| Version | Feature | Done |
|---|---|---|
| v0.1 | Resource pool, random selection, collision detection | ☐ |
| v0.2 | CSV output, Python plotting | ☐ |
| v0.3 | Sensing window + TS 38.214 §8.1.4 resource selection | ☐ |
| v0.4 | Semi-persistent scheduling (reselection counter) | ☐ |
| v0.5 | Pathloss + SINR-based reception, PRR vs distance | ☐ |
| v1.0 | Parameter sensitivity analysis, full README | ☐ |

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run

```bash
./build/v2xsim --ues 50 --duration 10000
./build/v2xsim --ues 50 --sensing            # sensing-based selection
./build/v2xsim --ues 50 --sensing --sps      # with SPS
```

Sweep UE density and plot:

```bash
bash scripts/sweep.sh
```

## Architecture

```
Config          all simulation parameters in one struct
Resource        a (slot, subchannel_start, num_subchannels) block
ResourcePool    the time-frequency grid; enumerates candidate resources
UE              position, mobility, traffic generation, resource selection
SensingWindow   per-UE history of observed transmissions and RSRP
Channel         log-distance pathloss, dBm/mW conversion
Simulator       main slot loop, collision detection, metrics
```

The simulator runs one slot at a time. Each UE that has a packet selects a
resource in a future slot within the selection window `[n+T1, n+T2]`, so
transmissions are recorded into that future slot's bucket. When the simulation
reaches that slot, overlapping transmissions are evaluated for collision.

## Key parameters

Defaults follow 3GPP TS 38.214 where applicable.

| Parameter | Default | Meaning |
|---|---|---|
| `sensing_window_ms` | 1100 | How far back a UE remembers observations (T0) |
| `selection_start_ms` | 1 | Earliest slot a UE may select (T1) |
| `selection_end_ms` | 100 | Latest slot, bounded by packet delay budget (T2) |
| `rsrp_threshold_dbm` | -110 | Resources above this are excluded |
| `candidate_ratio` | 0.2 | Keep the best 20% of remaining resources |
| `prob_resource_keep` | 0.4 | Probability of keeping a resource after the counter expires |

## Simplifications

This is a system-level simulator focused on the MAC-layer selection procedure.
Deliberately out of scope:

- No fast fading — pathloss is distance-only
- No PHY processing — reception is an SINR threshold test
- Straight-road mobility at constant speed
- Broadcast traffic only; no unicast, groupcast, or HARQ feedback

## References

- 3GPP TS 38.214 §8.1.4 — UE procedure for determining the subset of resources
- 3GPP TS 38.321 — MAC protocol specification for sidelink
