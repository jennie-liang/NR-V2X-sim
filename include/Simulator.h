#pragma once

#include <vector>
#include <random>
#include <string>
#include "Config.h"
#include "Resource.h"
#include "ResourcePool.h"
#include "UE.h"

struct Metrics {
    long total_transmissions = 0;
    long collisions          = 0;   // TX whose resource overlapped another TX

    // PRR accounting: for every TX, every RX inside comm_range counts as one
    // "opportunity"; it succeeds if SINR is above threshold.
    long rx_opportunities    = 0;
    long rx_success          = 0;

    double collisionRate() const {
        return total_transmissions ? double(collisions) / total_transmissions : 0.0;
    }
    double prr() const {
        return rx_opportunities ? double(rx_success) / rx_opportunities : 0.0;
    }
};

class Simulator {
public:
    explicit Simulator(const Config& cfg);

    void run();
    const Metrics& metrics() const { return metrics_; }

    // Append one row per run to the CSV so multiple runs accumulate.
    void writeCsv(const std::string& path) const;

private:
    // Main loop, one slot at a time.
    // TODO(v0.1):
    //   1. move all UEs
    //   2. for each UE with a packet, selectResource() and record a Transmission
    //   3. detect collisions among this slot's transmissions
    //   4. update metrics
    void stepSlot(int slot);

    // Any two transmissions overlapping in time+frequency collide.
    // v0.1: pure overlap counts as a collision.
    // v0.5: refine using SINR — a strong signal can survive a weak interferer.
    // TODO(v0.1)
    void detectCollisions(const std::vector<Transmission>& txs);

    Config       cfg_;
    ResourcePool pool_;
    std::vector<UE> ues_;
    std::mt19937 rng_;
    Metrics      metrics_;

    // Transmission history, indexed by slot. Needed for sensing in v0.3;
    // for v0.1 only the current slot matters.
    std::vector<std::vector<Transmission>> tx_history_;
};
