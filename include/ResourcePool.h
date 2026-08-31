#pragma once

#include <vector>
#include <random>
#include "Config.h"
#include "Resource.h"

// The time-frequency grid UEs select resources from.
//
// This class does NOT store occupancy history — the Simulator owns that.
// ResourcePool is responsible for describing the grid and enumerating
// candidate resources inside a selection window.
class ResourcePool {
public:
    explicit ResourcePool(const Config& cfg);

    int numSubchannels() const { return num_subchannels_; }

    // Every candidate resource of size `size_subch` in slots [start_slot, end_slot].
    // Used both by random selection (v0.1) and by sensing-based selection (v0.3),
    // which filters this list down.
    //
    // TODO(v0.1): implement.
    //   for each slot in [start_slot, end_slot]:
    //     for each subchannel_start in [0, num_subchannels - size_subch]:
    //       push Resource{slot, subchannel_start, size_subch}
    std::vector<Resource> candidates(int start_slot, int end_slot, int size_subch) const;

private:
    int num_subchannels_;
};
