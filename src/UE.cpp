#include "UE.h"
#include <algorithm>

UE::UE(int id, double position_m, double speed_mps, const Config& cfg)
    : id_(id), position_m_(position_m), speed_mps_(speed_mps) {
    (void)cfg;
}

void UE::move(double slot_duration_ms, double road_length_m) {
    // TODO(v0.1)
    // position += speed_mps * (slot_duration_ms / 1000.0)
    // if position > road_length: wrap to position - road_length
    (void)slot_duration_ms;
    (void)road_length_m;
}

bool UE::hasPacketAt(int slot, const Config& cfg) const {
    // TODO(v0.1)
    // Periodic traffic: a packet every packet_period_ms.
    // Stagger UEs so they don't all transmit in the same slot —
    // offset by (id_ * period / num_ues) or just (id_ % period).
    (void)slot;
    (void)cfg;
    return false;
}

Resource UE::selectResource(int current_slot,
                            const ResourcePool& pool,
                            const Config& cfg,
                            std::mt19937& rng) {
    // TODO(v0.1) — random selection
    //   1. window = [current_slot + T1, current_slot + T2]
    //   2. cands = pool.candidates(window, cfg.subchannels_per_pkt)
    //   3. pick one uniformly at random
    //
    // TODO(v0.3) — sensing-based selection
    //   Between steps 2 and 3, filter cands:
    //     a. exclude resources this UE could not sense (it was transmitting)
    //     b. exclude resources reserved by others whose RSRP > threshold
    //     c. if fewer than candidate_ratio * total remain, raise the
    //        threshold by 3 dB and repeat
    //     d. rank the survivors by average RSSI, keep the best 20%
    //
    // TODO(v0.4) — SPS
    //   Only run selection when resel_counter_ == 0; otherwise reuse
    //   current_resource_ shifted forward by one packet period.

    (void)current_slot;
    (void)pool;
    (void)cfg;
    (void)rng;
    return Resource{};
}

void UE::addSensingRecord(const SensingRecord& rec, int current_slot, const Config& cfg) {
    // TODO(v0.3)
    // Push the record, then drop anything older than sensing_window_ms.
    (void)rec;
    (void)current_slot;
    (void)cfg;
}
