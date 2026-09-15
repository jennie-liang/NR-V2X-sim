#include "Config.h"
#include "Simulator.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {

void printUsage(const char* prog) {
    std::cout << "usage: " << prog << " [options]\n"
              << "  --ues N            number of UEs (default 50)\n"
              << "  --duration MS      simulation duration in ms (default 10000)\n"
              << "  --period MS        packet period in ms (default 100)\n"
              << "  --sps              enable semi-persistent scheduling and sensing-based selection\n"
              << "  --seed N           RNG seed (default 42)\n"
              << "  --out PATH         output csv (default results/run.csv)\n";
}

}  // namespace

int main(int argc, char** argv) {
    Config cfg;
    std::string out_path = cfg.output_csv;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto next = [&](const char* what) -> const char* {
            if (i + 1 >= argc) {
                std::cerr << "missing value for " << what << "\n";
                std::exit(1);
            }
            return argv[++i];
        };

        if      (arg == "--ues")      cfg.num_ues         = std::atoi(next("--ues"));
        else if (arg == "--duration") cfg.sim_duration_ms = std::atoi(next("--duration"));
        else if (arg == "--period")   cfg.packet_period_ms = std::atof(next("--period"));
        else if (arg == "--seed")     cfg.seed            = std::atoi(next("--seed"));
        else if (arg == "--out")      out_path            = next("--out");
        else if (arg == "--sps")      cfg.enable_sps      = true;
        else if (arg == "-h" || arg == "--help") { printUsage(argv[0]); return 0; }
        else {
            std::cerr << "unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    Simulator sim(cfg);
    sim.run();
    sim.writeCsv(out_path);

    return 0;
}
