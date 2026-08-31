#include "Channel.h"
#include <cmath>
#include <algorithm>

namespace Channel {

double pathlossDb(double distance_m, const Config& cfg) {
    // TODO(v0.1)
    // PL = reference_loss_db + 10 * pathloss_exponent * log10(d)
    // Clamp d to at least 1.0 m.
    (void)distance_m;
    (void)cfg;
    return 0.0;
}

double rxPowerDbm(double tx_power_dbm, double distance_m, const Config& cfg) {
    return tx_power_dbm - pathlossDb(distance_m, cfg);
}

double dbmToMw(double dbm) {
    return std::pow(10.0, dbm / 10.0);
}

double mwToDbm(double mw) {
    return 10.0 * std::log10(std::max(mw, 1e-30));
}

}  // namespace Channel
