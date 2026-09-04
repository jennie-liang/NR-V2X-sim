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

    position_m_ += speed_mps_ * (slot_duration_ms / 1000.0);
    position_m_ = position_m_ > road_length_m ? (position_m_ - road_length_m) : position_m_;
}

bool UE::hasPacketAt(int slot, const Config& cfg) const {
    // TODO(v0.1)
    // Periodic traffic: a packet every packet_period_ms.
    // Stagger UEs so they don't all transmit in the same slot —
    // offset by (id_ * period / num_ues) or just (id_ % period).

    int period = cfg.packet_period_ms;
    if(id_ % period == slot% period)
        return true;

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

    //window
    int t1 = cfg.selection_start_ms;
    int t2 = cfg.selection_end_ms;
    int LsubCH = cfg.subchannels_per_pkt;
    
    //random selection
    if(!cfg.enable_sps){
        //Candidate
        std::vector<Resource> resources = pool.candidates(current_slot+t1, current_slot+t2, LsubCH);
        if(resources.empty()) return Resource{};
        
        //pick one resource and return
        std::uniform_int_distribution<size_t> dist(0, resources.size()-1);
        return resources[dist(rng)];
    }

    // TODO(v0.3) — semi-persistent scheduling
    //
    // Wrap the random selection above in a grant-holding state machine.
    // Reference: TS 38.321 section 5.22.1.2.
    //
    //   if (!cfg.enable_sps) -> current behaviour, nothing to change
    //
    //   Case A: holding a valid grant (resel_counter_ > 0)
    //       current_resource_.slot += reservation_period_;
    //       --resel_counter_;
    //       return current_resource_;
    //
    //   Case B: counter has expired (resel_counter_ == 0)
    //       With probability cfg.prob_resource_keep, keep the same
    //       subchannels: reset the counter and fall into Case A.
    //       Otherwise select a fresh resource as below, then:
    //           reservation_period_ = packet period expressed in slots
    //           resel_counter_ = uniform int in
    //               [cfg.resel_counter_min, cfg.resel_counter_max]
    //
    // Note the reservation period is what makes this UE predictable to
    // others. Simulator must copy reservationPeriod() into the Transmission
    // it records, otherwise sensing in v0.4 has nothing to read.
    //
    // Sanity check after implementing: total_transmissions must not change,
    // since SPS alters *which* resource is used, not how many packets are
    // sent. Collision rate should drop somewhat even without sensing, because
    // a UE that keeps a working resource stops re-rolling into occupied ones.

    std::uniform_int_distribution<size_t> dist_rc(cfg.resel_counter_min, cfg.resel_counter_max);

    //SPS scheduling
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

    //Candidate
    std::vector<Resource> resources = pool.candidates(current_slot+t1, current_slot+t2, LsubCH);
    if(resources.empty()) return Resource{};
    
    //pick one resource and return
    std::uniform_int_distribution<size_t> dist(0, resources.size()-1);
    current_resource_ = resources[dist(rng)];
    resel_counter_ = dist_rc(rng);
    reservation_period_ = cfg.packet_period_ms;

    SensingRecord sr = SensingRecord{current_resource_.slot, current_resource_.subchannel_start, current_resource_.num_subchannels, -200, reservation_period_};
    addSensingRecord(sr, current_slot, cfg);
    return  current_resource_;


    // TODO(v0.4) — sensing-based selection
    //   Between building `resources` and picking one, filter it:
    //     a. drop slots this UE could not sense because it was transmitting
    //     b. for each sensed transmission with reservation_period P observed
    //        at slot t, mark every future slot m where (m - t) % P == 0 as
    //        occupied; exclude those candidates when their RSRP exceeds
    //        cfg.rsrp_threshold_dbm
    //     c. if fewer than cfg.candidate_ratio * total candidates survive,
    //        raise the threshold by 3 dB and repeat from (b)
    //     d. rank survivors by average received power over the sensing window
    //        and keep the quietest 20%, then pick uniformly from those
}

void UE::addSensingRecord(const SensingRecord& rec, int current_slot, const Config& cfg) {
    // TODO(v0.3)
    // Push the record, then drop anything older than sensing_window_ms.
    sensing_history_.push_back(rec);

    const int cutoff = current_slot - cfg.sensing_window_ms;
    auto first_valid = std::find_if(
                            sensing_history_.begin(), 
                            sensing_history_.end(), 
                            [cutoff](const SensingRecord& r){return r.slot > cutoff;});
    sensing_history_.erase(sensing_history_.begin(), first_valid);
}
