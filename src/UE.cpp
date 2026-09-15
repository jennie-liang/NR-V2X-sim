#include "UE.h"
#include <algorithm>

UE::UE(int id, double position_m, double speed_mps, const Config& cfg)
    : id_(id), position_m_(position_m), speed_mps_(speed_mps) {
    (void)cfg;
}

void UE::move(double slot_duration_ms, double road_length_m) {
    position_m_ += speed_mps_ * (slot_duration_ms / 1000.0);
    position_m_ = position_m_ > road_length_m ? (position_m_ - road_length_m) : position_m_;
}

bool UE::hasPacketAt(int slot, const Config& cfg) const {
    int period = cfg.packet_period_ms;
    if(id_ % period == slot% period)
        return true;

    return false;
}

Resource UE::selectResource(int current_slot,
                            const ResourcePool& pool,
                            const Config& cfg,
                            std::mt19937& rng) {

    //window
    int start_slot = cfg.selection_start_ms + current_slot;
    int end_slot = cfg.selection_end_ms + current_slot;
    int LsubCH = cfg.subchannels_per_pkt;
    
    //random selection
    if(!cfg.enable_sps){
        //Candidate
        std::vector<Resource> resources = pool.candidates(start_slot, end_slot, LsubCH);
        if(resources.empty()) return Resource{};
        
        //pick one resource and return
        std::uniform_int_distribution<int> dist(0, resources.size()-1);
        return resources[dist(rng)];
    }

    //if counter is not 0 ==> using the same resource
    //semi-persistent scheduling 
    std::uniform_int_distribution<int> dist_rc(cfg.resel_counter_min, cfg.resel_counter_max);
    if(resel_counter_){
        current_resource_.slot += reservation_period_;
        resel_counter_--;

        if(resel_counter_ == 0){
            std::uniform_real_distribution<double> dist_prk(0, 1);
            if(dist_prk(rng)<cfg.prob_resource_keep){
                resel_counter_ = dist_rc(rng);
            }
        }
        return current_resource_;
    }

    //Resource reselection -- Channel sensing
    std::vector<Resource> resources = channelSensing(current_slot, cfg);
    if(resources.size() == 0)  resources = pool.candidates(start_slot, end_slot, LsubCH);

    //pick one resource and return
    std::uniform_int_distribution<int> dist(0, resources.size()-1);
    current_resource_ = resources[dist(rng)];
    resel_counter_ = dist_rc(rng);
    reservation_period_ = cfg.packet_period_ms;

    return  current_resource_;
}

std::vector<Resource> UE::channelSensing(int current_slot, const Config& cfg){
    // sensing-based selection, TS 38.214 section 8.1.4
    
    //selection window
    int start_slot = cfg.selection_start_ms + current_slot; //t1
    int end_slot = cfg.selection_end_ms + current_slot;     //t2
    int LsubCH = cfg.subchannels_per_pkt;

    int window = end_slot - start_slot + 1;
    double rsrp_threshold = cfg.rsrp_threshold_dbm;
    int resource_size = window * (cfg.num_subchannels - LsubCH + 1);

    std::vector<Resource> survivors;

    while(survivors.size() < resource_size * cfg.candidate_ratio && rsrp_threshold < 0.0){          //if the number of candidates is not enough, increase the rsrp threshold and re-do the sensing process
        std::vector<bool> occupied(window*cfg.num_subchannels, false);

        //exclude the resources occupied according to received SCI
        for(auto& rc: sensing_history_){
            if(rc.period_ms == 0) continue;
            if(rc.rsrp_dbm < rsrp_threshold) continue;

            for(int m=rc.slot+rc.period_ms; m<=end_slot; m+=rc.period_ms){
                if(m<start_slot) continue;

                int idx =(m-start_slot)*cfg.num_subchannels +rc.subch_start;
                for(int i=idx; i<idx+rc.num_subch;i++) occupied[i] = true; 
            }
        }

        //exclude the resources possibly occupeied according to the unmonitored slot
        for(auto& s: unmonitored_slot_){
            for(int m=s+cfg.packet_period_ms; m<=end_slot; m+=cfg.packet_period_ms){
                if(m<start_slot) continue;

                int idx =(m-start_slot)*cfg.num_subchannels;
                for(int i=idx; i<idx+cfg.num_subchannels;i++) occupied[i] = true; 
            }
        }

        //Collect available resources
        bool available = true;
        survivors.clear();
        for(int s=0; s<end_slot-start_slot+1; s++){
            for(int ch=0; ch<=cfg.num_subchannels-LsubCH; ch++){
                int idx = s * cfg.num_subchannels + ch;
                available = true;
                for(int i=idx; i<idx+LsubCH; i++){
                    if(occupied[i] == true) {
                        available = false;
                        break;
                    }
                }
                if(available) survivors.push_back(Resource{s+start_slot, ch , LsubCH});
            }
        }
        rsrp_threshold += 3;
    }
    return survivors;
}

void UE::addSensingRecord(const SensingRecord& rec, int current_slot, const Config& cfg) {
    // Push the record, then drop anything older than sensing_window_ms.
    sensing_history_.push_back(rec);

    const int cutoff = current_slot - cfg.sensing_window_ms;
    auto first_valid = std::find_if(
                            sensing_history_.begin(), 
                            sensing_history_.end(), 
                            [cutoff](const SensingRecord& r){return r.slot > cutoff;});
    sensing_history_.erase(sensing_history_.begin(), first_valid);
}

void UE::markUnmonitored(int slot, const Config& cfg){
    // Push the unmonitored slot, then drop anything older than sensing_window_ms.
    unmonitored_slot_.push_back(slot);

    const int cutoff = slot - cfg.sensing_window_ms;
    auto first_valid = std::find_if(unmonitored_slot_.begin(), unmonitored_slot_.end(),
                                    [cutoff](const int s){return s > cutoff;});
    unmonitored_slot_.erase(unmonitored_slot_.begin(), first_valid);

}
