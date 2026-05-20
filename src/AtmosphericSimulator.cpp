#include "AtmosphericSimulator.h"
#include "TelemetryPipe.h"
#include <chrono>

using namespace std::chrono;

AtmosphericSimulator::AtmosphericSimulator(const std::string& pipe_path,
                                           const AtmosphericModel::Config& cfg)
    : pipe_path_(pipe_path), model_(cfg)
{}

AtmosphericSimulator::~AtmosphericSimulator() { stop(); }

void AtmosphericSimulator::start() {
    running_.store(true, std::memory_order_relaxed);
    producer_ = std::thread(&AtmosphericSimulator::producerLoop, this);
    drainer_  = std::thread(&AtmosphericSimulator::drainLoop,    this);
}

void AtmosphericSimulator::stop() {
    running_.store(false, std::memory_order_relaxed);
    if (producer_.joinable()) producer_.join();
    if (drainer_.joinable())  drainer_.join();
}

void AtmosphericSimulator::producerLoop() {
    uint16_t altitude = 500;
    auto next = steady_clock::now();

    while (running_.load(std::memory_order_relaxed)) {
        next += microseconds(1000);  // 1 kHz = 1000 µs period

        TelemetryFrame f{};
        f.ts_us = static_cast<uint64_t>(
            duration_cast<microseconds>(
                system_clock::now().time_since_epoch()).count());
        f.channel_id = 0;
        f.altitude_m = altitude;
        auto s = model_.sample(altitude);
        f.backscatter_raw = s.backscatter_raw;
        f.snr_x10         = s.snr_x10;
        f.solar_flag      = s.solar_flag;
        f.crc8            = frame::crc8(
            reinterpret_cast<const uint8_t*>(&f), 15);

        if (!queue_.push(f))
            dropped_.fetch_add(1, std::memory_order_relaxed);

        altitude = (altitude >= 30000) ? 500 : static_cast<uint16_t>(altitude + 500);

        std::this_thread::sleep_until(next);
    }
}

void AtmosphericSimulator::drainLoop() {
    TelemetryPipe pipe(pipe_path_);

    while (running_.load(std::memory_order_relaxed) || !queue_.empty()) {
        auto frame = queue_.pop();
        if (!frame) {
            std::this_thread::sleep_for(microseconds(500));
            continue;
        }
        uint8_t buf[16];
        frame::serialise(*frame, buf);
        pipe.write_frame(buf);
    }
}
