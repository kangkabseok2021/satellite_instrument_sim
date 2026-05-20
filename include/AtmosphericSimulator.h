#pragma once
#include "SpscQueue.h"
#include "TelemetryFrame.h"
#include "AtmosphericModel.h"
#include <atomic>
#include <cstdint>
#include <string>
#include <thread>

class AtmosphericSimulator {
public:
    static constexpr std::size_t QUEUE_CAPACITY = 4096;

    explicit AtmosphericSimulator(const std::string& pipe_path,
                                  const AtmosphericModel::Config& cfg);
    ~AtmosphericSimulator();

    void start();
    void stop();

    uint64_t dropped() const { return dropped_.load(std::memory_order_relaxed); }

private:
    void producerLoop();
    void drainLoop();

    std::string pipe_path_;
    AtmosphericModel model_;
    SpscQueue<TelemetryFrame, QUEUE_CAPACITY> queue_;
    std::atomic<bool>     running_{false};
    std::atomic<uint64_t> dropped_{0};
    std::thread           producer_;
    std::thread           drainer_;
};
