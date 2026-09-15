#include "ResourcePool.h"
#include <iostream>

ResourcePool::ResourcePool(const Config& cfg)
    : num_subchannels_(cfg.num_subchannels) {}

std::vector<Resource> ResourcePool::candidates(int start_slot,
                                               int end_slot,
                                               int size_subch) const {
    std::vector<Resource> out;

    // Enumerate every (slot, subchannel_start) pair where a block of `size_subch` contiguous subchannels fits.
    for(int s=start_slot; s<=end_slot; s++){
        for(int c=0; c<=num_subchannels_-size_subch; c++){
            out.push_back(Resource{s, c, size_subch});
        }
    }
    
    //check the size
    if(int(out.size())!=(end_slot-start_slot+1)*(num_subchannels_-size_subch+1)){
        std::cout<<"Check the resource pool function!"<<std::endl;
    }

    return out;
}
