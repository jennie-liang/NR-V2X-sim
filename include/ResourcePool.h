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

    std::vector<Resource> candidates(int start_slot, int end_slot, int size_subch) const;

private:
    int num_subchannels_;
};
