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

Requires a C++17 compiler. No external dependencies.

```bash
make            # release build
make debug      # -O0 -g with address and UB sanitizers
make clean
```

## Run

```bash
./v2xsim --ues 50 --duration 10000
./v2xsim --ues 50 --sensing            # sensing-based selection
./v2xsim --ues 50 --sensing --sps      # with SPS
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

## Validating results

Simulation output is only meaningful if you can explain it. Two checks are used
here: a link budget that predicts what the numbers *should* be, and a set of
invariants that must hold regardless of parameters.

### Link budget

With the default configuration, noise is never the limiting factor. At the edge
of the communication range:

```
PL(300 m) = 47.86 + 20·log₁₀(300)  = 97.40 dB
Rx power  = 23 − 97.40             = −74.40 dBm
SNR       = −74.40 − (−95)         = 20.60 dB
Threshold =                           3.00 dB
```

The margin at the worst-case distance is 17.6 dB, so a packet can only fail
when an interferer is present. With a pathloss exponent of 2.0 the SINR between
two transmitters reduces to a distance ratio:

```
SINR ≈ 20·log₁₀(d_interferer / d_signal)
```

Falling below the 3 dB threshold therefore requires the interferer to be within
1.41× the distance of the wanted transmitter, which happens in roughly half of
the collision events. That predicts:

```
PRR ≈ 1 − (collision_rate × 0.5)
```

At 50 UEs the measured collision rate is 14.4%, so the predicted PRR is ~92.8%
against a measured 93.3%. The model is internally consistent.

### Invariants

These must hold for any parameter set. If one breaks, there is a bug.

| Check | Expected |
|---|---|
| `total_transmissions` | ≈ `num_ues × sim_duration_ms / packet_period_ms` |
| Same seed, two runs | Byte-identical output |
| UE count ↑ | Collision rate ↑, PRR ↓, monotonically |
| Very low UE count (≤5) | Collision rate ≈ 0, PRR ≈ 100% |
| Sensing enabled (v0.3) | Collision rate strictly lower than random selection |
| `collisions` | ≤ `total_transmissions` |
| `rx_success` | ≤ `rx_opportunities` |

Note that PRR is higher than `1 − collision_rate` would suggest. This is
expected: a resource collision does not always destroy reception, because a
nearby transmitter can be decoded despite a distant interferer sharing the same
resource. This is the capture effect, and its presence is a sign the SINR model
is doing something more meaningful than a binary overlap test.

## Simplifications

This is a system-level simulator focused on the MAC-layer selection procedure.
Deliberately out of scope:

- No fast fading — pathloss is distance-only
- No shadow fading — see calibration notes below
- No PHY processing — reception is an SINR threshold test
- Straight-road mobility at constant speed
- Broadcast traffic only; no unicast, groupcast, or HARQ feedback
- Any resource overlap is treated as full-band interference; partial overlap is
  not weighted by the fraction of shared subchannels

## Calibration notes

The v0.1 defaults are deliberately optimistic so that the MAC-layer behaviour is
visible without being masked by channel effects. Published NR-V2X studies report
PRR in the 70–90% range at comparable densities; this simulator sits above that
because of the following, listed in order of impact.

| Change | Current | More realistic | Effect |
|---|---|---|---|
| Shadow fading | none | log-normal, σ = 3–8 dB | Largest gap. Deep fades cause failures that a deterministic pathloss model never produces. |
| `pathloss_exponent` | 2.0 (free space) | 2.7–3.0 highway LOS | Faster decay with distance; edge-of-range reception degrades. |
| `comm_range_m` | 300 | 500–1000 | Extends the accounting to distances where the link genuinely fails. |
| `sinr_threshold_db` | 3.0 | 5–10 for higher MCS | 3 dB corresponds to a low-rate QPSK operating point. |

These are intentionally deferred. Shadow fading is planned for v0.5, where an
A/B comparison with and without it becomes an analysis result in its own right
rather than just a parameter change.

## References

- 3GPP TS 38.214 §8.1.4 — UE procedure for determining the subset of resources
- 3GPP TS 38.321 — MAC protocol specification for sidelink
