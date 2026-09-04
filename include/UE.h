#pragma once

#include <vector>
#include <random>
#include "Config.h"
#include "Resource.h"
#include "ResourcePool.h"

// What a UE observed on one resource during sensing.
// Used in v0.3 to exclude resources and rank them by RSRP.
struct SensingRecord {
    int      slot        = -1;
    int      subch_start = -1;
    int      num_subch   = 0;
    double   rsrp_dbm    = -200.0;
    int      period_ms   = 0;   // reservation period signalled in SCI
};

class UE {
public:
    UE(int id, double position_m, double speed_mps, const Config& cfg);

    int    id() const { return id_; }
    double position() const { return position_m_; }

    // Advance position by one slot. Wraps around at the end of the road
    // so UE density stays constant.
    //
    // TODO(v0.1): implement.
    void move(double slot_duration_ms, double road_length_m);

    // Does this UE have a packet to send in this slot?
    // With periodic CAM traffic, a packet is generated every packet_period_ms.
    //
    // TODO(v0.1): implement.
    bool hasPacketAt(int slot, const Config& cfg) const;

    // Pick the resource to transmit on.
    // v0.1: uniformly random from all candidates in the selection window. DONE
    // v0.3: hold the resource across several periods (SPS).
    // v0.4: filter candidates using the sensing window before picking.
    Resource selectResource(int current_slot,
                            const ResourcePool& pool,
                            const Config& cfg,
                            std::mt19937& rng);

    // The reservation period this UE currently announces in its SCI, in slots.
    // 0 when SPS is disabled or the UE holds no grant, meaning the transmission
    // is one-shot and tells listeners nothing about the future.
    int reservationPeriod() const { return reservation_period_; };

    // Record something this UE heard, for later use by sensing-based selection.
    // TODO(v0.3)
    void addSensingRecord(const SensingRecord& rec, int current_slot, const Config& cfg);

    int reselCounter() const { return resel_counter_; }

private:
    int    id_;
    double position_m_;
    double speed_mps_;

    // ---- SPS state (v0.3) ----
    // The grant this UE is currently holding. Its slot is advanced by one
    // reservation period after every transmission, so it always points at the
    // next slot this UE intends to use.
    Resource current_resource_;
    int      resel_counter_      = 0;
    int      reservation_period_ = 0;

    std::vector<SensingRecord> sensing_history_;
};
