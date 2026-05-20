#include "AtmosphericSimulator.h"
AtmosphericSimulator::AtmosphericSimulator(const std::string& p,
    const AtmosphericModel::Config& cfg) : pipe_path_(p), model_(cfg) {}

AtmosphericSimulator::~AtmosphericSimulator() { stop(); }
void AtmosphericSimulator::start() {}
void AtmosphericSimulator::stop() {}
void AtmosphericSimulator::producerLoop() {}
void AtmosphericSimulator::drainLoop() {}
