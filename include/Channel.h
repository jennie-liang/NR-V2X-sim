#pragma once

#include "Config.h"

// Simplified PHY abstraction. No fading, no fast time variation —
// just distance-based pathloss and an SINR threshold.
namespace Channel {

// Log-distance pathloss in dB.
double pathlossDb(double distance_m, const Config& cfg);

// Received power in dBm at `distance_m` from a transmitter at `tx_power_dbm`.
double rxPowerDbm(double tx_power_dbm, double distance_m, const Config& cfg);

// Convert dBm to linear mW and back — needed to sum interference correctly.
// Interference does NOT add in dB.
double dbmToMw(double dbm);
double mwToDbm(double mw);

}  // namespace Channel
