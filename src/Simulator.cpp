#include "Simulator.h"
#include "Channel.h"

#include <fstream>
#include <iostream>
#include <cmath>

Simulator::Simulator(const Config& cfg)
    : cfg_(cfg), pool_(cfg), rng_(cfg.seed) {

    // Place UEs uniformly along the road with random speeds.
    std::uniform_real_distribution<double> pos_dist(0.0, cfg.road_length_m);
    std::uniform_real_distribution<double> spd_dist(cfg.speed_min_mps, cfg.speed_max_mps);

    ues_.reserve(cfg.num_ues);
    for (int i = 0; i < cfg.num_ues; ++i) {
        ues_.emplace_back(i, pos_dist(rng_), spd_dist(rng_), cfg);
    }

    // One bucket per slot. Selection can reach up to T2 slots ahead,
    // so allocate room for that overshoot.
    tx_history_.resize(cfg.sim_duration_ms + cfg.selection_end_ms + 2);
}

void Simulator::run() {
    for (int slot = 0; slot < cfg_.sim_duration_ms; ++slot) {
        stepSlot(slot);
    }

    std::cout << "=== NR-V2X Mode 2 Simulation ===\n"
              << "UEs                : " << cfg_.num_ues << "\n"
              << "Duration           : " << cfg_.sim_duration_ms << " ms\n"
              << "SPS                : " << (cfg_.enable_sps ? "on" : "off") << "\n"
              << "--------------------------------\n"
              << "Transmissions      : " << metrics_.total_transmissions << "\n"
              << "Collisions         : " << metrics_.collisions << "\n"
              << "Collision rate     : " << metrics_.collisionRate() * 100.0 << " %\n"
              << "PRR                : " << metrics_.prr() * 100.0 << " %\n";
}

void Simulator::stepSlot(int slot) {
    //Move UE
    for(auto &ue: ues_)
        ue.move(1, cfg_.road_length_m);

    //UE packet check and send
    for(auto &ue: ues_){
        if(ue.hasPacketAt(slot, cfg_)){
            Resource r = ue.selectResource(slot, pool_, cfg_, rng_);
            if(r.valid())
            tx_history_[r.slot].push_back(Transmission{ue.id(), r, ue.position(), cfg_.tx_power_dbm, ue.reservationPeriod()});
        }
    }

    //detect collision in this slot
    detectCollisions(tx_history_[slot]);

    // Every UE that is NOT transmitting in this slot decodes the SCIs it can
    // hear and records them. A UE transmitting in this slot hears nothing:
    // sidelink is half-duplex, and that blind spot is a real limitation of
    // the standard, not a modelling shortcut.

    const auto& txs = tx_history_[slot];
    for(auto& ue: ues_){
        //if the ue is transmitting, then it can't receive SCI from other ues
        bool transmitting = std::any_of(txs.begin(), txs.end(), 
                                        [&](const Transmission& t){return t.ue_id == ue.id();});

        if(transmitting) {
            ue.markUnmonitored(slot, cfg_);
            continue;
        }

        for(auto& tx: txs){
            double distance = std::abs(ue.position()-tx.tx_pos_m);
            double rsrp = Channel::rxPowerDbm(tx.tx_power_dbm, distance, cfg_);

            //sensingRecord : slot, subch_start, num_subch, rsrp, period
            SensingRecord sr = SensingRecord{slot, 
                                            tx.resource.subchannel_start,
                                            tx.resource.num_subchannels,
                                            rsrp,
                                            tx.reservation_period};
            ue.addSensingRecord(sr, slot, cfg_);
        }
    }

    
   
}

void Simulator::detectCollisions(const std::vector<Transmission>& txs) {
    //SINR-based reception
    if(txs.size()==0) return;

    double signal, interference, sinr;
    int collision = 0;
    int opportunity = 0;
    int rx_success = 0;

    // number of collision
    for(int i=0; i<int(txs.size())-1; i++){
        for(int j=i+1; j<int(txs.size()); j++){
            if(txs[i].resource.overlaps(txs[j].resource)){
                collision ++;
            }
        }
    }

    metrics_.total_transmissions += txs.size();
    metrics_.collisions += collision;

    // PRR
    for(auto &ue: ues_){
        for(auto &tx: txs){
            interference = 0.0;
            signal = Channel::rxPowerDbm(tx.tx_power_dbm, std::abs(tx.tx_pos_m - ue.position()), cfg_);

            if(tx.ue_id == ue.id()) continue;
            if(std::abs(tx.tx_pos_m - ue.position()) > cfg_.comm_range_m) continue;
            
            opportunity ++;
                
            for(auto &overlap_tx: txs){
                if(overlap_tx.ue_id == ue.id() || overlap_tx.ue_id == tx.ue_id) continue;
                if(std::abs(overlap_tx.tx_pos_m - ue.position()) > cfg_.comm_range_m) continue;
                
                if(tx.resource.overlaps(overlap_tx.resource)){
                    interference += Channel::dbmToMw(Channel::rxPowerDbm(overlap_tx.tx_power_dbm, std::abs(overlap_tx.tx_pos_m - ue.position()), cfg_));
                }
                
            }

            sinr = signal - Channel::mwToDbm(interference + Channel::dbmToMw(cfg_.noise_floor_dbm));
            rx_success += (sinr > cfg_.sinr_threshold_db);

        }
    }


    metrics_.rx_opportunities += opportunity;
    metrics_.rx_success += rx_success;
}

void Simulator::writeCsv(const std::string& path) const {
    // Append a row so repeated runs with different parameters accumulate
    // into one file that Python can plot directly.
    bool need_header = false;
    {
        std::ifstream probe(path);
        need_header = !probe.good() || probe.peek() == std::ifstream::traits_type::eof();
    }

    std::ofstream out(path, std::ios::app);
    if (!out) {
        std::cerr << "warning: cannot open " << path << " for writing\n";
        return;
    }

    if (need_header) {
        out << "num_ues,duration_ms,seed,sps,packet_period_ms,"
               "transmissions,collisions,collision_rate,prr\n";
    }

    out << cfg_.num_ues << ','
        << cfg_.sim_duration_ms << ','
        << cfg_.seed << ','
        << (cfg_.enable_sps ? 1 : 0) << ','
        << cfg_.packet_period_ms << ','
        << metrics_.total_transmissions << ','
        << metrics_.collisions << ','
        << metrics_.collisionRate() << ','
        << metrics_.prr() << '\n';
}
