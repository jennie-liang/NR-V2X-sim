#include "ResourcePool.h"

ResourcePool::ResourcePool(const Config& cfg)
    : num_subchannels_(cfg.num_subchannels) {}

std::vector<Resource> ResourcePool::candidates(int start_slot,
                                               int end_slot,
                                               int size_subch) const {
    std::vector<Resource> out;

    // TODO(v0.1)
    // Enumerate every (slot, subchannel_start) pair where a block of
    // `size_subch` contiguous subchannels fits.
    //
    // Hint: reserve() the expected size first — it's a hot path.
    //   expected = (end_slot - start_slot + 1) * (num_subchannels_ - size_subch + 1)

    (void)start_slot;
    (void)end_slot;
    (void)size_subch;
    return out;
}
