#pragma once

#include <cstdint>

// All simulation parameters in one place.
// Values follow 3GPP TS 38.214 / TS 38.331 defaults where applicable.
struct Config {
    // ---- Resource pool ----
    // NR numerology mu=0 (15 kHz SCS) -> 1 slot = 1 ms
    int    num_subchannels      = 5;      // subchannels per slot
    int    subchannel_size_prb  = 10;     // PRBs per subchannel (informational)
    double slot_duration_ms     = 1.0;

    // ---- Simulation ----
    int    num_ues              = 50;
    int    sim_duration_ms      = 10000;  // 10 s
    double road_length_m        = 2000.0;
    unsigned seed               = 70;

    // ---- Traffic (CAM / BSM style periodic broadcast) ----
    double packet_period_ms     = 100.0;
    int    subchannels_per_pkt  = 2;      // L_subCH

    // ---- Sensing (TS 38.214 sec 8.1.4) ----
    int    sensing_window_ms    = 1100;   // T0
    int    selection_start_ms   = 1;      // T1
    int    selection_end_ms     = 100;    // T2, must be <= packet delay budget
    double rsrp_threshold_dbm   = -110.0; // Th(pi,pj), raised by 3 dB if too few candidates
    double candidate_ratio      = 0.2;    // X: keep top 20% of resources

    // ---- SPS / resource reselection ----
    int    resel_counter_min    = 5;      // C_resel range depends on packet period
    int    resel_counter_max    = 15;
    double prob_resource_keep   = 0.4;    // probResourceKeep in {0, 0.2, 0.4, 0.6, 0.8}

    // ---- Channel / PHY abstraction ----
    double tx_power_dbm         = 23.0;
    double noise_floor_dbm      = -95.0;
    double sinr_threshold_db    = 3.0;    // above this -> packet decoded
    double pathloss_exponent    = 2.0;    // free-space-ish
    double reference_loss_db    = 47.86;  // FSPL at 1 m, 5.9 GHz
    double comm_range_m         = 300.0;  // only count RX within this range for PRR

    // ---- Mobility ----
    double speed_min_mps        = 20.0;   // ~72 km/h
    double speed_max_mps        = 30.0;   // ~108 km/h

    // ---- Feature switches (for A/B experiments) ----
    bool   enable_sensing       = false;  // v0.1: false = pure random selection
    bool   enable_sps           = false;  // v0.4

    // ---- Output ----
    const char* output_csv      = "results/run.csv";
};
