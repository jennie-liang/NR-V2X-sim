#pragma once

// A sidelink resource: a contiguous block of subchannels within one slot.
struct Resource {
    int slot            = -1;   // absolute slot index
    int subchannel_start = -1;
    int num_subchannels  = 0;

    bool valid() const { return slot >= 0 && subchannel_start >= 0 && num_subchannels > 0; }

    // Does this resource overlap another in both time and frequency?
    bool overlaps(const Resource& other) const {
        if (slot != other.slot) return false;
        int a_end = subchannel_start + num_subchannels;
        int b_end = other.subchannel_start + other.num_subchannels;
        return subchannel_start < b_end && other.subchannel_start < a_end;
    }
};

// One actual transmission on the medium. The simulator records these per slot
// so it can detect collisions and so UEs can build their sensing history.
struct Transmission {
    int      ue_id      = -1;
    Resource resource;
    double   tx_pos_m   = 0.0;
    double   tx_power_dbm = 0.0;
};
