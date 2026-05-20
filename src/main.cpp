#include "AtmosphericSimulator.h"
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

static std::atomic<bool> g_quit{false};
static void sigHandler(int) { g_quit = true; }

static bool ensure_fifo(const std::string& path) {
    if (mkfifo(path.c_str(), 0666) == 0) return true;
    return errno == EEXIST;
}

int main(int argc, char* argv[]) {
    std::string fifo_path = "/data/sim.fifo";
    int rate_hz = 1000;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--fifo") == 0 && i + 1 < argc)
            fifo_path = argv[++i];
        else if (std::strcmp(argv[i], "--rate") == 0 && i + 1 < argc)
            rate_hz = std::atoi(argv[++i]);
    }

    std::signal(SIGTERM, sigHandler);
    std::signal(SIGINT,  sigHandler);

    if (!ensure_fifo(fifo_path)) {
        std::cerr << "satellite_sim: cannot create FIFO " << fifo_path << '\n';
        return 1;
    }

    AtmosphericModel::Config cfg;
    cfg.solar_contaminated = (std::getenv("SIM_SOLAR") != nullptr);

    AtmosphericSimulator sim(fifo_path, cfg);
    sim.start();
    std::cout << "satellite_sim: running at " << rate_hz
              << " Hz, fifo=" << fifo_path << '\n';

    while (!g_quit) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "satellite_sim: shutting down, dropped="
              << sim.dropped() << '\n';
    sim.stop();
    return 0;
}
